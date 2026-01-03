#pragma once

#include <QVector>
#include <QPair>
#include <QSet>

class MapGraph
{
public:
    MapGraph();

    // shortest distance from src to dst
    // blocked: 不允许经过的节点（通常是“有敌方兵团/要塞”的格）
    // return -1 if unreachable
    int shortestDistance(int src, int dst, const QSet<int>& blocked) const;

private:
    QVector<QVector<QPair<int,int>>> adj; // to, dist
};
