#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>

#include "RetimingGraph.hpp"

#include <iostream>
#include <iterator>
#include <algorithm>
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

// Struct which contains (u, v, D(u, v))
// Used in OPT1 algorithm to sort D matrix and perform binary search on it
struct dElements
{
	int src, dest, D;
};

void RetimingGraph::addVertex(int d)
{
	add_vertex(VertexData{ d }, originalGraph);
}

void RetimingGraph::addEdge(Vertex_d src, Vertex_d dest, int w)
{
	add_edge(src, dest, EdgeData{ w }, originalGraph);
}

void RetimingGraph::printGraph()
{
	print_graph(originalGraph);
}

/* Algorithm CP (page 7)
 *
 * Compute the clock period of a circuit
*/
int RetimingGraph::CP(void)
{	
	// Step 1
	filtered_graph<RGraph, ZeroEdgePredicate> filteredGraph(originalGraph, ZeroEdgePredicate(originalGraph));

	// Step 2
	std::vector<Vertex_d> vertices;
	topological_sort(filteredGraph, std::back_inserter(vertices));

	// Step 3
	std::vector<int> delta(vertices.size());
	for (std::vector<Vertex_d>::reverse_iterator v = vertices.rbegin(); v != vertices.rend(); ++v)
	{
		int maximum = 0;

		graph_traits<decltype(filteredGraph)>::in_edge_iterator eStart, eEnd;
		for (tie(eStart, eEnd) = in_edges(*v, filteredGraph); eStart != eEnd; ++eStart)
		{
			auto src = source(*eStart, filteredGraph);
			maximum = std::max(maximum, delta[src]);
		}

		delta[*v] = originalGraph[*v].d + maximum;
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
	for (tie(eStart, eEnd) = edges(originalGraph); eStart != eEnd; ++eStart)
	{
		auto d_src = originalGraph[source(*eStart, originalGraph)].d;
		originalGraph[*eStart].wwd.we = originalGraph[*eStart].w;
		originalGraph[*eStart].wwd.du = -d_src;
	}

	// Step 2
	int V = num_vertices(originalGraph);
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
	johnson_all_pairs_shortest_paths(originalGraph, distance_matrix, weight_map(get(&EdgeData::wwd, originalGraph)).
									 distance_zero(zero).distance_inf(inf).
									 distance_compare(cmp).distance_combine(plus));

	// Step 3
	for (int i = 0; i < V; i++)
	{
		for (int j = 0; j < V; j++)
		{
			W[i][j] = distance_matrix[i][j].we;
			D[i][j] = originalGraph[j].d - distance_matrix[i][j].du;
		}
	}

	for (int i = 0; i < V; i++)
		free(distance_matrix[i]);
	free(distance_matrix);
}

/* Algorithm OPT1 (page 14)
 *
 * Compute a legal retiming so to minimize the clock period
*/
void RetimingGraph::OPT1(void)
{
	// Step 1
	int V = num_vertices(originalGraph);
	int **W, **D;
	W = (int**)malloc(V * sizeof(*W));
	D = (int**)malloc(V * sizeof(*D));
	for (int i = 0; i < V; i++)
	{
		W[i] = (int*)malloc(V * sizeof(**W));
		D[i] = (int*)malloc(V * sizeof(**D));
	}
	WD(W, D);

	// Step 2
	std::vector<dElements> dE;
	for (int i = 0; i < V; i++)
		for (int j = 0; j < V; j++)
			dE.push_back({ i, j, D[i][j] });

	auto cmp = [](dElements first, dElements second)
	{
		return first.D < second.D;
	};
	std::sort(dE.begin(), dE.end(), cmp);
	
	// Step 3
	std::vector<dElements>::iterator it;
	for (it = dE.begin(); it != dE.end(); ++it)
	{
		constraintGraph.clear();

		// STL's binary search is not suitable here beacuse it does not return a pointer
		// to the maching elements, while upper_bound does (always in O(log n))
		auto startRange = std::upper_bound(dE.begin(), dE.end(), *it, cmp); // returns iterator to first element greater than it->D
		// add edge (v -> u) when D[u][v] > c
		for (; startRange != dE.end(); ++startRange)
			add_edge(startRange->dest, startRange->src, W[startRange->src][startRange->dest] - 1, constraintGraph);
		
		// add edge (v -> u) for every edge (u -> v) in original graph
		edgeIt eStart, eEnd;
		for (tie(eStart, eEnd) = edges(originalGraph); eStart != eEnd; ++eStart)
		{
			auto src = source(*eStart, originalGraph);
			auto dest = target(*eStart, originalGraph);
			int originalWeight = originalGraph[*eStart].w;

			auto e = edge(dest, src, constraintGraph);
			if (e.second) // if edge already exists in contraintGraph
			{
				int existingWeight = get(edge_weight_t(), constraintGraph, e.first);
				put(edge_weight_t(), constraintGraph, e.first, std::min(existingWeight, originalWeight));
			}
			else
			{
				add_edge(dest, src, originalWeight, constraintGraph);
			}
		}

		// add one more vertex with zero edges 
		// lastVertex has the biggest vertex descriptor
		auto lastVertex = add_vertex(constraintGraph);
		for (int i = 0; i < lastVertex; i++)
			add_edge(lastVertex, i, 0, constraintGraph);

		// bellman_ford for constraintGraph (https://www.oreilly.com/library/view/vlsi-digital-signal/9780471241867/sec-4.3.html)
		int VconstraintGraph = num_vertices(constraintGraph);
		std::vector<int> distance(VconstraintGraph, std::numeric_limits<int>::max());
		distance[lastVertex] = 0; // use lastVertex as the source at distance 0

		// Step 4
		if (bellman_ford_shortest_paths(constraintGraph, VconstraintGraph, distance_map(&distance[0])))
		{
			// apply retiming with clock period c = it->D to original graph
			edgeIt eStart, eEnd;
			for (tie(eStart, eEnd) = edges(originalGraph); eStart != eEnd; ++eStart)
			{
				auto src = source(*eStart, originalGraph);
				auto dest = target(*eStart, originalGraph);
				originalGraph[*eStart].w += distance[dest] - distance[src];
			}

			break;
		}
	}

	for (int i = 0; i < V; i++)
	{
		free(W[i]);
		free(D[i]);
	}
	free(W);
	free(D);
}
