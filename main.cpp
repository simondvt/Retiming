#include <iostream>
#include <boost/graph/adjacency_list.hpp> 
#include <boost/graph/graph_utility.hpp>

using namespace boost;
using namespace std;

typedef boost::adjacency_list<vecS, vecS, directedS> mygraph;

#include "RetimingGraph.hpp"

int main()
{
    RetimingGraph rg;
    rg.addVertex(0); // Vh
    rg.addVertex(3); // V1
    rg.addVertex(3); // V2
    rg.addVertex(3); // V3
    rg.addVertex(3); // V4
    rg.addVertex(7); // V5
    rg.addVertex(7); // V6
    rg.addVertex(7); // V7
    rg.addEdge(0, 1, 1);
    rg.addEdge(1, 2, 1);
    rg.addEdge(2, 3, 1);
    rg.addEdge(3, 4, 1);
    rg.addEdge(1, 7, 0);
    rg.addEdge(2, 6, 0);
    rg.addEdge(3, 5, 0);
    rg.addEdge(4, 5, 0);
    rg.addEdge(5, 6, 0);
    rg.addEdge(6, 7, 0);
    rg.addEdge(7, 0, 0);
    
    rg.printEdges();
    cout << "filtered\n\n";
    cout << "CP: " << rg.CP() << endl;;
    

    return 0;
    /*mygraph g;
    add_edge(0, 1, g);
    add_edge(0, 3, g);
    add_edge(1, 2, g);
    add_edge(2, 3, g);
    mygraph::vertex_iterator vertexIt, vertexEnd;
    mygraph::adjacency_iterator neighbourIt, neighbourEnd;
    tie(vertexIt, vertexEnd) = vertices(g);
    for (; vertexIt != vertexEnd; ++vertexIt)
    {
        cout << *vertexIt << " is reached by ";
        tie(neighbourIt, neighbourEnd) = adjacent_vertices(*vertexIt, g);
        for (; neighbourIt != neighbourEnd; ++neighbourIt)
            cout << *neighbourIt << " ";
        cout << "\n";
    }*/
}