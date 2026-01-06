#include "pieceitem.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QtMath>
#include <QMenu>
#include "regionitem.h"

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
    if (m_inLayout) return;

    const QPointF center = mapToScene(boundingRect().center());
    emit dropReleased(this, center);
}

void PieceItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* e)
{
    if (m_kind != PieceKind::Corps) {
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
