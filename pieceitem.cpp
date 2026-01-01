#include "pieceitem.h"
#include "cityslotitem.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QLineF>
#include <QtMath>

static QVector<QPointF> makeOffsets(int n, qreal w, qreal h, qreal gap)
{
    // offset 是相对“城市中心点”的偏移（单位：scene）
    QVector<QPointF> off;
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
    if (!m_inLayout) snapToNearestCity();
}

void PieceItem::snapToNearestCity()
{
    if (!scene()) return;

    const int oldId = m_slotId;

    QPointF pieceCenter = mapToScene(boundingRect().center());

    QRectF searchRect(pieceCenter - QPointF(snapRadius, snapRadius),
                      QSizeF(snapRadius * 2, snapRadius * 2));

    auto nearby = scene()->items(searchRect, Qt::IntersectsItemShape);

    CitySlotItem* best = nullptr;
    qreal bestDist = 1e18;
    for (auto* it : nearby) {
        if (it->type() != CitySlotType) continue;
        auto* slot = static_cast<CitySlotItem*>(it);
        qreal d = QLineF(pieceCenter, slot->centerScene()).length();
        if (d < bestDist) { bestDist = d; best = slot; }
    }

    // ❌ 松手不在任何城市：回到上一次有效位置（等于不能放到非城市）
    if (!best || bestDist > snapRadius) {
        if (m_lastValidSlotId != -1) {
            m_inLayout = true;
            setPos(m_lastValidPos);
            m_inLayout = false;

            // 让原城市队形恢复（防止你拖动时把队形弄乱）
            // 找到 oldSlot 并重排（你之前那套遍历找 slot 的方法可复用）
            CitySlotItem* oldSlot = nullptr;
            for (auto* it : scene()->items()) {
                if (it->type() != CitySlotType) continue;
                auto* s = static_cast<CitySlotItem*>(it);
                if (s->id() == m_lastValidSlotId) { oldSlot = s; break; }
            }
            if (oldSlot) relayoutSlot(oldSlot);
        }
        return;
    }

    // ✅ 命中城市：归属到新城市
    const int newId = best->id();
    m_slotId = newId;

    // 变化则重排旧城市
    if (oldId != -1 && oldId != newId) {
        CitySlotItem* oldSlot = nullptr;
        for (auto* it : scene()->items()) {
            if (it->type() != CitySlotType) continue;
            auto* s = static_cast<CitySlotItem*>(it);
            if (s->id() == oldId) { oldSlot = s; break; }
        }
        if (oldSlot) relayoutSlot(oldSlot);
    }

    // 重排新城市
    relayoutSlot(best);

    // ✅ 记录“有效落点”：通常 relayout 后该棋子已经被摆到队形位置了
    m_lastValidSlotId = m_slotId;
    m_lastValidPos = pos();
}

void PieceItem::relayoutSlot(CitySlotItem* slot)
{
    if (!scene() || !slot) return;

    // 找出“属于该城市”的所有棋子
    QList<PieceItem*> pieces;
    for (auto* it : scene()->items()) {
        if (it->type() != PieceType) continue;
        auto* p = static_cast<PieceItem*>(it);
        if (p->slotId() == slot->id()) pieces << p;
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
    QPointF center = slot->centerScene();

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

void PieceItem::placeToSlot(CitySlotItem* slot)
{
    if (!slot || !scene()) return;

    // 归属
    setSlotId(slot->id());

    // 让该城市的所有棋子重新排（内部会把自己也摆到中心/品字/网格）
    relayoutSlot(slot);

    m_lastValidSlotId = m_slotId;
    m_lastValidPos = pos();
}
