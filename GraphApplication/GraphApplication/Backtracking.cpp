#include "pch.h"
#include "Graph.h"
#include <set>
#include <tuple>


// =============================================================================
// SalesmanTrackBacktracking ===================================================
// =============================================================================
CTrack SalesmanTrackBacktracking(CGraph& graph, CVisits& visits) 
{
	return CTrack(&graph);
}


// =============================================================================
// SalesmanTrackBacktrackingGreedy =============================================
// =============================================================================
void BacktrackingGreedy(vector<vector<tuple<CTrack*, double>>>& visitsTracksMat, vector<bool>& inStackVec, int actVertex, vector<int>& stackTrack, vector<int>& optTrack, int nVertexToVisit, double& optDist, double& actDist)
{
	stackTrack.push_back(actVertex);
	inStackVec[actVertex] = true;

	if (stackTrack.size() == nVertexToVisit && stackTrack.back() == nVertexToVisit - 1)
	{
		if (actDist < optDist)
		{
			optTrack = stackTrack;
			optDist = actDist;
		}
	}
	else {
		for (int nextVertex = 0; nextVertex < nVertexToVisit; nextVertex++)
		{
			if (!inStackVec[nextVertex])
			{
				actDist += get<1>(visitsTracksMat[actVertex][nextVertex]);
				BacktrackingGreedy(visitsTracksMat, inStackVec, nextVertex, stackTrack, optTrack, nVertexToVisit, optDist, actDist);
			}
		}
	}

	int popVertex = stackTrack.back();
	stackTrack.pop_back();
	inStackVec[popVertex] = false;

	if (stackTrack.size() > 0) 
		actDist -= get<1>(visitsTracksMat[popVertex][stackTrack.back()]);
	else 
		actDist = 0;

	return;
}

CTrack SalesmanTrackBacktrackingGreedy(CGraph& graph, CVisits& visits)
{
	// Guardar en un array de dues dimensions tots els camins òptims entre vèrtexs a visitar.
	vector<vector<tuple<CTrack*, double>>> visitsTracksMat(visits.GetNVertices(), vector<tuple<CTrack*, double>>(visits.GetNVertices(), {0, 0}));
	
	for (auto it_i = begin(visits.m_Vertices); it_i != end(visits.m_Vertices); it_i++) {
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


	int nVertexToVisit = visits.m_Vertices.size();
	vector<int> stackTrack;
	vector<bool> inStackMap(nVertexToVisit, false);
	vector<int> optimousTrack;
	int actVertex = 0;
	double optimousDist = std::numeric_limits<double>::max();
	double actDist = 0;
	BacktrackingGreedy(visitsTracksMat, inStackMap, actVertex, stackTrack, optimousTrack, nVertexToVisit, optimousDist, actDist);


	CTrack resultTrack(&graph);
	int dest = optimousTrack.front();
	optimousTrack.erase(optimousTrack.begin());
	for (int orig : optimousTrack) {
		CTrack* t = get<0>(visitsTracksMat[dest][orig]);
		resultTrack.Append(*t);
		dest = orig;
	}

	return resultTrack;
}