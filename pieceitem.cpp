#include "pieceitem.h"
#include "regionitem.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QtMath>
#include <QMenu>

static QList<QPair<int,int>> splitOptions(int level)
{
    // 返回 (a,b) 表示 level -> a + b
    switch (level) {
    case 4: return { {2,2}, {1,3} };
    case 3: return { {1,2} };
    case 2: return { {1,1} };
    default: return {};
    }
}

static QList<QPointF> makeOffsets(int n, qreal w, qreal h, qreal gap)
{
    // offset 是相对“城市中心点”的偏移（单位：scene）
    QList<QPointF> off;
    if (n <= 0) return off;

    if (n == 1) {
        off << QPointF(0, 0);
    } else if (n == 2) {
        off << QPointF(-(w+gap)/2, 0)
            << QPointF( +(w+gap)/2, 0);
    } else if (n == 3) {
        // 品字形：上 1 下 2
        off << QPointF(0, -(h+gap)/2)
            << QPointF(-(w+gap)/2, +(h+gap)/2)
            << QPointF( +(w+gap)/2, +(h+gap)/2);
    } else if (n == 4) {
        // 2x2
        off << QPointF(-(w+gap)/2, -(h+gap)/2)
            << QPointF( +(w+gap)/2, -(h+gap)/2)
            << QPointF(-(w+gap)/2, +(h+gap)/2)
            << QPointF( +(w+gap)/2, +(h+gap)/2);
    } else {
        // n>4：做一个尽量方形的网格（稳、通用）
        int cols = qCeil(qSqrt(n));
        int rows = qCeil(double(n) / cols);

        qreal totalW = cols * w + (cols - 1) * gap;
        qreal totalH = rows * h + (rows - 1) * gap;

        int k = 0;
        for (int r = 0; r < rows && k < n; ++r) {
            for (int c = 0; c < cols && k < n; ++c, ++k) {
                qreal x = -totalW/2 + c*(w+gap) + w/2;
                qreal y = -totalH/2 + r*(h+gap) + h/2;
                off << QPointF(x, y);
            }
        }
    }
    return off;
}

PieceItem::PieceItem(const QPixmap& pm) : QGraphicsPixmapItem(pm)
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setZValue(20);
}

void PieceItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
    QGraphicsPixmapItem::mouseReleaseEvent(e);
    if (!m_inLayout) snapToNearestRegion();
}

void PieceItem::snapToNearestRegion() {
    if (!scene() || !m_placementManager) return;

    const QPointF center = mapToScene(boundingRect().center());
    const int hitId = m_placementManager->hitTestRegionId(center);

    if (hitId < 0) {
        // 回滚到上次有效位置
        if (m_lastValidRegionId >= 0) {
            setInLayout(true);
            setPos(m_lastValidPos);
            setInLayout(false);
            m_placementManager->relayoutRegion(m_lastValidRegionId);
        }
        return;
    }

    const int oldId = m_regionId;
    m_placementManager->movePieceToRegion(this, hitId);

    m_lastValidRegionId = hitId;
    m_lastValidPos = pos();

    if (oldId >= 0 && oldId != hitId) {
        emit movedRegionToRegion(oldId, hitId, m_side);
    }
}

void PieceItem::relayoutRegion(RegionItem* regionItem)
{
    if (!scene() || !regionItem) return;

    // 找出“属于该城市”的所有棋子
    QList<PieceItem*> pieces;
    for (auto* it : scene()->items()) {
        if (it->type() != PieceType) continue;
        auto* p = static_cast<PieceItem*>(it);
        if (p->regionId() == regionItem->id()) pieces << p;
    }

    if (pieces.isEmpty()) return;

    // 固定顺序：避免每次重排乱跳（你也可以按创建时间/名字等排序）
    std::sort(pieces.begin(), pieces.end(), [](PieceItem* a, PieceItem* b){
        return a < b; // 用指针地址做一个稳定排序（够用）
    });

    // 以第一个棋子的大小作为排布单元（如果棋子大小不同，你可以改为取最大 w/h）
    qreal w = pieces.first()->boundingRect().width();
    qreal h = pieces.first()->boundingRect().height();
    qreal gap = 8.0; // 间距（scene单位）自行调

    auto offsets = makeOffsets(pieces.size(), w, h, gap);
    QPointF center = regionItem->centerScene();

    // 布局
    for (int i = 0; i < pieces.size(); ++i) {
        PieceItem* p = pieces[i];
        p->m_inLayout = true;

        QPointF targetCenter = center + offsets[i];
        p->setPos(targetCenter - p->boundingRect().center());

        // 可选：让后放上去的在更上层
        p->setZValue(20 + i * 0.1);

        p->m_inLayout = false;
    }
}

void PieceItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* e)
{
    if (m_kind != UnitKind::Corps) {
        QGraphicsPixmapItem::contextMenuEvent(e);
        return;
    }

    if(m_level < 2) {
        QGraphicsPixmapItem::contextMenuEvent(e);
        return;
    }

    if(m_regionId < 0) {
        QGraphicsPixmapItem::contextMenuEvent(e);
        return;
    }

    QGraphicsScene* sc = scene();
    if (!sc) { e->ignore(); return; }

    RegionItem* regionItem = nullptr;
    for (auto* it : sc->items()) {
        if (it->type() != RegionType) continue;
        auto* s = static_cast<RegionItem*>(it);
        if (s->id() == m_regionId) { regionItem = s; break; }
    }
    if (!regionItem) { e->ignore(); return; }

    QMenu menu;
    auto opts = splitOptions(m_level);
    for (auto [a,b] : opts) {
        auto* act = menu.addAction(
            QString("%1级兵团 分成 %2级兵团 + %3级兵团").arg(m_level).arg(a).arg(b)
        );
        act->setData(QString("%1,%2").arg(a).arg(b));
    }

    QAction* chosen = menu.exec(e->screenPos());
    if (!chosen) return;

    const auto parts = chosen->data().toString().split(',');
    if (parts.size() != 2) return;
    const int a = parts[0].toInt();
    const int b = parts[1].toInt();

    emit splitRequested(this, a, b);
}
