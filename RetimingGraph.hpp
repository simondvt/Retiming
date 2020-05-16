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
	weightWD wwd; // used in WD algorithm
};

// Struct which contains (u, v, D(u, v))
// Used in OPT1 algorithm to sort D matrix and perform binary search on it
struct dElements
{
	int src, dest, D;
};

// the graph is directed, but I use bidirectional so that I can use "in_edges" in CP algorithm
using RGraph = adjacency_list<vecS, vecS, bidirectionalS, VertexData, EdgeData>;
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
	RGraph originalGraph;
	adjacency_list<vecS, vecS, directedS, no_property, property<edge_weight_t, int>> constraintGraph;

public:
	void addVertex(int d);
	void addEdge(Vertex_d src, Vertex_d dest, int w);

	void printGraph();
	// Given a retiming r: V -> Z, it applies it to the graph
	// If undo == true it undoes the retiming
	void applyRetiming(const std::vector<int>& r, bool undo = false);

	// Compute the clock period of a circuit
	int CP(std::vector<int>& delta);
	// Compute W and D
	void WD(int **W, int **D);
	// Clock period minimization
	void OPT(bool opt = true);
	// Feasible clock period test
	std::vector<int> FEAS(int c);
	// OPT1
	std::vector<int> OPT1(int **W, int **D, std::vector<dElements>& dE, bool cmp(dElements first, dElements second));
	// OPT2
	std::vector<int> OPT2(int** W, int** D, std::vector<dElements>& dE);
};

