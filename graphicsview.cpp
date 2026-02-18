#include "graphicsview.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include "graphicsframe.h"
#include "util.h"

GraphicsView::GraphicsView(GraphicsFrame *graphicsFrame)
    : QGraphicsView()
    , m_graphicsFrame(graphicsFrame)
{
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setMouseTracking(true);
}

bool GraphicsView::canDropToLand(const QPoint& viewPos) const
{
    if (!scene()) return false;

    const QPointF scenePos = mapToScene(viewPos);
    const auto items = scene()->items(scenePos, Qt::IntersectsItemShape, Qt::DescendingOrder);

    for (auto* it : items) {
        if (it && it->type() == RegionType) {
            return true;
        }
    }
    return false;
}

void GraphicsView::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        if (e->angleDelta().y() > 0)
            m_graphicsFrame->zoomIn(6);
        else
            m_graphicsFrame->zoomOut(6);
        e->accept();
    } else {
        QGraphicsView::wheelEvent(e);
    }
}

void GraphicsView::dragEnterEvent(QDragEnterEvent *e)
{
    const bool ok = e->mimeData()->hasFormat(DragDrop::MimePiece) ||
                    e->mimeData()->hasFormat(DragDrop::MimeEventPiece);

    if (!ok) { QGraphicsView::dragEnterEvent(e); return; }

    e->acceptProposedAction();
}

void GraphicsView::dragMoveEvent(QDragMoveEvent *e)
{
    const bool isNormal = e->mimeData()->hasFormat(DragDrop::MimePiece);
    const bool isEvent  = e->mimeData()->hasFormat(DragDrop::MimeEventPiece);

    if (!isNormal && !isEvent) { QGraphicsView::dragMoveEvent(e); return; }

    if (canDropToLand(e->position().toPoint())) {
        e->setDropAction(Qt::MoveAction);
        e->accept();
    } else {
        e->ignore();
    }
}

void GraphicsView::dragLeaveEvent(QDragLeaveEvent *e)
{
    e->accept();
}

void GraphicsView::dropEvent(QDropEvent *e)
{
    const bool isNormal = e->mimeData()->hasFormat(DragDrop::MimePiece);
    const bool isEvent  = e->mimeData()->hasFormat(DragDrop::MimeEventPiece);

    if (!isNormal && !isEvent) { QGraphicsView::dropEvent(e); return; }

    if (!canDropToLand(e->position().toPoint())) {
        e->ignore();
        return;
    }

    const QPointF scenePos = mapToScene(e->position().toPoint());

    if (isNormal) {
        const QString pixPath = QString::fromUtf8(e->mimeData()->data(DragDrop::MimePiece));
        emit dropPieceToScene(scenePos, pixPath);
    }
    if (isEvent) {
        const QString pixPath = QString::fromUtf8(e->mimeData()->data(DragDrop::MimeEventPiece));
        emit dropEventPieceToScene(scenePos, pixPath);
    }

    e->setDropAction(Qt::MoveAction);
    e->accept();
}

