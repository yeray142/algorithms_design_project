#include "pch.h"
#include "Graph.h"
#include <vector>
#include <queue>
using namespace std;

// =============================================================================
// SpanningTreePrim ============================================================
// =============================================================================
CSpanningTree SpanningTreePrim(CGraph & graph)
{
	if (graph.GetNEdges() == 0) {
		return CSpanningTree(&graph);
	}

	struct comparator {
		bool operator()(CEdge* pE1, CEdge* pE2) {
			return pE1->m_Length > pE2->m_Length;
		}
	};

	// 1 - Inicialitzar un arbre amb un unic vertex seleccionat arbitrariament del graf
	CSpanningTree spanningTree(&graph);
	priority_queue<CEdge*, vector<CEdge*>, comparator> edgeQueue;
	
	CVertex* initialVertex = graph.GetVertex(0);

	for (CVertex& v : graph.m_Vertices) v.m_PrimInTree = true;
	initialVertex->m_PrimInTree = false;

	for (CEdge* edge : initialVertex->m_Edges) {
		edgeQueue.push(edge);
	}

	// 2 - Afegir aresta amb pes minim que connecti un vertex de l'arbre amb un vertex de fora de l'arbre
	while ((spanningTree.GetNEdges() < graph.GetNVertices() - 1) && !(edgeQueue.empty())) {
		// Treu l'aresta de pes minim de la cua:
		CEdge* minEdge = edgeQueue.top();
		edgeQueue.pop();

		// Obtenir vertexs origen i desti de l'aresta:
		CVertex* originV = minEdge->m_pOrigin;
		CVertex* destV = minEdge->m_pDestination;

		// Comprova si aquesta aresta forma un cicle a l'arbre actual:
		if (originV->m_PrimInTree || destV->m_PrimInTree) {
			// Aresta no crea un cicle, aixi que l'afegim a l'arbre:
			spanningTree.Add(minEdge);
			destV->m_PrimInTree = false;

			// Afegim les arestes del vertex desti no visitades a la cua.
			for (CEdge* edge : destV->m_Edges) {
				if (edge->m_pDestination->m_PrimInTree) {
					edgeQueue.push(edge);
				}
			}
		}
		// 3 - Repetir el pas 2 fins que l'arbre contingui tots els vertexs del graf o cua buida.
	}

	return spanningTree;
}
