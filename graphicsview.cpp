#include "graphicsview.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include "graphicsframe.h"
#include "dragdrop.h"

GraphicsView::GraphicsView(GraphicsFrame *graphicsFrame)
    : QGraphicsView()
    , m_graphicsFrame(graphicsFrame)
{
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setMouseTracking(true);
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
    if (e->mimeData()->hasFormat(DragDrop::MimePiece) ||
        e->mimeData()->hasFormat(DragDrop::MimeEventPiece)) {
        e->setDropAction(Qt::MoveAction);
        e->accept();
        return;
    }
    QGraphicsView::dragEnterEvent(e);
}

void GraphicsView::dragMoveEvent(QDragMoveEvent *e)
{
    if (e->mimeData()->hasFormat(DragDrop::MimePiece) ||
        e->mimeData()->hasFormat(DragDrop::MimeEventPiece)) {
        e->setDropAction(Qt::MoveAction);
        e->accept();
        return;
    }
    QGraphicsView::dragMoveEvent(e);
}

void GraphicsView::dragLeaveEvent(QDragLeaveEvent *e)
{
    e->accept();
}

void GraphicsView::dropEvent(QDropEvent *e)
{
    const bool isNormal = e->mimeData()->hasFormat(DragDrop::MimePiece);
    const bool isEvent  = e->mimeData()->hasFormat(DragDrop::MimeEventPiece);

    if (!isNormal && !isEvent) {
        QGraphicsView::dropEvent(e);
        return;
    }

    if (isNormal) {
        QString pixPath = QString::fromUtf8(e->mimeData()->data(DragDrop::MimePiece));
        QPointF scenePos = mapToScene(e->position().toPoint());
        emit dropPieceToScene(scenePos, pixPath);
    }
    if (isEvent) {
        QString pixPath = QString::fromUtf8(e->mimeData()->data(DragDrop::MimeEventPiece));
        QPointF scenePos = mapToScene(e->position().toPoint());
        emit dropEventPieceToScene(scenePos, pixPath);
    }

    e->setDropAction(Qt::MoveAction);
    e->accept();
}
