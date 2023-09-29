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
    for (auto it = begin(graph.m_Vertices); it != end(graph.m_Vertices); ++it) {
        // Inicialitzar les distàncies dels vèrtexs a infinit excepte la del vèrtex start que serà 0
        it->m_DijkstraDistance = numeric_limits<double>::max();
        // Marcar tots el vèrtex com no visitats (bool m_DijkstraVisit).
        it->m_DijkstraVisit = false;
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
                /* 
                Si la distància de v és més grossa que la distància del vèrtex actual més la longitud de l’aresta que
                els uneix, actualitzar la distància de v amb la distancia de va més la longitud de l’aresta que els
                uneix i posar - lo a la cua.
                */

            //Marcar va com visitat
            va->m_DijkstraVisit = true;
        }
    }
}
