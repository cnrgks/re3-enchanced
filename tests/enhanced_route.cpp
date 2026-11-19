#include "../src/extras/EnhancedRoute.h"
#include <cstdio>
#include <cstdlib>

struct Graph {
    struct Edge { int to; float cost; };
    std::vector<std::vector<Edge>> edges;
    std::vector<bool> closed;
    std::vector<float> goals;
    explicit Graph(int count) : edges(count), closed(count, false),
        goals(count, std::numeric_limits<float>::infinity()) {}
    int NodeCount() const { return int(edges.size()); }
    float GoalPenalty(int node) const { return goals[node]; }
    template<class Visitor> void VisitEdges(int node, Visitor visit) const {
        if (closed[node]) return;
        for (const Edge &edge : edges[node])
            if (!closed[edge.to]) visit(edge.to, edge.cost);
    }
};
static void require(bool ok, const char *message)
{
    if (!ok) { std::fprintf(stderr, "FAILED: %s\n", message); std::exit(1); }
}
int main()
{
    Graph city(7);
    city.edges[0]={{1,1},{2,4}};
    city.edges[1]={{3,1}};
    city.edges[2]={{3,1},{4,2}};
    city.edges[3]={{5,1}};
    city.edges[4]={{5,1}};
    city.goals[5]=0;
    require(EnhancedRoute::Search(city,0)==std::vector<int>({0,1,3,5}),"shortest connected road route");
    city.closed[1]=true;
    require(EnhancedRoute::Search(city,0)==std::vector<int>({0,2,3,5}),"closed road forces detour");
    city.closed[3]=true;
    require(EnhancedRoute::Search(city,0)==std::vector<int>({0,2,4,5}),"second closure avoids original junction");
    require(EnhancedRoute::Search(city,5).size()==1,"already at target");
    city.goals[5]=std::numeric_limits<float>::infinity();
    city.goals[0]=0;
    require(EnhancedRoute::Search(city,5).empty(),"one-way roads cannot be traversed backwards");
    city.goals[0]=std::numeric_limits<float>::infinity();
    city.goals[6]=0;
    require(EnhancedRoute::Search(city,0).empty(),"disconnected target has no fake connector");
    city.goals[4]=8;
    require(EnhancedRoute::Search(city,0)==std::vector<int>({0,2,4}),"reachable alternative entrance");
    require(EnhancedRoute::Search(city,-1).empty(),"invalid start");

    Graph entrances(3);
    entrances.edges[0]={{1,1},{2,100}};
    entrances.goals[1]=entrances.goals[2]=0;
    const auto clearEntrance = [](int node) { return node == 2; };
    require(EnhancedRoute::Search(entrances,0,clearEntrance)==std::vector<int>({0,2}),
        "clear entrance wins over a closer route ending at a wall");
    entrances.closed[2]=true;
    require(EnhancedRoute::Search(entrances,0,clearEntrance)==std::vector<int>({0,1}),
        "unreachable clear entrance leaves a reachable road endpoint");
    Graph longRoad(1200);
    for (int i=0;i<1199;++i) longRoad.edges[i].push_back({i+1,1});
    longRoad.goals[1199]=0;
    const auto route=EnhancedRoute::Search(longRoad,0);
    require(route.size()==1200 && route.back()==1199,"long path is not truncated to 512 points");
    std::puts("PASS: shortest paths, closures, one-way roads, alternate entrances, disconnected and long routes");
}
