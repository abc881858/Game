#include "mapgraph.h"
#include <queue>
#include <limits>

MapGraph::MapGraph()
{
    adj.resize(35);

    auto addEdge = [&](int a, int b, int w){
        adj[a].push_back({b,w});
        adj[b].push_back({a,w});
    };

    // ===== 复制你给的边表 =====
    addEdge(0, 1, 1);
    addEdge(0, 14, 8);
    addEdge(1, 2, 0);
    addEdge(2, 3, 1);
    addEdge(2, 4, 2);
    addEdge(3, 7, 1);
    addEdge(4, 5, 2);
    addEdge(4, 8, 2);
    addEdge(5, 9, 1);
    addEdge(5, 10, 1);
    addEdge(6, 7, 1);
    addEdge(6, 11, 1);
    addEdge(6, 12, 1);
    addEdge(6, 14, 2);
    addEdge(7, 8, 1);
    addEdge(7, 13, 1);
    addEdge(8, 9, 1);
    addEdge(8, 13, 1);
    addEdge(9, 10, 1);
    addEdge(9, 13, 1);
    addEdge(9, 18, 1);
    addEdge(10, 19, 1);
    addEdge(11, 12, 1);
    addEdge(11, 14, 1);
    addEdge(11, 16, 1);
    addEdge(12, 13, 1);
    addEdge(12, 16, 1);
    addEdge(12, 17, 1);
    addEdge(13, 17, 1);
    addEdge(13, 18, 1);
    addEdge(14, 15, 1);
    addEdge(14, 16, 1);
    addEdge(15, 16, 1);
    addEdge(15, 21, 1);
    addEdge(15, 22, 1);
    addEdge(16, 23, 1);
    addEdge(17, 20, 1);
    addEdge(17, 23, 1);
    addEdge(18, 19, 1);
    addEdge(18, 24, 1);
    addEdge(18, 20, 1);
    addEdge(19, 24, 1);
    addEdge(20, 25, 1);
    addEdge(20, 26, 1);
    addEdge(21, 22, 1);
    addEdge(21, 29, 1);
    addEdge(22, 23, 1);
    addEdge(22, 28, 1);
    addEdge(22, 30, 1);
    addEdge(23, 25, 1);
    addEdge(23, 28, 1);
    addEdge(24, 26, 1);
    addEdge(24, 27, 1);
    addEdge(24, 32, 2);
    addEdge(25, 26, 1);
    addEdge(25, 28, 1);
    addEdge(25, 31, 1);
    addEdge(26, 31, 1);
    addEdge(26, 33, 2);
    addEdge(26, 32, 2);
    addEdge(27, 32, 1);
    addEdge(28, 30, 1);
    addEdge(28, 31, 1);
    addEdge(29, 30, 1);
    addEdge(30, 31, 2);
    addEdge(31, 33, 2);
    addEdge(32, 33, 1);
    addEdge(32, 34, 1);
    addEdge(33, 34, 2);
}

int MapGraph::shortestDistance(int src, int dst, const QSet<int>& blocked) const
{
    if (src < 0 || dst < 0 || src >= adj.size() || dst >= adj.size()) return -1;
    if (src == dst) return 0;

    if (blocked.contains(dst)) return -1; // dst 在 blocked => 一定不可达

    const int INF = std::numeric_limits<int>::max()/4;
    QVector<int> dist(adj.size(), INF);

    using Node = QPair<int,int>; // (dist, v)
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;

    dist[src] = 0;
    pq.push({0, src});

    while(!pq.empty()){
        auto [d, v] = pq.top(); pq.pop();
        if (d != dist[v]) continue;
        if (v == dst) return d;

        for (auto [to, w] : adj[v]) {
            if (blocked.contains(to)) continue; // blocked 节点一律不允许进入（包括 dst）
            int nd = d + w;
            if (nd < dist[to]) {
                dist[to] = nd;
                pq.push({nd, to});
            }
        }
    }
    return -1;
}
