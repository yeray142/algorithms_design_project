#include "pch.h"
#include "Graph.h"
#include <queue>
#include <iostream>
#include <iomanip> 
#include <tuple> 
#include <set>


// SalesmanTrackBranchAndBound1 ===================================================
CTrack SalesmanTrackBranchAndBound1(CGraph& graph, CVisits& visits)
{
	return CTrack(&graph);
}


// SalesmanTrackBranchAndBound2 ===================================================
class CBB2 {
public:
	CBB2* m_pFather;
	int m_Visita;
	double m_Length;

	// Llista de vertexs a visitar.
	set<int> m_ToVisit;

	// Cotes superior i inferior.
	double m_CotaInferior;
	double m_CotaSuperior;

	// Per alliberar memoria.
	unsigned m_CntRef;

	// Constructor per al vertex inicial.
	CBB2(int visita, double length, set<int>& toVisit)
		: m_pFather(NULL)
		, m_Length(length)
		, m_Visita(visita)
		, m_CntRef(1)
		, m_ToVisit(toVisit)
	{
	}

	// Constructor per a la resta de vertexs.
	CBB2(CBB2* pFather, int visita, double length)
		: m_pFather(pFather)
		, m_Visita(visita)
		, m_Length(pFather->m_Length + length)
		, m_CntRef(1)
	{
		++m_pFather->m_CntRef;

		m_ToVisit = pFather->m_ToVisit;
		m_ToVisit.erase(visita);
	}

	// Allibera memoria.
	void Unlink()
	{
		if (--m_CntRef == 0) {
			if (m_pFather) m_pFather->Unlink();
			delete this;
		}
	}
};

struct comparatorCBB2 {
	bool operator()(const CBB2* s1, const CBB2* s2) {
		return s1->m_CotaInferior > s2->m_CotaInferior;
	}
};

void createVisitsTracksMatrix(vector<vector<tuple<CTrack*, double>>>& visitsTracksMat, set<int>& visitsIndex, CGraph& graph, CVisits& visits)
{
	for (auto it_i = begin(visits.m_Vertices); it_i != end(visits.m_Vertices); it_i++)
	{
		visitsIndex.insert(distance(visits.m_Vertices.begin(), it_i));
		DijkstraQueue(graph, *it_i);

		for (auto it_j = begin(visits.m_Vertices); it_j != end(visits.m_Vertices); it_j++) {

			CTrack* track = new CTrack(&graph);
			CVertex* pV = (*it_j);
			while (pV->m_Name != (*it_i)->m_Name) {
				track->AddFirst(pV->m_pDijkstraPrevious);
				pV = pV->m_pDijkstraPrevious->m_pOrigin;
			}

			if ((*it_i)->m_Name != (*it_j)->m_Name) {
				visitsTracksMat[distance(visits.m_Vertices.begin(), it_i)][distance(visits.m_Vertices.begin(), it_j)] = { track, track->Length() };
			}
		}
	}
}

void createResultTrack(vector<vector<tuple<CTrack*, double>>>& visitsTracksMat, CTrack& resultTrack, list<int>& camiMesCurt)
{
	int dest = camiMesCurt.front();
	camiMesCurt.erase(camiMesCurt.begin());
	for (int orig : camiMesCurt) {
		CTrack* t = get<0>(visitsTracksMat[dest][orig]);
		resultTrack.Append(*t);
		dest = orig;
	}
}

void getMinMaxCVj(vector<vector<tuple<CTrack*, double>>>& visitsTracksMat, double& min, double& max, int j)
{
	max = 0;
	min = numeric_limits<double>::max();

	for (int i = 0; i < visitsTracksMat.size() - 1; i++)
	{
		// Si es del mateix al mateix node, no tenir en compte.
		if (i == j) continue;

		// Si es del primer a l'ultim node, no tenir en compte.
		if (i == 0 && j == visitsTracksMat.size() - 1) continue;

		// Trobar maxims i minims de columnes.
		double distance = get<1>(visitsTracksMat[i][j]);
		if (distance > max) max = distance;
		if (distance < min) min = distance;
	}
}

