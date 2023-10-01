#include "pch.h"
#include "Graph.h"
#include <queue>

// =============================================================================
// Dijkstra ====================================================================
// =============================================================================

void Dijkstra(CGraph& graph, CVertex *pStart)
{
}

// =============================================================================
// DijkstraQueue ===============================================================
// =============================================================================

struct Vertex
{
    CVertex* vertex;
    double distance;

    Vertex(CVertex* v, double d) : vertex(v), distance(d) {}
};

struct comparator {
    bool operator()(Vertex V1, Vertex V2) {
        return V1.distance > V2.distance;
    }
};

void DijkstraQueue(CGraph& graph, CVertex *pStart)
{
    for (auto& vertex : graph.m_Vertices) {
        // Inicialitzar les distàncies dels vèrtexs a infinit excepte la del vèrtex start que serà 0
        vertex.m_DijkstraDistance = numeric_limits<double>::max();
        // Marcar tots el vèrtex com no visitats (bool m_DijkstraVisit).
        vertex.m_DijkstraVisit = false;
    }
    pStart->m_DijkstraDistance = 0;

    // Posar a la cua el “vèrtex” start
    priority_queue<Vertex, vector<Vertex>, comparator> queue;
    queue.push(Vertex(pStart, 0));

    // Mentre cua no buida
    while (!queue.empty()) {
        // va=primer element de la cua i treure'l de la cua.
        Vertex va = queue.top();
        CVertex* pva = va.vertex;
        queue.pop();

        // Si no està marcat com visitat
        if (!pva->m_DijkstraVisit) {
            // Recorre tots els veïns v de va i fer el següent
            for (CEdge* e : pva->m_Edges) {
                /*
                Si la distància de v és més grossa que la distància del vèrtex actual més la longitud de l’aresta que
                els uneix, actualitzar la distància de v amb la distancia de va més la longitud de l’aresta que els
                uneix i posar - lo a la cua.
                */
                CVertex* v = e->m_pDestination;
                double new_distance = pva->m_DijkstraDistance + e->m_Length;
                if (v->m_DijkstraDistance > new_distance) {
                    v->m_DijkstraDistance = new_distance;
                    queue.push(Vertex(v, new_distance));
                }
            }

            //Marcar va com visitat
            pva->m_DijkstraVisit = true;
        }
    }
}
