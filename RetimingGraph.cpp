#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/graph_utility.hpp>

#include "RetimingGraph.hpp"

#include <iostream>
#include <iterator>
#include <vector>

using std::cout; using std::endl;

// Edge predicate for filtering in CP algorithm
// Only zero weight edges are allowed in the filtered graph
struct ZeroEdgePredicate
{
public:
	ZeroEdgePredicate() : g(0) {}
	ZeroEdgePredicate(RGraph& g_) : g(&g_) {}

	bool operator() (const Edge_d& e) const
	{
		return (*g)[e].w == 0;
	}

private:
	RGraph* g;
};

void RetimingGraph::addVertex(unsigned int d)
{
	add_vertex(VertexData{ d }, g);
}

void RetimingGraph::addEdge(Vertex_d src, Vertex_d dest, unsigned int w)
{
	add_edge(src, dest, EdgeData{ w }, g);
}

void RetimingGraph::printEdges()
{
	vertexIt vStart, vEnd;
	adjacentIt aStart, aEnd;

	for (tie(vStart, vEnd) = vertices(g); vStart != vEnd; ++vStart)
	{
		cout << *vStart << " is connected with ";
		tie(aStart, aEnd) = adjacent_vertices(*vStart, g);
		for (; aStart != aEnd; ++aStart)
			cout << *aStart << " ";
		cout << "\n";
	}
}

/* Algorithm CP (page 7)
 *
 * Compute the clock period of a circuit
*/
unsigned int RetimingGraph::CP(void)
{	
	// Step 1
	filtered_graph<RGraph, ZeroEdgePredicate> filteredGraph(g, ZeroEdgePredicate(g));
	print_graph(filteredGraph);

	// Step 2
	std::vector<Vertex_d> vertices;
	topological_sort(filteredGraph, std::back_inserter(vertices));

	// Step 3
	std::vector<unsigned int> delta(vertices.size());
	for (std::vector<Vertex_d>::reverse_iterator v = vertices.rbegin(); v != vertices.rend(); ++v)
	{
		unsigned int maximum = 0;

		cout << "\n\nVertice " << *v << "\nraggiunto da:" << endl;

		graph_traits<decltype(filteredGraph)>::in_edge_iterator eStart, eEnd;
		for (tie(eStart, eEnd) = in_edges(*v, filteredGraph); eStart != eEnd; ++eStart)
		{
			auto src = source(*eStart, filteredGraph);
			maximum = std::max(maximum, delta[src]);
			cout << src << '\t';
		}

		delta[*v] = g[*v].d + maximum;
	}

	// Step 4
	return *std::max_element(delta.begin(), delta.end());
}
