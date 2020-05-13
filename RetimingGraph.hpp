#pragma once

#include <boost/graph/adjacency_list.hpp> 
using namespace boost;

// Vertex bundle property
struct VertexData
{
	unsigned int d;	// gate's delay
};

// Edge bundle property
struct EdgeData
{
	unsigned int w; // number of registers
};


// the graph is directed, but I use bidirectional so that I can use "in_edges" in CP algorithm
using RGraph = boost::adjacency_list<vecS, vecS, bidirectionalS, VertexData, EdgeData>;
// Vertex descriptor
using Vertex_d = graph_traits<RGraph>::vertex_descriptor;
// Edge descriptor
using Edge_d = graph_traits<RGraph>::edge_descriptor;

// Vertex iterator
using vertexIt = RGraph::vertex_iterator;
// Edge iterator
using edgeIt = RGraph::edge_iterator;
// Adjacent vertex iterator
using adjacentIt = RGraph::adjacency_iterator;

class RetimingGraph
{
private:
	RGraph g;

public:
	void addVertex(unsigned int d);
	void addEdge(Vertex_d src, Vertex_d dest, unsigned int w);

	void printEdges();

	// Compute the clock period of a circuit
	unsigned int CP(void);
};

