#include "pch.h"
#include "Graph.h"
#include <queue>
#include <iostream>
#include <iomanip> 
#include <tuple> 
#include <set>

// Funcions generals ==============================================================
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


// SalesmanTrackBranchAndBound1 ===================================================
class CBB1 {
public:
	CBB1* m_pFatherBB1;
	int m_VisitaBB1;
	double m_LengthBB1;

	// Llista vertex a visitar
	set<int> m_ToVisitBB1;

	// Constructor vertex inicial
	CBB1(int visita, double length, set<int>& toVisit)
		: m_pFatherBB1(nullptr)
		, m_LengthBB1(length)
		, m_VisitaBB1(visita)
		, m_ToVisitBB1(toVisit)
	{
	}

	// Constructor resta de vertexs
	CBB1(CBB1* pFather, int visita, double length)
		: m_pFatherBB1(pFather)
		, m_VisitaBB1(visita)
		, m_LengthBB1(pFather->m_LengthBB1 + length)
	{
		m_ToVisitBB1 = pFather->m_ToVisitBB1;
		m_ToVisitBB1.erase(visita);
	}
};

struct comparatorCBB1 {
	bool operator()(const CBB1* s1, const CBB1* s2) {
		return s1->m_LengthBB1 > s2->m_LengthBB1;
	}
};

