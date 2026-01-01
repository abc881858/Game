#include "graphicsview.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QRubberBand>
#include <QDebug>
#include "view.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QGraphicsPixmapItem>
#include "pieceitem.h"
#include "cityslotitem.h"
#include "util.h"

inline bool isActionTokenPath(const QString& pixPath)
{
    return pixPath.contains("_XDQ", Qt::CaseSensitive);
}

GraphicsView::GraphicsView(View *v) : QGraphicsView(), view(v)
{
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setMouseTracking(true);
}

void GraphicsView::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        if (e->angleDelta().y() > 0)
            view->zoomIn(6);
        else
            view->zoomOut(6);
        e->accept();
    } else {
        QGraphicsView::wheelEvent(e);
    }
}

void GraphicsView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && (e->modifiers() & Qt::ShiftModifier)) {
        selecting = true;
        origin = e->pos();

        if (!rubberBand)
            rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

        rubberBand->setGeometry(QRect(origin, QSize()));
        rubberBand->show();

        e->accept();
        return;
    }

    QGraphicsView::mousePressEvent(e);
}

void GraphicsView::mouseMoveEvent(QMouseEvent *e)
{
    if (selecting && rubberBand) {
        rubberBand->setGeometry(QRect(origin, e->pos()).normalized());
        e->accept();
        return;
    }

    QGraphicsView::mouseMoveEvent(e);
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *e)
{
    if (selecting && e->button() == Qt::LeftButton && rubberBand) {
        selecting = false;
        rubberBand->hide();

        QRect vr = rubberBand->geometry().normalized();

        // 转成 scene 坐标
        QPointF tl = mapToScene(vr.topLeft());
        QPointF br = mapToScene(vr.bottomRight());

        QRectF sr(QPointF(qMin(tl.x(), br.x()), qMin(tl.y(), br.y())),
                  QPointF(qMax(tl.x(), br.x()), qMax(tl.y(), br.y())));

        qDebug().noquote() << QString("QRectF(%1, %2, %3, %4),")
                              .arg(sr.x(), 0, 'f', 2)
                              .arg(sr.y(), 0, 'f', 2)
                              .arg(sr.width(), 0, 'f', 2)
                              .arg(sr.height(), 0, 'f', 2);

        e->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(e);
}

void GraphicsView::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasFormat("application/x-piece") ||
        e->mimeData()->hasFormat("application/x-event-piece")) {
        e->setDropAction(Qt::MoveAction);
        e->accept();
        return;
    }
    QGraphicsView::dragEnterEvent(e);
}

void GraphicsView::dragMoveEvent(QDragMoveEvent *e)
{
    if (e->mimeData()->hasFormat("application/x-piece") ||
        e->mimeData()->hasFormat("application/x-event-piece")) {
        e->setDropAction(Qt::MoveAction);
        e->accept();
        return;
    }
    QGraphicsView::dragMoveEvent(e);
}

void GraphicsView::dropEvent(QDropEvent *e)
{
    if (!scene()) { QGraphicsView::dropEvent(e); return; }

    const bool isNormal = e->mimeData()->hasFormat("application/x-piece");
    const bool isEvent  = e->mimeData()->hasFormat("application/x-event-piece");

    if (!isNormal && !isEvent) {
        QGraphicsView::dropEvent(e);
        return;
    }

    QString pixPath;
    QString eventId;

    if (isNormal) {
        pixPath = QString::fromUtf8(e->mimeData()->data("application/x-piece"));
    } else {
        const QString payload = QString::fromUtf8(e->mimeData()->data("application/x-event-piece"));
        const auto parts = payload.split('|');
        if (parts.size() != 2) { e->ignore(); return; }
        eventId = parts[0];
        pixPath = parts[1];
    }

    QPixmap pm(pixPath);
    if (pm.isNull()) { e->ignore(); return; }

    QPointF scenePos = mapToScene(e->position().toPoint());

    // ✅ 行动签：只要 drop 到棋盘（sceneRect）就消耗，不需要城市格
    if (isNormal && isActionTokenPath(pixPath)) {
        if (scene()->sceneRect().contains(scenePos)) {
            e->setDropAction(Qt::MoveAction); // 告诉源端扣数量
            e->accept();
        } else {
            e->ignore(); // 丢到棋盘外不消耗
        }
        return;
    }

    // ====== 下面保持你原来的逻辑：必须命中城市格 ======

    CitySlotItem* hitSlot = nullptr;
    const auto hitItems = scene()->items(scenePos, Qt::IntersectsItemShape, Qt::DescendingOrder);
    for (auto* it : hitItems) {
        if (it->type() == CitySlotType) {
            hitSlot = static_cast<CitySlotItem*>(it);
            break;
        }
    }

    if (!hitSlot) { e->ignore(); return; }

    if (isEvent) {
        if (!eventAllowedSlotIds.contains(hitSlot->id())) { e->ignore(); return; }
    }

    auto *item = new PieceItem(pm);
    Side side;
    int lvl;
    if (parseCorpsFromPixPath(pixPath, side, lvl)) {
        item->setUnitMeta(UnitKind::Corps, side, lvl, pixPath);
    } else {
        item->setUnitMeta(UnitKind::Other, Side::Unknown, 0, pixPath);
    }
    item->setZValue(20);
    scene()->addItem(item);

    item->setPos(scenePos - item->boundingRect().center());
    item->placeToSlot(hitSlot);

    if (isEvent) {
        emit eventPiecePlaced(eventId, pixPath, hitSlot->id());
    }

    e->setDropAction(Qt::MoveAction);
    e->accept();
}

void GraphicsView::dragLeaveEvent(QDragLeaveEvent *e)
{
    // 不交给 QGraphicsView 默认实现（它会在 enter/leave 不配对时 qWarning）
    e->accept();
}
