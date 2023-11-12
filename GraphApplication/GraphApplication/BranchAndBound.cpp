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

	set<int> m_ToVisit;

	double m_CotaInferior;
	double m_CotaSuperior;

	unsigned m_CntRef;

	CBB2(int visita, double length)
		: m_pFather(NULL)
		, m_Length(length)
		, m_Visita(visita)
		, m_CntRef(1)
	{
	}

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

void createVisitsTracksMatrix(vector<vector<tuple<CTrack*, double>>>& visitsTracksMat, list<int>& visitsIndex, CGraph& graph, CVisits& visits)
{
	for (auto it_i = begin(visits.m_Vertices); it_i != end(visits.m_Vertices); it_i++)
	{
		visitsIndex.push_back(distance(visits.m_Vertices.begin(), it_i));
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

void getInitialMaxMinCVi(vector<vector<tuple<CTrack*, double>>>& visitsTracksMat, double& cotaSuperior, double& cotaInferior, set<int> toVisit) 
{
	cotaSuperior = 0.0;
	cotaInferior = 0.0;

	for (int v : toVisit) 
	{
		double cotaSuperiorAux = 0;
		double cotaInferiorAux = numeric_limits<double>::max();
		for (int i = 0; i < visitsTracksMat.size() - 1; i++) 
		{
			if (i == v)
				continue;

			double distance = get<1>(visitsTracksMat[i][v]);
			if (distance < cotaInferiorAux) cotaInferiorAux = distance;
			if (distance > cotaSuperiorAux) cotaSuperiorAux = distance;
		}
		cotaSuperior += cotaSuperiorAux;
		cotaInferior += cotaInferiorAux;
	}
}

void getMinMaxCVj(vector<vector<tuple<CTrack*, double>>>& visitsTracksMat, double& min, double& max,int j) 
{
	max = 0;
	min = numeric_limits<double>::max();

	for (int i = 0; i < visitsTracksMat.size() - 1; i++)
	{
		if (i == j)
			continue;

		double distance = get<1>(visitsTracksMat[i][j]);
		if (distance > max) max = distance;
		if (distance < min) min = distance;
	}
}

CTrack SalesmanTrackBranchAndBound2(CGraph& graph, CVisits &visits)
{
	// Llista per guardar el cami mes curt i la longitud.
	list<int> camiMesCurt;
	double longitudCamiMesCurt = numeric_limits<double>::max();
	double cotaSuperiorMinima = numeric_limits<double>::max();

	// Guardar en un array de dues dimensions tots els camins òptims entre vèrtexs a visitar.
	vector<vector<tuple<CTrack*, double>>> visitsTracksMat(visits.GetNVertices(), vector<tuple<CTrack*, double>>(visits.GetNVertices(), { 0, 0 }));
	list<int> visitsIndex;
	createVisitsTracksMatrix(visitsTracksMat, visitsIndex, graph, visits);
	
	visitsIndex.pop_front();
	CBB2 nodeStart(0, 0);
	for (int v : visitsIndex) nodeStart.m_ToVisit.insert(v);
	double cotaSuperior, cotaInferior;
	getInitialMaxMinCVi(visitsTracksMat, cotaSuperior, cotaInferior, nodeStart.m_ToVisit);
	nodeStart.m_CotaInferior = cotaInferior;
	nodeStart.m_CotaSuperior = cotaSuperior;
	if (cotaSuperior < cotaSuperiorMinima) cotaSuperiorMinima = cotaSuperior;

	priority_queue<CBB2*, std::vector<CBB2*>, comparatorCBB2> queue;
	queue.push(&nodeStart);

	while (!queue.empty())
	{
		CBB2* pN = queue.top();
		queue.pop();

		for (int v : pN->m_ToVisit) 
		{
			if (pN->m_ToVisit.size() > 1 && v == visits.GetNVertices() - 1)
				continue;

			// Calcular cotes inferiors i superiors.
			double minCVj, maxCVj;
			getMinMaxCVj(visitsTracksMat, minCVj, maxCVj, v);
			cotaSuperior = pN->m_CotaSuperior - maxCVj + get<1>(visitsTracksMat[pN->m_Visita][v]) + 1e-5;
			cotaInferior = pN->m_CotaInferior - minCVj + get<1>(visitsTracksMat[pN->m_Visita][v]);
			if (cotaSuperior < cotaSuperiorMinima) cotaSuperiorMinima = cotaSuperior;

			// Si cal podar per cota saltar-se node.
			if (cotaInferior > cotaSuperiorMinima)
				continue;
			
			// Crear nova instancia de node CBB2.
			CBB2* nodeFill = new CBB2(pN, v, get<1>(visitsTracksMat[pN->m_Visita][v]));
			nodeFill->m_CotaInferior = cotaInferior;
			nodeFill->m_CotaSuperior = cotaSuperior;

			// Podar si la longitud actual es superior a la del cami mes curt trobat fins ara.
			if (nodeFill->m_Length > longitudCamiMesCurt) {
				nodeFill->Unlink();
				continue;
			}

			// Afegir node a la cua.
			queue.push(nodeFill);

			// Si es solucio, actualitzar longitud cami mes curt i llista de camiMesCurt.
			if (v == visits.m_Vertices.size() - 1)
			{
				CBB2* pAnterior = nodeFill;
				camiMesCurt.clear();
				while (pAnterior) {
					camiMesCurt.push_front(pAnterior->m_Visita);
					pAnterior = pAnterior->m_pFather;
				}
				longitudCamiMesCurt = nodeFill->m_Length;
			}
		}
		// pN->Unlink();
	}

	while (!queue.empty()) {
		CBB2* pS = queue.top();
		queue.pop();
		pS->Unlink();
	}
	
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