CTrack SalesmanTrackBranchAndBound1(CGraph& graph, CVisits& visits) {
	// Llista per guardar el cami mes curt i la seva longitud
	list<int> camiMesCurtBB1;
	double longitudCamiMesCurtBB1 = numeric_limits<double>::max();

	// Guarda en un array 2D tots els camins óptims entre vertexs a visitar
	vector<vector<tuple<CTrack*, double>>> visitsTracksMat(visits.GetNVertices(), vector<tuple<CTrack*, double>>(visits.GetNVertices(), { 0, -1 }));
	set<int> visitsIndex;
	createVisitsTracksMatrix(visitsTracksMat, visitsIndex, graph, visits);

	// Calcula cotes i nodes a visitar pel node inicial
	visitsIndex.erase(visitsIndex.begin());
	CBB1 nodeStart(0, 0, visitsIndex);

	// Afegeix node inicial a la cua de prioritat
	priority_queue<CBB1*, vector<CBB1*>, comparatorCBB1> queue;
	queue.push(&nodeStart);

	while (!queue.empty()) {
		// Treu de la cua el node amb la longitud mínima
		CBB1* pN = queue.top();
		queue.pop();

		for (int v : pN->m_ToVisitBB1) {
			// Si es la última visita i no ha visitat totes les anteriors, poda
			if (pN->m_ToVisitBB1.size() > 1 && v == visits.GetNVertices() - 1) {
				continue;
			}

			// Poda si la longitud actual es superior a la del cami mes curt trobat
			if (pN->m_LengthBB1 + get<1>(visitsTracksMat[pN->m_VisitaBB1][v]) > longitudCamiMesCurtBB1) {
				continue;
			}

			// Si es solució, actualiza longitud de cami mes curt i la seva llista de visites 
			if (v == visits.m_Vertices.size() - 1) {
				CBB1* pAnterior = pN;
				camiMesCurtBB1.clear();
				camiMesCurtBB1.push_front(v);
				while (pAnterior) {
					camiMesCurtBB1.push_front(pAnterior->m_VisitaBB1);
					pAnterior = pAnterior->m_pFatherBB1;
				}
				longitudCamiMesCurtBB1 = pN->m_LengthBB1 + get<1>(visitsTracksMat[pN->m_VisitaBB1][v]);
				continue;
			}

			// Crea nou CBB1
			CBB1* nodeFill = new CBB1(pN, v, get<1>(visitsTracksMat[pN->m_VisitaBB1][v]));

			// Afegeix node a la cua de prioritat
			queue.push(nodeFill);
		}
	}

	// Crea el track a partir del resultat trobat
	CTrack resultTrackBB1(&graph);
	createResultTrack(visitsTracksMat, resultTrackBB1, camiMesCurtBB1);

	return resultTrackBB1;
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

CTrack SalesmanTrackBranchAndBound2(CGraph& graph, CVisits &visits)
{
	// Llista per guardar el cami mes curt i la seva longitud.
	list<int> camiMesCurt;
	double longitudCamiMesCurt = numeric_limits<double>::max();
	double cotaSuperiorMinima = numeric_limits<double>::max();

	// Guardar en un array de dues dimensions tots els camins òptims entre vèrtexs a visitar.
	vector<vector<tuple<CTrack*, double>>> visitsTracksMat(visits.GetNVertices(), vector<tuple<CTrack*, double>>(visits.GetNVertices(), { 0, -1 }));
	set<int> visitsIndex;
	createVisitsTracksMatrix(visitsTracksMat, visitsIndex, graph, visits);
	
	// Precalcular minMaxCVj
	vector<tuple<double, double >> minMaxCVj(visits.GetNVertices(), tuple<double, double>(numeric_limits<double>::max(), 0));
	for (int j = 0; j < visitsTracksMat.size(); j++)
	{
		for (int i = 0; i < visitsTracksMat.size() - 1; i++)
		{
			// Si es del mateix al mateix node, no tenir en compte.
			if (i == j) continue;

			// Si es del primer a l'ultim node, no tenir en compte.
			if (i == 0 && j == visitsTracksMat.size() - 1) continue;

			// Ajustar valors per a CVj.
			if (get<1>(visitsTracksMat[i][j]) > get<1>(minMaxCVj[j])) get<1>(minMaxCVj[j]) = get<1>(visitsTracksMat[i][j]);
			if (get<1>(visitsTracksMat[i][j]) < get<0>(minMaxCVj[j])) get<0>(minMaxCVj[j]) = get<1>(visitsTracksMat[i][j]);
		}
	}

	// Calcular cotes i nodes a visitar per al node inicial.
	visitsIndex.erase(visitsIndex.begin());
	CBB2 nodeStart(0, 0, visitsIndex);
	double cotaSuperior = 0.0, cotaInferior = 0.0;
	for (int j : nodeStart.m_ToVisit)
	{
		cotaSuperior += get<1>(minMaxCVj[j]);
		cotaInferior += get<0>(minMaxCVj[j]);
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
			cotaSuperior = pN->m_CotaSuperior - get<1>(minMaxCVj[v]) + get<1>(visitsTracksMat[pN->m_Visita][v]);
			cotaInferior = pN->m_CotaInferior - get<0>(minMaxCVj[v]) + get<1>(visitsTracksMat[pN->m_Visita][v]);
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
void minMaxCota(double &min, double &max, vector<vector<tuple<CTrack*, double>>> &visitsTracksMat, CBB2* node)
{
	min = 0.0;
	max = 0.0;

	for (int j : node->m_ToVisit) 
	{
		double minAux = numeric_limits<double>::max();
		double maxAux = 0.0;

		for (int i : node->m_ToVisit)
		{
			// Si i es l'ultim node, no tenir en compte.
			if (i == visitsTracksMat.size() - 1) continue;

			// Si es del mateix al mateix node, no tenir en compte.
			if (i == j) continue;

			// Si es del primer a l'ultim node, no tenir en compte.
			if (i == 0 && j == visitsTracksMat.size() - 1) continue;

			// Trobar maxims i minims de columnes.
			double distance = get<1>(visitsTracksMat[i][j]);
			if (distance > maxAux) maxAux = distance;
			if (distance < minAux) minAux = distance;
		}

		// Sumar del node actual als nodes futurs.
		if (node->m_ToVisit.size() > 1) 
		{
			if (j != visitsTracksMat.size() - 1) {
				double distance = get<1>(visitsTracksMat[node->m_Visita][j]);
				if (distance > maxAux) maxAux = distance;
				if (distance < minAux) minAux = distance;
			}
		}
		else {
			double distance = get<1>(visitsTracksMat[node->m_Visita][j]);
			if (distance > maxAux) maxAux = distance;
			if (distance < minAux) minAux = distance;
		}


		min += minAux;
		max += maxAux;
	}
}


CTrack SalesmanTrackBranchAndBound3(CGraph& graph, CVisits &visits)
{
	// Llista per guardar el cami mes curt i la seva longitud.
	list<int> camiMesCurt;
	double longitudCamiMesCurt = numeric_limits<double>::max();
	double cotaSuperiorMinima = numeric_limits<double>::max();

	// Guardar en un array de dues dimensions tots els camins òptims entre vèrtexs a visitar.
	vector<vector<tuple<CTrack*, double>>> visitsTracksMat(visits.GetNVertices(), vector<tuple<CTrack*, double>>(visits.GetNVertices(), { 0, -1 }));
	set<int> visitsIndex;
	createVisitsTracksMatrix(visitsTracksMat, visitsIndex, graph, visits);

	// Calcular cotes i nodes a visitar per al node inicial.
	visitsIndex.erase(visitsIndex.begin());
	CBB2 nodeStart(0, 0, visitsIndex);
	double cotaSuperior = 0.0, cotaInferior = 0.0;
	minMaxCota(cotaInferior, cotaSuperior, visitsTracksMat, &nodeStart);
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


			// Crear nova instancia de node CBB2.
			CBB2* nodeFill = new CBB2(pN, v, get<1>(visitsTracksMat[pN->m_Visita][v]));

			// Calcular cotes inferiors i superiors.
			double min, max;
			minMaxCota(min, max, visitsTracksMat, nodeFill);
			cotaSuperior = nodeFill->m_Length + max;
			cotaInferior = nodeFill->m_Length + min;
			if (cotaSuperior < cotaSuperiorMinima) cotaSuperiorMinima = cotaSuperior;

			nodeFill->m_CotaInferior = cotaInferior;
			nodeFill->m_CotaSuperior = cotaSuperior;

			// Podar si la cota inferior es superior a la cota superior minima o si la cota inferior es superior a la longitud del cami mes curt.
			if (cotaInferior > cotaSuperiorMinima + 1e-5 || cotaInferior > longitudCamiMesCurt) {
				// nodeFill->Unlink();
				continue;
			}

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
