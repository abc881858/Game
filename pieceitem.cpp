#include "pieceitem.h"
#include "cityslotitem.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QLineF>
#include <QtMath>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>

static QVector<QPair<int,int>> splitOptions(int level)
{
    // 返回 (a,b) 表示 level -> a + b
    switch (level) {
    case 4: return { {2,2}, {1,3} };
    case 3: return { {1,2} };
    case 2: return { {1,1} };
    default: return {};
    }
}

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

    QPointF pieceCenter = mapToScene(boundingRect().center());

    QRectF searchRect(pieceCenter - QPointF(snapRadius, snapRadius), QSizeF(snapRadius * 2, snapRadius * 2));

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

    // ✅ 命中城市：统一交给 placeToSlot 做归属/重排/扣点触发
    placeToSlot(best);
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

void PieceItem::placeToSlot(CitySlotItem* newSlot)
{
    if (!newSlot || !scene()) return;

    const int oldId = m_slotId;
    const int newId = newSlot->id();

    // 先更新归属
    m_slot = newSlot;
    m_slotId = newId;

    // 如果换城，先重排旧城（避免旧城队形乱）
    if (oldId != -1 && oldId != newId) {
        CitySlotItem* oldSlotPtr = nullptr;
        for (auto* it : scene()->items()) {
            if (it->type() != CitySlotType) continue;
            auto* s = static_cast<CitySlotItem*>(it);
            if (s->id() == oldId) { oldSlotPtr = s; break; }
        }
        if (oldSlotPtr) relayoutSlot(oldSlotPtr);
    }

    // 重排新城（会把自己也排好位置）
    relayoutSlot(newSlot);

    // 记录有效落点
    m_lastValidSlotId = m_slotId;
    m_lastValidPos = pos();

    // ✅ 触发移动信号
    if (oldId != -1 && oldId != newId) {
        qDebug() << "movedCityToCity" << oldId << newId << int(m_side);
        emit movedCityToCity(oldId, newId, m_side);
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

    if(m_slotId < 0) {
        QGraphicsPixmapItem::contextMenuEvent(e);
        return;
    }

    QGraphicsScene* sc = scene();
    if (!sc) { e->ignore(); return; }

    // ✅ 先把所在城市 slot 指针找出来并缓存（非常关键）
    CitySlotItem* slotPtr = nullptr;
    for (auto* it : sc->items()) {
        if (it->type() != CitySlotType) continue;
        auto* s = static_cast<CitySlotItem*>(it);
        if (s->id() == m_slotId) { slotPtr = s; break; }
    }
    if (!slotPtr) { e->ignore(); return; }

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

    // ✅ 缓存拆分需要的数据（删除 this 之后不能再用成员）
    const Side side = m_side;

    // ✅ 先把自己从 scene 移除再 delete（更稳）
    sc->removeItem(this);
    delete this; // ⚠️ 从这里开始绝对不能再用 this

    auto spawnOne = [&](int lvl){
        const QString path = corpsPixPath(side, lvl);
        QPixmap pm(path);
        if (pm.isNull()) return;

        auto* ni = new PieceItem(pm);
        ni->setUnitMeta(UnitKind::Corps, side, lvl, path);
        ni->setZValue(20);
        sc->addItem(ni);

        // ✅ 用缓存的 slotPtr 放入同城并重排
        ni->placeToSlot(slotPtr);
    };

    spawnOne(a);
    spawnOne(b);
}
