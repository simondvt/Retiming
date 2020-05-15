#pragma once

#include <boost/graph/adjacency_list.hpp> 
using namespace boost;


// weightning for WD algorithm
struct weightWD
{
	int we, du;

	//weightWD(int we_, int du_) : we(we_), du(du_) {}
	//weightWD() : we(0), du(0) {}

	//bool operator < (const weightWD& other) const
	//{
	//	if (we < other.we) return true;
	//	if (we == other.we && du < other.du) return true;
	//
	//	return false;
	//}

	//weightWD operator + (const weightWD& other) const
	//{
	//	weightWD res;
	//	res.we = we + other.we;
	//	res.du = du + other.du;
	//
	//	return res;
	//}

	bool operator == (const weightWD& other) const
	{
		return (we == other.we) && (du == other.du);
	}

	weightWD operator - (const weightWD& other) const
	{
		weightWD res;
		res.we = we - other.we;
		res.du = du - other.du;
		
		return res;
	}
};

// Vertex bundle property
struct VertexData
{
	int d;	// gate's delay
};

// Edge bundle property
struct EdgeData
{
	int w; // number of registers
	weightWD wwd;
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
	void addVertex(int d);
	void addEdge(Vertex_d src, Vertex_d dest, int w);

	void printEdges();

	// Compute the clock period of a circuit
	int CP(void);
	// Compute W and D
	void WD(int **W, int **D);
};

