#pragma once
#include <algorithm>
#include <limits>
#include <queue>
#include <vector>

namespace EnhancedRoute {
// Read-only graph search: never uses the engine's shared AI search scratch.
// Goal penalties allow several nearby road entrances instead of insisting on
// an unreachable node on the other side of a wall, bridge or one-way street.
template<class Graph>
std::vector<int> Search(const Graph &graph, int start)
{
    const float infinity = std::numeric_limits<float>::infinity();
    const int count = graph.NodeCount();
    if (start < 0 || start >= count) return {};
    struct Entry {
        float cost;
        int node;
        bool operator<(const Entry &other) const { return cost > other.cost; }
    };
    std::vector<float> distance(count, infinity);
    std::vector<int> previous(count, -1);
    std::priority_queue<Entry> open;
    distance[start] = 0;
    open.push({0, start});
    int goal = -1;
    float best = infinity;
    while (!open.empty()) {
        const Entry current = open.top();
        open.pop();
        if (current.cost != distance[current.node]) continue;
        if (current.cost > best) break;
        const float total = current.cost + graph.GoalPenalty(current.node);
        if (total < best) { best = total; goal = current.node; }
        graph.VisitEdges(current.node, [&](int next, float weight) {
            if (next < 0 || next >= count || !(weight > 0)) return;
            const float candidate = current.cost + weight;
            if (candidate >= distance[next]) return;
            distance[next] = candidate;
            previous[next] = current.node;
            open.push({candidate, next});
        });
    }
    std::vector<int> result;
    if (goal < 0) return result;
    for (int node = goal; node != -1; node = previous[node])
        result.push_back(node);
    std::reverse(result.begin(), result.end());
    return result;
}
}
