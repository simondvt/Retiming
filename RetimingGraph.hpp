#ifndef RETIMING_GRAPH
#define RETIMING_GRAPH

#include <boost/graph/adjacency_list.hpp> 
using namespace boost;

// weighing for WD algorithm
struct weightWD
{
	int we, du;

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
	int w;	      // number of registers
	weightWD wwd; // used in WD algorithm
};

// Struct which contains (u, v, D(u, v))
// Used in OPT1 algorithm
struct dElements
{
	int src, dest, D;
};

// the graph is directed, but I use bidirectional so that I can use "in_edges" in CP algorithm at constant cost
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

public:
	void addVertex(int d);
	void addEdge(Vertex_d src, Vertex_d dest, int w);
	void printGraph();
	void saveDOT(std::string path);

	// Given a retiming r: V -> Z, it applies it to the graph
	// If undo == true it undoes the retiming
	void applyRetiming(const std::vector<int>& r, bool undo = false);

	// Compute the clock period of a circuit
	int CP(std::vector<int>& delta);
	int CP(void);

	// Compute W and D
	void WD(int** W, int** D);

	enum class optEnum {OPT1, OPT2};
	// Clock period minimization
	void OPT(optEnum opt);

	// OPT2 clock period test
	std::vector<int> FEAS(int c);
	// OPT1 clock period test
	std::vector<int> OPT1(int** W, std::vector<dElements>& dE, int c, bool cmp(dElements first, dElements second));
};

#endif // RETIMING_GRAPH