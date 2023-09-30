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
    struct comparator {
        bool operator()(CVertex* pV1, CVertex* pV2) {
            return pV1->m_DijkstraDistance > pV2->m_DijkstraDistance;
        }
    };
    priority_queue<CVertex*, vector<CVertex*>, comparator> queue;
    queue.push(pStart);

    // Mentre cua no buida
    while (!queue.empty()) {
        // va=primer element de la cua i treure'l de la cua.
        CVertex* va = queue.top();
        queue.pop();

        // Si no està marcat com visitat
        if (!va->m_DijkstraVisit) {
            // Recorre tots els veïns v de va i fer el següent
            for (CEdge* e : va->m_Edges) {
                /*
                Si la distància de v és més grossa que la distància del vèrtex actual més la longitud de l’aresta que
                els uneix, actualitzar la distància de v amb la distancia de va més la longitud de l’aresta que els
                uneix i posar - lo a la cua.
                */
                CVertex* v = e->m_pDestination;
                if (v->m_DijkstraDistance > (va->m_DijkstraDistance + e->m_Length)) {
                    v->m_DijkstraDistance = va->m_DijkstraDistance + e->m_Length;
                    queue.push(v);
                }
            }

            //Marcar va com visitat
            va->m_DijkstraVisit = true;
        }
    }
}