CTrack SalesmanTrackBranchAndBound2(CGraph& graph, CVisits &visits)
{
	// Llista per guardar el cami mes curt i la seva longitud.
	list<int> camiMesCurt;
	double longitudCamiMesCurt = numeric_limits<double>::max();
	double cotaSuperiorMinima = numeric_limits<double>::max();

	// Guardar en un array de dues dimensions tots els camins òptims entre vèrtexs a visitar.
	vector<vector<tuple<CTrack*, double>>> visitsTracksMat(visits.GetNVertices(), vector<tuple<CTrack*, double>>(visits.GetNVertices(), { 0, 0 }));
	set<int> visitsIndex;
	createVisitsTracksMatrix(visitsTracksMat, visitsIndex, graph, visits);
	
	// Calcular cotes i nodes a visitar per al node inicial.
	visitsIndex.erase(visitsIndex.begin());
	CBB2 nodeStart(0, 0, visitsIndex);
	double cotaSuperior = 0.0, cotaInferior = 0.0;
	for (int j : nodeStart.m_ToVisit)
	{
		double max, min;
		getMinMaxCVj(visitsTracksMat, min, max, j);

		cotaSuperior += max;
		cotaInferior += min;
	}
	nodeStart.m_CotaInferior = cotaInferior;
	nodeStart.m_CotaSuperior = cotaSuperior;
	if (cotaSuperior < cotaSuperiorMinima && cotaSuperior > 0) cotaSuperiorMinima = cotaSuperior;

	// Afegir node inicial a la cua.
	priority_queue<CBB2*, std::vector<CBB2*>, comparatorCBB2> queue;
	queue.push(&nodeStart);

	while (!queue.empty())
	{
		// Treure de la cua el node amb la cota minima mes baixa.
		CBB2* pN = queue.top();
		queue.pop();

		// Si hem trobat una solucio millor al node amb la cota inferior mes baixa de la cua, no cal continuar.
		if (pN->m_CotaInferior > cotaSuperiorMinima + 1e-5 || pN->m_CotaInferior > longitudCamiMesCurt + 1e-5) 
			break;

		for (int v : pN->m_ToVisit) 
		{
			// Si es la darrera visita i encara no hem visitat totes les anteriors, podar.
			if (pN->m_ToVisit.size() > 1 && v == visits.GetNVertices() - 1) continue;

			// Podar si la longitud actual es superior a la del cami mes curt trobat fins ara.
			if (pN->m_Length + get<1>(visitsTracksMat[pN->m_Visita][v]) > longitudCamiMesCurt) continue;

			// Si es solucio, actualitzar longitud cami mes curt i llista de visites que conformen el cami mes curt.
			if (v == visits.m_Vertices.size() - 1)
			{
				CBB2* pAnterior = pN;
				camiMesCurt.clear();
				camiMesCurt.push_front(v);
				while (pAnterior) {
					camiMesCurt.push_front(pAnterior->m_Visita);
					pAnterior = pAnterior->m_pFather;
				}
				longitudCamiMesCurt = pN->m_Length + get<1>(visitsTracksMat[pN->m_Visita][v]);
				continue;
			}

			// Calcular cotes inferiors i superiors.
			double minCVj, maxCVj;
			getMinMaxCVj(visitsTracksMat, minCVj, maxCVj, v);
			cotaSuperior = pN->m_CotaSuperior - maxCVj + get<1>(visitsTracksMat[pN->m_Visita][v]);
			cotaInferior = pN->m_CotaInferior - minCVj + get<1>(visitsTracksMat[pN->m_Visita][v]);
			if (cotaSuperior < cotaSuperiorMinima) cotaSuperiorMinima = cotaSuperior;

			// Podar si la cota inferior es superior a la cota superior minima o si la cota inferior es superior a la longitud del cami mes curt.
			if (cotaInferior > cotaSuperiorMinima + 1e-5 || cotaInferior > longitudCamiMesCurt) continue;

			// Crear nova instancia de node CBB2.
			CBB2* nodeFill = new CBB2(pN, v, get<1>(visitsTracksMat[pN->m_Visita][v]));
			nodeFill->m_CotaInferior = cotaInferior;
			nodeFill->m_CotaSuperior = cotaSuperior;

			// Afegir node a la cua.
			queue.push(nodeFill);
		}

		// Allibera memoria.
		// if (pN->m_Visita != nodeStart.m_Visita) pN->Unlink();
	}

	// Allibera memoria restant.
	/*
	while (!queue.empty()) {
		CBB2* pS = queue.top();
		queue.pop();
		pS->Unlink();
	}
	*/
	
	// Crear el track a partir del cami d'indexos mes curt.
	CTrack resultTrack(&graph);
	createResultTrack(visitsTracksMat, resultTrack, camiMesCurt);

	return resultTrack;
}


// SalesmanTrackBranchAndBound3 ===================================================
CTrack SalesmanTrackBranchAndBound3(CGraph& graph, CVisits &visits)
{
	return CTrack(&graph);
}
