#include "pch.h"
#include "Graph.h"
#include <set>
#include <tuple>


// =============================================================================
// SalesmanTrackBacktracking ===================================================
// =============================================================================

CTrack CMesCurt(NULL);
CVertex* pDest;

double LongitudCMesCurt;
double LongitudCActual;

struct Node {
	CEdge* m_pEdge;
	Node* m_pAnterior;
};

bool everythingVisited(CVisits& visits) 
{
	for (CVertex* pV : visits.m_Vertices) {
		if (pV->m_VertexToVisit)
			return false;
	}
	return true;
}

void BacktrackingPur(CVisits& visits, Node* pAnterior, CVertex* pActual, int& currentTram)
{
	if (pActual == pDest && everythingVisited(visits)) {
		// Solo consideramos el camino si todas las visitas intermedias han sido visitadas
		if (LongitudCActual < LongitudCMesCurt) {
			CMesCurt.Clear();
			while (pAnterior) {
				CMesCurt.m_Edges.push_front(pAnterior->m_pEdge);
				pAnterior = pAnterior->m_pAnterior;
			}
			LongitudCMesCurt = LongitudCActual;
		}
	}
	else if (LongitudCActual < LongitudCMesCurt) {
		Node node;
		node.m_pAnterior = pAnterior;

		for (CEdge* pE : pActual->m_Edges) {
			if (!(find(pE->m_pDestination->m_Trams.begin(), pE->m_pDestination->m_Trams.end(), currentTram) != end(pE->m_pDestination->m_Trams))) {
				
				node.m_pEdge = pE;
				bool vertexToVisit = pE->m_pDestination->m_VertexToVisit;
				if (vertexToVisit) {
					pE->m_pDestination->m_VertexToVisit = false;
					currentTram++;
				}
				pE->m_pDestination->m_Trams.push_back(currentTram);
				LongitudCActual += pE->m_Length;

				// Pasamos la lista de visitas intermedias a la siguiente llamada recursiva
				BacktrackingPur(visits, &node, pE->m_pDestination, currentTram);

				LongitudCActual -= pE->m_Length;
				pE->m_pDestination->m_Trams.pop_back();
				if (vertexToVisit) {
					pE->m_pDestination->m_VertexToVisit = true;
					currentTram--;
				}
			}
		}
	}
}


CTrack SalesmanTrackBacktracking(CGraph& graph, CVisits& visits) 
{
	CMesCurt.SetGraph(&graph);
	CMesCurt.Clear();

	LongitudCMesCurt = numeric_limits<double>::max();
	LongitudCActual = 0.0;
	pDest = visits.m_Vertices.back();

	for (CVertex& pV : graph.m_Vertices) {
		pV.m_Trams.clear();
		pV.m_VertexToVisit = false;
	}
	for (CVertex* pV : visits.m_Vertices) pV->m_VertexToVisit = true;

	int currentTram = 0;
	visits.m_Vertices.front()->m_VertexToVisit = false;
	BacktrackingPur(visits, NULL, visits.m_Vertices.front(), currentTram);

	return CMesCurt;
}



// =============================================================================
// SalesmanTrackBacktrackingGreedy =============================================
// =============================================================================
list<int> camiMesCurt;
double longitudCamiMesCurt = numeric_limits<double>::max();
double longitudCamiActual = 0;

struct TrackNode {
	int m_visita;
	TrackNode* m_pAnterior;
	int m_nNodes;
};

bool isVisited(const TrackNode* pAnterior, int visita) {
	for (; pAnterior; pAnterior = pAnterior->m_pAnterior) {
		if (pAnterior->m_visita == visita)
			return true;
	}
	return false;
}

void BacktrackingGreedy(vector<vector<tuple<CTrack*, double>>>& visitsTracksMatrix, list<int>& visits, TrackNode* pAnterior, int pActual)
{
	// Si el cami ja conte totes les visites.
	if (pAnterior->m_nNodes == visitsTracksMatrix.size() && pActual == visitsTracksMatrix.size() - 1)
	{
		// Actualitzar cami mes curt si l'actual es mes curt.
		if (longitudCamiActual < longitudCamiMesCurt) 
		{
			camiMesCurt.clear();
			while (pAnterior) {
				camiMesCurt.push_front(pAnterior->m_visita);
				pAnterior = pAnterior->m_pAnterior;
			}
			longitudCamiMesCurt = longitudCamiActual;
		}
		return;
	}
	else if (longitudCamiActual < longitudCamiMesCurt) 
	{
		TrackNode node;
		node.m_pAnterior = pAnterior;
		node.m_nNodes = pAnterior->m_nNodes;
		for (int visitIndex : visits) {
			if (!isVisited(pAnterior, visitIndex)) {
				if (visitIndex == visitsTracksMatrix.size() - 1 &&
					node.m_nNodes < visitsTracksMatrix.size() - 1)
					continue;
				node.m_visita = visitIndex;
				node.m_nNodes++;
				longitudCamiActual += get<1>(visitsTracksMatrix[pAnterior->m_visita][visitIndex]);
				BacktrackingGreedy(visitsTracksMatrix, visits, &node, visitIndex);
				longitudCamiActual -= get<1>(visitsTracksMatrix[pAnterior->m_visita][visitIndex]);
				node.m_nNodes--;
			}
		}
	}
}

CTrack SalesmanTrackBacktrackingGreedy(CGraph& graph, CVisits& visits)
{
	// Guardar en un array de dues dimensions tots els camins òptims entre vèrtexs a visitar.
	vector<vector<tuple<CTrack*, double>>> visitsTracksMat(visits.GetNVertices(), vector<tuple<CTrack*, double>>(visits.GetNVertices(), {0, 0}));
	list<int> visitsIndex;
	
	for (auto it_i = begin(visits.m_Vertices); it_i != end(visits.m_Vertices); it_i++) {
		DijkstraQueue(graph, *it_i);
		visitsIndex.push_back(distance(visits.m_Vertices.begin(), it_i));

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

	visitsIndex.pop_front();
	TrackNode node = {0, NULL, 1};
	BacktrackingGreedy(visitsTracksMat, visitsIndex, &node, 0);


	CTrack resultTrack(&graph);
	int dest = camiMesCurt.front();
	camiMesCurt.erase(camiMesCurt.begin());
	for (int orig : camiMesCurt) {
		CTrack* t = get<0>(visitsTracksMat[dest][orig]);
		resultTrack.Append(*t);
		dest = orig;
	}

	return resultTrack;
}