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
        e->setDropAction(Qt::CopyAction);
        e->accept();
        return;
    }
    QGraphicsView::dragEnterEvent(e);
}

void GraphicsView::dragMoveEvent(QDragMoveEvent *e)
{
    if (e->mimeData()->hasFormat("application/x-piece") ||
        e->mimeData()->hasFormat("application/x-event-piece")) {
        e->setDropAction(Qt::CopyAction);
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
        // payload: eventId|pixPath
        const QString payload = QString::fromUtf8(e->mimeData()->data("application/x-event-piece"));
        const auto parts = payload.split('|');
        if (parts.size() != 2) { e->ignore(); return; }
        eventId = parts[0];
        pixPath = parts[1];
    }

    QPixmap pm(pixPath);
    if (pm.isNull()) { e->ignore(); return; }

    QPointF scenePos = mapToScene(e->position().toPoint());

    // 点命中找城市
    CitySlotItem* hitSlot = nullptr;
    const auto hitItems = scene()->items(scenePos, Qt::IntersectsItemShape, Qt::DescendingOrder);
    for (auto* it : hitItems) {
        if (it->type() == CitySlotType) {
            hitSlot = static_cast<CitySlotItem*>(it);
            break;
        }
    }

    if (!hitSlot) { e->ignore(); return; }

    // ✅ 事件投放：只能放在允许的苏联占领格
    if (isEvent) {
        if (!eventAllowedSlotIds.contains(hitSlot->id())) { e->ignore(); return; }
    }

    // 创建棋子并强制入城
    auto *item = new PieceItem(pm);
    item->setZValue(20);
    scene()->addItem(item);

    item->setPos(scenePos - item->boundingRect().center());
    item->placeToSlot(hitSlot);

    // ✅ 告诉对话框：事件兵已成功落子
    if (isEvent) {
        emit eventPiecePlaced(eventId, pixPath, hitSlot->id());
    }

    e->acceptProposedAction();
}

void GraphicsView::dragLeaveEvent(QDragLeaveEvent *e)
{
    // 不交给 QGraphicsView 默认实现（它会在 enter/leave 不配对时 qWarning）
    e->accept();
}
