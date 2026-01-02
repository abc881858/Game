#include "placementmanager.h"
#include <QGraphicsScene>
#include <QtMath>
#include <algorithm>
#include "regionitem.h"
#include "pieceitem.h"

PlacementManager::PlacementManager(QGraphicsScene* scene, QObject* parent)
    : QObject(parent), m_scene(scene)
{
}

void PlacementManager::addRegionItem(RegionItem* region)
{
    m_regions.insert(region->id(), region);
}

RegionItem* PlacementManager::region(int regionId) const
{
    return m_regions.value(regionId, nullptr);
}

int PlacementManager::hitTestRegionId(const QPointF& scenePos) const
{
    const auto items = m_scene->items(scenePos, Qt::IntersectsItemShape, Qt::DescendingOrder);
    for (auto* it : items) {
        if (it->type() == RegionType) {
            return static_cast<RegionItem*>(it)->id();
        }
    }
    return -1;
}

void PlacementManager::removePieceItem(PieceItem* piece)
{
    const int rid = piece->regionId();
    if (rid < 0) return;

    auto& vec = m_piecesInRegion[rid];
    vec.removeAll(piece);
    relayoutRegion(rid);
}

void PlacementManager::movePieceToRegion(PieceItem* piece, int newRegionId)
{
    const int old = piece->regionId(); // 最小改动
    if (old == newRegionId) {
        relayoutRegion(newRegionId);
        return;
    }

    if (old >= 0) {
        auto& vOld = m_piecesInRegion[old];
        vOld.removeAll(piece);
        relayoutRegion(old);
    }

    piece->setRegionId(newRegionId); // 最小改动
    m_piecesInRegion[newRegionId].push_back(piece);
    relayoutRegion(newRegionId);
}

QList<QPointF> PlacementManager::makeOffsets(int n, qreal w, qreal h, qreal gap) const
{
    QList<QPointF> off;
    if (n <= 0) return off;
    if (n == 1) return { {0,0} };
    if (n == 2) return { {-(w+gap)/2,0}, {+(w+gap)/2,0} };

    int cols = qCeil(qSqrt(n));
    int rows = qCeil(double(n)/cols);
    qreal totalW = cols*w + (cols-1)*gap;
    qreal totalH = rows*h + (rows-1)*gap;

    int k=0;
    for (int r=0; r<rows && k<n; ++r) {
        for (int c=0; c<cols && k<n; ++c, ++k) {
            qreal x = -totalW/2 + c*(w+gap) + w/2;
            qreal y = -totalH/2 + r*(h+gap) + h/2;
            off << QPointF(x,y);
        }
    }
    return off;
}

void PlacementManager::relayoutRegion(int regionId)
{
    auto* r = region(regionId);
    if (!r) return;

    auto& vec = m_piecesInRegion[regionId];
    if (vec.isEmpty()) return;

    std::sort(vec.begin(), vec.end(), [](PieceItem* a, PieceItem* b){ return a < b; });

    qreal w = vec.first()->boundingRect().width();
    qreal h = vec.first()->boundingRect().height();
    qreal gap = 8.0;

    const auto offsets = makeOffsets(vec.size(), w, h, gap);
    const QPointF center = r->centerScene();

    for (int i=0; i<vec.size(); ++i) {
        auto* p = vec[i];
        p->setInLayout(true);
        QPointF targetCenter = center + offsets[i];
        p->setPos(targetCenter - p->boundingRect().center());
        p->setZValue(20 + i * 0.1);
        p->setInLayout(false);
    }
}
