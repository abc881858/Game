#pragma once

#include <QObject>
#include <QHash>
#include <QList>

class RegionItem;
class PieceItem;

class PlacementManager : public QObject
{
    Q_OBJECT
public:
    explicit PlacementManager(QObject* parent=nullptr);
    void addRegionItem(RegionItem* region);               // 注册可放置区域
    RegionItem* region(int regionId) const;
    void movePieceToRegion(PieceItem* piece, int newRegionId); // 维护pieces并重排
    void removePieceItem(PieceItem* piece);                        // 删除/拆分前调用
    void relayoutRegion(int regionId); // 只重排一个区域
    QList<PieceItem*> piecesInRegion(int regionId) const;
private:
    QHash<int, RegionItem*> m_regions;
    QHash<int, QList<PieceItem*>> m_piecesInRegion;
    QList<QPointF> makeOffsets(int n, qreal w, qreal h, qreal gap) const;
};
