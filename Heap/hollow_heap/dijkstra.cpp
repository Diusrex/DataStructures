#include "hollow_heap_base.h"

#include <chrono>
#include <iostream>
#include <limits>
#include <queue>
#include <vector>

using namespace std;

// TODO: This should have its own folder in Heap directory, and be able to run
// hollow heap and pairing heap.

// Assumes the data is provided through input stream, in format:
// N E
// One line per edge X->Y

// Also assumes the graph is 0 indexed

// Assumes undirected graph.


const int Inf = numeric_limits<int>::max();

struct Edge {
    int to;
    int weight;
};

vector<vector<Edge>> GenerateGraph(int N, int E) {
    vector<vector<Edge>> edges(N);

    vector<vector<bool>> connected(N, vector<bool>(N, false));
    // To make things easier.
    connected[0][0] = true;
    for (int e = 0; e < E; ++e) {
        // 0-0 is always considered to be connected!
        int from = 0;
        int to = 0;

        while (connected[from][to]) {
            from = rand() % N;
            to = from;
            while (to == from) {
                to = rand() % N;
            }
        }

        int cost = 1 + rand() % 100000;

        connected[from][to] = true;
        edges[from].push_back({to, cost});
    }

    return edges;
}

// Distance will be numeric_limits<int>::max if node isn't reachable from startingNode.
vector<int> DijkstraKeyUpdate(const vector<vector<Edge>>& edges, int startingNode,
        hollow_heap_base<int, int> heap) {

    int N = edges.size();
    // Add all items with distance inf
    vector<int> distances(N, Inf);
    distances[startingNode] = 0;

    for (int node = 0; node < N; ++node) {
        if (node != startingNode) {
            heap.insert(node, Inf);
        } else {
            heap.insert(node, 0);
        }
    }

    while (heap.size() != 0 && heap.find_min().second != Inf) {
        const pair<int, int> next = heap.extract_min();
        const int node = next.first;
        const int d = next.second;

        for (const Edge& e : edges[node]) {
            int cost = d + e.weight;

            if (distances[e.to] > cost) {
                distances[e.to] = cost;
                heap.decrease_key(e.to, cost);
            }
        }
    }

    return distances;
}

// Distance will be numeric_limits<int>::max if node isn't reachable from startingNode.
vector<int> DijkstraReInsert(const vector<vector<Edge>>& edges, int startingNode) {
    // <-weight, node> to ensure organised by weight first.
    priority_queue<pair<int, int>> heap;

    int N = edges.size();
    // Add all items with distance inf
    vector<int> distances(N, Inf);
    distances[startingNode] = 0;

    heap.push(make_pair(0, startingNode));

    while (heap.size() != 0) {
        const pair<int, int> next = heap.top();
        heap.pop();

        const int node = next.second;
        const int d = -next.first;

        if (distances[node] > d) {
            continue;
        }

        for (const Edge& e : edges[node]) {
            int cost = d + e.weight;

            if (distances[e.to] > cost) {
                distances[e.to] = cost;
                heap.push(make_pair(-cost, e.to));
            }
        }
    }

    return distances;
}

enum class DijkstrasToUse {REINSERT, UPDATE_KEY_MULTI_ROOT, UPDATE_KEY_SINGLE_ROOT};
int main(int argc, char *argv[]) {
    if (argc < 3) {
        cout << "ERROR: Need argument `reinsert`, `updatekey_multiroot`, or `updatekey_singleroot`.\n";
        cout << "Then, `input` or `generated` to identify how the graph should be created.\n";
        cout << "Adding `verbose` after will cause graph related output to be printed.\n";
        return 1;
    }

    string run_type = argv[1];

    DijkstrasToUse dijkstras_to_use;
    if (run_type == "reinsert") {
        dijkstras_to_use = DijkstrasToUse::REINSERT;
    } else if (run_type == "updatekey_multiroot") {
        dijkstras_to_use = DijkstrasToUse::UPDATE_KEY_MULTI_ROOT;
    } else  if (run_type == "updatekey_singleroot") {
        dijkstras_to_use = DijkstrasToUse::UPDATE_KEY_SINGLE_ROOT;
    } else {
        cout << "ERROR: Need argument `reinsert`, `updatekey_multiroot`, or `updatekey_singleroot`.\n";
        cout << "Argument " << run_type  << " not recognized\n";
        return 1;
    }

    string graph_gen_type = argv[2];

    bool printOutput = false;
    if (argc >= 4 && string(argv[3]) == "verbose") {
        printOutput = true;
    } else if (argc >= 4) {
        cout << "Error: Unrecognised input " << argv[3] << '\n';
        return 1;
    }

    int N, E;
    cin >> N >> E;

    vector<vector<Edge>> edges;

    if (graph_gen_type == "input") {
        edges.resize(N);
        for (int i = 0; i < E; ++i) {
            int a, b, c;
            cin >> a >> b >> c;
            edges[a].push_back(Edge{b, c});
            edges[b].push_back(Edge{a, c});
        }
        std::cout << "Done loading edges.\n";
    } else {
        std::cout << "Generating!\n";
        edges = GenerateGraph(N, E);
        std::cout << "Done generate edges.\n";
    }
    
    chrono::milliseconds before =
        chrono::duration_cast< chrono::milliseconds >(
                chrono::system_clock::now().time_since_epoch());

    int source;
    while (cin >> source) {
        vector<int> distances;
        switch (dijkstras_to_use) {
        case DijkstrasToUse::REINSERT:
            distances = DijkstraReInsert(edges, source);
            break;

        case DijkstrasToUse::UPDATE_KEY_MULTI_ROOT:
            distances = DijkstraKeyUpdate(
                    edges, source, hollow_heap_base<int, int>{hollow_heap_type::MULTIPLE_ROOTS});
            break;

        case DijkstrasToUse::UPDATE_KEY_SINGLE_ROOT:
            distances = DijkstraKeyUpdate(
                    edges, source, hollow_heap_base<int, int>{hollow_heap_type::SINGLE_ROOT});
            break;
        }

        if (printOutput) {
            for (int i = 0; i < N; ++i) {
                cout << i << ": " << distances[i] << '\n';
            }
            cout << "\n\n";
        }
    }

    chrono::milliseconds after =
        chrono::duration_cast< chrono::milliseconds >(
                chrono::system_clock::now().time_since_epoch());

    cout << "Time for dijkstras (ms): " << (after - before).count() << '\n';
}
