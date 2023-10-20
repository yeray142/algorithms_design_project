#include "pch.h"
#include "Graph.h"
#include "GraphApplicationDlg.h"
#include <set>



// =============================================================================
// CONVEX HULL =================================================================
// =============================================================================

// QuickHull ===================================================================

double PosicioRespecteRecta(CGPoint& a, CGPoint& b, CGPoint& c) {
	return (a.m_Y - b.m_Y) * (c.m_X - b.m_X) - (a.m_X - b.m_X) * (c.m_Y - b.m_Y);
}

double AreaTriangle(CGPoint& a, CGPoint& b, CGPoint c) {
	return abs((a.m_Y - b.m_Y) * (c.m_X - b.m_X) - (a.m_X - b.m_X) * (c.m_Y - b.m_Y)) / 2.0;
}

CConvexHull ConvexHull(CVertex* a, CVertex* b, CConvexHull& punts)
{
	// Si Punts �s Buit retornar la llista [A,B]
	if (punts.GetNVertices() == 0)
	{
		CConvexHull buit(punts.m_pGraph);
		if (a->m_Name == b->m_Name)
			buit.m_Vertices.push_back(a);
		else 
		{
			buit.m_Vertices.push_back(a);
			buit.m_Vertices.push_back(b);
		}
		return buit;
	}

	// Trobar el punt C de Punts que maximitzi AreaTriangle(A,C,B)
	CVertex* c = nullptr;
	double max = 0.0;

	for (CVertex* v : punts.m_Vertices)
	{
		double newArea = AreaTriangle(a->m_Point, v->m_Point, b->m_Point);
		if (newArea > max)
		{
			c = v;
			max = newArea;
		}
	}

	// Separar punts en:
	// Left: Punts que estan a l�esquerra de la recta AC
	// Right: Punts que estan a la dreta de CB
	CConvexHull Left(punts.m_pGraph), Right(punts.m_pGraph);
	for (CVertex* v : punts.m_Vertices)
	{
		if (PosicioRespecteRecta(a->m_Point, c->m_Point, v->m_Point) > 0)
			Left.m_Vertices.push_back(v);

		if (PosicioRespecteRecta(c->m_Point, b->m_Point, v->m_Point) > 0)
			Right.m_Vertices.push_back(v);
	}

	// Calcular les parts esquerra i dreta de l�envoltant convexa
	CConvexHull CHLeft = ConvexHull(a, c, Left);
	CConvexHull CHRight = ConvexHull(c, b, Right);

	// Ajuntar les dos CHLeft i CHRight concatenant les llistes de punts. Compta en no repetir el punt C que pertanyen a les dues parts.
	/*list<CVertex*>::iterator it = CHRight.m_Vertices.begin();
	it++;
	for (; it != CHRight.m_Vertices.end(); it++)
		CHLeft.m_Vertices.push_back((*it));
	*/
	if (CHRight.m_Vertices.size() > 1) {
		CHLeft.m_Vertices.insert(CHLeft.m_Vertices.end(), std::next(CHRight.m_Vertices.begin()), CHRight.m_Vertices.end());
	}

	return CHLeft;
}

CConvexHull QuickHull(CGraph& graph)
{
	// Empty graph
	if (graph.GetNVertices() == 0)
		return CConvexHull(&graph);

	// Just one node
	if (graph.GetNVertices() == 1) 
	{
		CConvexHull ch = CConvexHull(&graph);
		ch.m_Vertices.push_back(&graph.m_Vertices.front());
		return ch;
	}

	// Buscar dos punts extrems A i B que poden ser el que est� m�s a l�esquerra i el que est� m�s a la dreta.
	CVertex* A = &graph.m_Vertices.front();
	CVertex* B = &graph.m_Vertices.front();
	for (CVertex& v : graph.m_Vertices)
	{
		if (v.m_Point.m_X < A->m_Point.m_X)
			A = &v;

		if (v.m_Point.m_X > B->m_Point.m_X)
			B = &v;
	}

	// En cas de que siguin vertexs verticals:
	if (A->m_Name == B->m_Name)
	{
		for (CVertex& v : graph.m_Vertices)
		{
			if (v.m_Point.m_Y < A->m_Point.m_Y)
				A = &v;

			if (v.m_Point.m_Y > B->m_Point.m_Y)
				B = &v;
		}
	}

	// En cas de que esiguin superposats:
	if (A->m_Name == B->m_Name && graph.GetNVertices() >= 2) 
	{
		CConvexHull ch(&graph);
		ch.m_Vertices.push_back(A);
		return ch;
	}

	// Separar el conjunt punts en els quals estan a sobre i els que estan a sota de la recta AB
	CConvexHull Up(&graph), Down(&graph);
	for (CVertex& v : graph.m_Vertices) {
		if (PosicioRespecteRecta(A->m_Point, B->m_Point, v.m_Point) > 0)
			Up.m_Vertices.push_back(&v);
		else if (PosicioRespecteRecta(A->m_Point, B->m_Point, v.m_Point) < 0)
			Down.m_Vertices.push_back(&v);
	}

	// Calcular el tros superior e inferior de l�envoltant convexa
	CConvexHull CHUp = ConvexHull(A, B, Up);
	CConvexHull CHDown = ConvexHull(B, A, Down);

	// Ajuntar les dues CHUp i CHDown concatenant les llistes de punts. Compta en no repetir els punts A i B que pertanyen a les dues parts.
	/*list<CVertex*>::iterator it = CHDown.m_Vertices.begin();
	it++;
	for (; it != CHDown.m_Vertices.end(); it++)
		CHUp.m_Vertices.push_back((*it));
	CHUp.m_Vertices.pop_back();
	*/
	if (CHDown.m_Vertices.size() > 1) {
		CHUp.m_Vertices.insert(CHUp.m_Vertices.end(), std::next(CHDown.m_Vertices.begin()), std::prev(CHDown.m_Vertices.end()));
	}
	
	return CHUp;
}