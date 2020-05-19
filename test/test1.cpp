/*
 * Tests for Correlator 1 (Figure 3, pag 5)
 */

#define BOOST_TEST_MODULE Correlator1
#include <boost/test/unit_test.hpp>

#include "../RetimingGraph.hpp"
#include <ctime>
#include <cstdlib>
#include <iostream>

BOOST_AUTO_TEST_CASE(random)
{
    for (int t = 0; t < 50; t++)
    {
        srand(t*100);

        RetimingGraph rg;
        const int V = 20, E = 50;
        const int maxValue = 500;

        for (int i = 0; i < V; i++)
        {
            rg.addVertex(rand() % maxValue);
        }
        for (int i = 0; i < E; i++)
        {
            rg.addEdge(rand() % V, rand() % V, rand() % maxValue);
        }

        int originalCP = rg.CP();
        rg.OPT(RetimingGraph::optEnum::OPT1);
        int remitedCP = rg.CP();

        BOOST_CHECK_LE(remitedCP, originalCP);
    }
}

BOOST_AUTO_TEST_CASE(Correlator1CP)
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

    BOOST_CHECK_EQUAL(rg.CP(), 24);
}

BOOST_AUTO_TEST_CASE(Correlator1OPT1)
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

    rg.OPT(RetimingGraph::optEnum::OPT1);

    BOOST_CHECK_EQUAL(rg.CP(), 13);
}

BOOST_AUTO_TEST_CASE(Correlator1OPT2)
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

    rg.OPT(RetimingGraph::optEnum::OPT1);

    BOOST_CHECK_EQUAL(rg.CP(), 13);
}