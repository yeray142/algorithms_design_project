#include "pch.h"
#include "Graph.h"
// SalesmanTrackGreedy =========================================================
bool cmp(CVertex* V1, CVertex* V2) {
	return V1->m_DijkstraDistance < V2->m_DijkstraDistance;
}

CTrack SalesmanTrackGreedy(CGraph& graph, CVisits& visits)
{
	// Sigui v la primera visita de la llista de visites.
	CVertex* v = visits.m_Vertices.front();

	// Candidats= tots els vèrtexs de la llista de visites menys el primer i l'últim.
	list<CVertex*> candidats = visits.m_Vertices;

	candidats.pop_front();
	candidats.pop_back();

	CTrack t(&graph);
	// Repetir mentre candidats no estigui buit.
	while (!candidats.empty())
	{
		// Aplicant l’algorisme de Dijkstra calcular les distàncies de tots els vèrtexs del graf a v.
		DijkstraQueue(graph, v);

		// Seleccionar v1 com el vèrtex pertanyent a Candidats amb distància mínima.
		auto v1 = min_element(candidats.begin(), candidats.end(), cmp);

		// Afegir el camí de v a v1 al resultat.
		CTrack aux(&graph);
		CVertex* v1_aux = *v1;
		while (v1_aux != v) {
			aux.AddFirst(v1_aux->m_pDijkstraPrevious);
			v1_aux = v1_aux->m_pDijkstraPrevious->m_pOrigin;
		}
		t.Append(aux);

		// v = v1
		v = *v1;

		// Treure v1 de Candidats.
		candidats.erase(v1);
	}

	// Anar de l’últim candidat seleccionat al vèrtex final de la llista de visites.
	DijkstraQueue(graph, v);

	CTrack aux(&graph);
	CVertex* v1 = visits.m_Vertices.back();
	while (v1 != v) {
		aux.AddFirst(v1->m_pDijkstraPrevious);
		v1 = v1->m_pDijkstraPrevious->m_pOrigin;
	}
	t.Append(aux);

	return t;
}
