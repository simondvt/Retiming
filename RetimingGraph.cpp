#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>
#include <boost/graph/bellman_ford_shortest_paths.hpp>
#include <boost/graph/graphviz.hpp>

#include "RetimingGraph.hpp"

#include <iostream>
#include <iterator>
#include <algorithm>
#include <vector>
#include <fstream>

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
	add_vertex(VertexData{ d }, originalGraph);
}

void RetimingGraph::addEdge(Vertex_d src, Vertex_d dest, int w)
{
	add_edge(src, dest, EdgeData{ w }, originalGraph);
}

void RetimingGraph::printGraph()
{
	edgeIt eStart, eEnd;
	for (tie(eStart, eEnd) = edges(originalGraph); eStart != eEnd; ++eStart)
	{
		auto src = source(*eStart, originalGraph);
		auto dest = target(*eStart, originalGraph);
		cout << src << " -> " << dest << " : " << originalGraph[*eStart].w << endl;
	}
}

void RetimingGraph::saveDOT(std::string path)
{
	std::filebuf fb;
	fb.open(path, std::ios::out);
	std::ostream os(&fb);

	write_graphviz(os, originalGraph,
				   make_label_writer(get(&VertexData::d, originalGraph)),
				   make_label_writer(get(&EdgeData::w, originalGraph)));
}

void RetimingGraph::applyRetiming(const std::vector<int>& r, bool undo)
{
	edgeIt eStart, eEnd;
	for (tie(eStart, eEnd) = edges(originalGraph); eStart != eEnd; ++eStart)
	{
		auto src = source(*eStart, originalGraph);
		auto dest = target(*eStart, originalGraph);

		if (undo == false)
			originalGraph[*eStart].w += r[dest] - r[src];
		else
			originalGraph[*eStart].w -= r[dest] - r[src];
	}
}

/* Algorithm CP (page 7)
 *
 * Compute the clock period of a circuit
*/
int RetimingGraph::CP(std::vector<int>& delta)
{
	// Step 1
	filtered_graph<RGraph, ZeroEdgePredicate> filteredGraph(originalGraph, ZeroEdgePredicate(originalGraph));

	// Step 2
	std::vector<Vertex_d> vertices;
	topological_sort(filteredGraph, std::back_inserter(vertices));

	// Step 3
	delta.resize((vertices.size()));
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

int RetimingGraph::CP(void)
{
	std::vector<int> dummyDelta;
	return CP(dummyDelta);
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
	distance_matrix = (weightWD**)malloc(V * sizeof(*distance_matrix));
	for (int i = 0; i < V; i++)
		distance_matrix[i] = (weightWD*)malloc(V * sizeof(**distance_matrix));

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

/* Algorithm OPT
 *
 * Runs either OPT1 or OPT2 based on how the retiming r is computed
 * opt == true runs OPT1
 * opt == false runs OPT2
*/
void RetimingGraph::OPT(bool opt)
{
	// Step 1
	int V = num_vertices(originalGraph);
	int** W, ** D;
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
	std::vector<int> r;
	r = opt ? OPT1(W, D, dE, cmp) : OPT2(dE);

	// Step 4
	applyRetiming(r);

	for (int i = 0; i < V; i++)
	{
		free(W[i]);
		free(D[i]);
	}
	free(W);
	free(D);
}

/* Algorithm FEAS (page 16)
 *
 * Feasible clock period test
*/
std::vector<int> RetimingGraph::FEAS(int c)
{
	// Step 1
	int V = num_vertices(originalGraph);
	std::vector<int> r(V, 0);

	// Step 2
	for (int i = 0; i < V - 1; i++)
	{
		// Step 2.1
		applyRetiming(r);

		// Step 2.2
		std::vector<int> delta;
		CP(delta);

		// must undo the retiming for the next being valid
		applyRetiming(r, true);

		// Step 2.3
		for (int v = 0; v < V; v++)
		{
			if (delta[v] > c)
				r[v]++;
		}
	}

	applyRetiming(r);
	int testC = CP();
	applyRetiming(r, true);

	// Step 3
	if (testC > c) // no feasible retiming exists
		return std::vector<int>();
	else
		return r;
}


/* Algorithm OPT1 (page 14)
 *
 * Clock period minimization
*/
std::vector<int> RetimingGraph::OPT1(int** W, int** D, std::vector<dElements>& dE, bool cmp(dElements first, dElements second))
{
	std::vector<dElements>::iterator it;
	for (it = dE.begin(); it != dE.end(); ++it)
	{
		constraintGraph.clear();

		// STL's binary search is not suitable here beacuse it does not return a pointer
		// to the matching elements, while upper_bound does (always in O(log n))
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
			return distance;
	}

	// no legal retiming found
	cout << "No legal retiming found!" << endl;
	return std::vector<int>();
}

/* Algorithm OPT2 (page 17)
 *
 * Clock period minimization
*/
std::vector<int> RetimingGraph::OPT2(std::vector<dElements>& dE)
{
	std::vector<int> r;
	std::vector<dElements>::iterator it;
	std::set<int> de;

	// add elements into an ordered set so that we don't check the same c more than once
	for (it = dE.begin(); it != dE.end(); ++it)
		de.insert(it->D);
	
	for (auto c : de)
	{
		r = FEAS(c);
		if (r.size() > 0)
			return r;
	}

	// no legal retiming found
	cout << "No legal retiming found!" << endl;
	return std::vector<int>();
}
