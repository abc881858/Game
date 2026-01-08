#pragma once

#include <QList>
#include <QPair>
#include <QSet>

class MapGraph
{
public:
    MapGraph();

    // blocked: 不允许经过的节点（通常是“有敌方兵团/要塞”的格）
    // return -1 if unreachable
    int shortestDistance(int src, int dst, const QSet<int>& blocked) const;

private:
    QList<QList<QPair<int,int>>> adj; // to, dist
};
