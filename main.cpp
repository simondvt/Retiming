#include <iostream>
using namespace std;

#include "RetimingGraph.hpp"

int main()
{
    RetimingGraph rg;
    // Correlator 1 (Figure 3, pag 5)
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

    cout << "CP: " << rg.CP() << endl;
    rg.printGraph();
    rg.OPT(false);
    cout << "Retimed CP: " << rg.CP() << endl << endl;
    rg.printGraph();
    rg.saveDOT("graph.dot");
    

    return 0;
}