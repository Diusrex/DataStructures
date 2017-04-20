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
    if (argc < 2) {
        cout << "ERROR: Need argument `reinsert`, `updatekey_multiroot`, or `updatekey_singleroot`.\n";
        cout << "Adding `less_out` after will ensure no graph related output is printed.\n";
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

    bool printOutput = true;
    if (argc >= 3 && string(argv[2]) == "less_out") {
        printOutput = false;
    } else if (argc >= 3) {
        cout << "Warning: Unrecognised input " << argv[2] << '\n';
    }

    int N, E;
    cin >> N >> E;

    vector<vector<Edge>> edges(N);
    for (int i = 0; i < E; ++i) {
        int a, b, c;
        cin >> a >> b >> c;
        edges[a].push_back(Edge{b, c});
        edges[b].push_back(Edge{a, c});
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
            distances = DijkstraKeyUpdate(edges, source, hollow_heap_base<int, int>{true});
            break;

        case DijkstrasToUse::UPDATE_KEY_SINGLE_ROOT:
            distances = DijkstraKeyUpdate(edges, source, hollow_heap_base<int, int>{false});
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
