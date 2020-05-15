#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>

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

void RetimingGraph::addVertex(int d)
{
	add_vertex(VertexData{ d }, g);
}

void RetimingGraph::addEdge(Vertex_d src, Vertex_d dest, int w)
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
int RetimingGraph::CP(void)
{	
	// Step 1
	filtered_graph<RGraph, ZeroEdgePredicate> filteredGraph(g, ZeroEdgePredicate(g));
	print_graph(filteredGraph);

	// Step 2
	std::vector<Vertex_d> vertices;
	topological_sort(filteredGraph, std::back_inserter(vertices));

	// Step 3
	std::vector<int> delta(vertices.size());
	for (std::vector<Vertex_d>::reverse_iterator v = vertices.rbegin(); v != vertices.rend(); ++v)
	{
		int maximum = 0;

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

/* Algorithm WD (page 12)
 *
 * Compute W and D
*/
void RetimingGraph::WD(int** W, int** D)
{
	// Step 1
	edgeIt eStart, eEnd;
	for (tie(eStart, eEnd) = edges(g); eStart != eEnd; ++eStart)
	{
		auto d_src = g[source(*eStart, g)].d;
		g[*eStart].wwd.we = g[*eStart].w;
		g[*eStart].wwd.du = -d_src;
	}

	// Step 2
	int V = num_vertices(g);
	weightWD** distance_matrix;
	distance_matrix = (weightWD**) malloc(V * sizeof(*distance_matrix));
	for (int i = 0; i < V; i++)
		distance_matrix[i] = (weightWD*) malloc(V * sizeof(**distance_matrix));

	auto cmp = [](weightWD first, weightWD second)
	{
		if (first.we < second.we) return true;
		if (first.we == second.we && first.du < second.du) return true;

		return false;
	};

	auto plus = [](weightWD first, weightWD second)
	{
		weightWD res;
		res.we = first.we + second.we;
		res.du = first.du + second.du;

		return res;
	};

	const weightWD zero{ 0, 0 };
	const weightWD inf{ std::numeric_limits<int>::max(), std::numeric_limits<int>::max() };
	johnson_all_pairs_shortest_paths(g, distance_matrix, weight_map(get(&EdgeData::wwd, g)).
									 distance_zero(zero).distance_inf(inf).
									 distance_compare(cmp).distance_combine(plus));

	// Step 3
	for (int i = 0; i < V; i++)
	{
		for (int j = 0; j < V; j++)
		{
			W[i][j] = distance_matrix[i][j].we;
			D[i][j] = g[j].d - distance_matrix[i][j].du;
		}
	}

	for (int i = 0; i < V; i++)
		free(distance_matrix[i]);
	free(distance_matrix);
}
