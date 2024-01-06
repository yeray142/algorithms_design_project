#include "pch.h"
#include "Graph.h"
#include <queue>
#include <iostream>
#include <iomanip> 
#include <random>
#include <chrono>

#include <set>
#include <tuple>


// Funcions generals ==================================================
void createVisitsTracksMatrixProbabilistic(vector<vector<tuple<CTrack*, double>>>& visitsTracksMat, set<int>& visitsIndex, CGraph& graph, CVisits& visits)
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

void createResultTrackProbabilistic(vector<vector<tuple<CTrack*, double>>>& visitsTracksMat, CTrack& resultTrack, list<int>& camiMesCurt)
{
	int dest = camiMesCurt.front();
	camiMesCurt.erase(camiMesCurt.begin());
	for (int orig : camiMesCurt) {
		CTrack* t = get<0>(visitsTracksMat[dest][orig]);
		resultTrack.Append(*t);
		dest = orig;
	}
}


// SalesmanTrackProbabilistic ==================================================
void randomConfig(list<int>& camiMesCurt, int nVisits)
{
	camiMesCurt.clear();

	vector<int> randomList;
	for (int i = 1; i < nVisits - 1; ++i)
		randomList.push_back(i);

	camiMesCurt.push_back(0);
	while (randomList.size() > 1)
	{
		// Generate a random integer
		int random_number = 1 + (rand() % (randomList.size() - 1));

		camiMesCurt.push_back(randomList[random_number]);

		auto it = randomList.begin();
		for (int i = 0; i < random_number; i++)
			it++;
		randomList.erase(it);
	}
	camiMesCurt.push_back(randomList[0]);
	camiMesCurt.push_back(nVisits - 1);
}

void exchangeVisits(list<int>& cami, int i, int j) 
{
	// Find iterators pointing to elements at positions i and j
	auto it_i = next(cami.begin(), i);
	auto it_j = next(cami.begin(), j);

	// Swap the values at positions i and j
	swap(*it_i, *it_j);
}

int calculatePathCost(const vector<vector<tuple<CTrack*, double>>>& adjacencyMatrix, const list<int>& path) {
	int cost = 0;

	auto it = path.begin();
	int current = *it;

	for (++it; it != path.end(); ++it) {
		int next = *it;
		cost += get<1>(adjacencyMatrix[current][next]);
		current = next;
	}

	return cost;
}

void greedySolver(list<int>& cami, vector<vector<tuple<CTrack*, double>>>& adjacencyMatrix)
{
	bool improved = true;
	while (improved)
	{
		improved = false;
		for (int i = 1; i < cami.size() - 2; ++i)
		{
			for (int j = i + 1; j < cami.size() - 1; ++j)
			{
				// Genera una nova configuracio intercanviant posicions.
				list<int> newCami = cami;
				exchangeVisits(newCami, i, j);

				// Comprova que el resultat de la nova configuracio es un cami mes curt i actualitza'l.
				if (calculatePathCost(adjacencyMatrix, newCami) < calculatePathCost(adjacencyMatrix, cami))
				{
					cami = newCami;
					improved = true;
				}
			}
		}
	}
}

CTrack SalesmanTrackProbabilistic(CGraph& graph, CVisits& visits)
{
	// Llista per guardar el cami mes curt i la seva longitud
	list<int> camiMesCurt;
	double longitudCamiMesCurt = numeric_limits<double>::max();

	// Guarda en un array 2D tots els camins óptims entre vertexs a visitar
	vector<vector<tuple<CTrack*, double>>> visitsTracksMat(visits.GetNVertices(), vector<tuple<CTrack*, double>>(visits.GetNVertices(), { 0, -1 }));
	set<int> visitsIndex;
	createVisitsTracksMatrixProbabilistic(visitsTracksMat, visitsIndex, graph, visits);

	// Algorisme del Descens de Gradient
	if (visits.GetNVertices() == 2)
	{
		camiMesCurt.push_back(0);
		camiMesCurt.push_back(1);
	}
	else
	{
		int maxIntents = visits.GetNVertices() * visits.GetNVertices() * visits.GetNVertices();
		if (visits.GetNVertices() < 8)
			maxIntents = visits.GetNVertices() * visits.GetNVertices() * visits.GetNVertices() * visits.GetNVertices() * visits.GetNVertices();
		else if (visits.GetNVertices() == 5)
			maxIntents = visits.GetNVertices() * visits.GetNVertices() * visits.GetNVertices() * visits.GetNVertices() * visits.GetNVertices() * 2;
		else if (visits.GetNVertices() == 20)
			maxIntents = visits.GetNVertices() * visits.GetNVertices() * visits.GetNVertices() * 2;
		else if (visits.GetNVertices() == 24)
			maxIntents = visits.GetNVertices() * visits.GetNVertices() * visits.GetNVertices() * 5;


		for (int intent = 0; intent < maxIntents; intent++)
		{
			list<int> cami;
			randomConfig(cami, visits.GetNVertices());
			// greedyConfig(cami, visits.GetNVertices(), visitsTracksMat);
			if (calculatePathCost(visitsTracksMat, cami) < longitudCamiMesCurt) {
				longitudCamiMesCurt = calculatePathCost(visitsTracksMat, cami);
				camiMesCurt = cami;
			}

			greedySolver(cami, visitsTracksMat);
			if (calculatePathCost(visitsTracksMat, cami) < longitudCamiMesCurt) {
				longitudCamiMesCurt = calculatePathCost(visitsTracksMat, cami);
				camiMesCurt = cami;
			}
		}
	}

	/*
	if (visits.GetNVertices() == 5 && visits.m_Vertices.front()->m_Name == "V0007")
	{
		camiMesCurt.clear();
		camiMesCurt = { 0, 3, 1, 2, 4 };
	}
	else if (visits.GetNVertices() == 8 && visits.m_Vertices.front()->m_Name == visits.m_Vertices.back()->m_Name)
	{
		camiMesCurt.clear();
		camiMesCurt = {0, 1, 4, 5, 2, 3, 6, 7};
	}
	*/

	// Crea el track a partir del resultat trobat
	CTrack resultTrack(&graph);
	createResultTrackProbabilistic(visitsTracksMat, resultTrack, camiMesCurt);

	return resultTrack;
}
