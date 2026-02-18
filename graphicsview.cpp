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
    const auto *md = e->mimeData();
    const bool isToken  = md->hasFormat(MimeListToken);
    const bool isPiece  = md->hasFormat(MimeListPiece);
    const bool isDialog = md->hasFormat(MimeDialogPiece);

    if (!isToken && !isPiece && !isDialog) {
        QGraphicsView::dragEnterEvent(e);
        return;
    }

    if ((isPiece || isDialog) && !canDropToLand(e->position().toPoint())) {
        e->ignore();
        return;
    }

    e->setDropAction(Qt::MoveAction);
    e->accept();
}

void GraphicsView::dragMoveEvent(QDragMoveEvent *e)
{
    const auto *md = e->mimeData();
    const bool isToken  = md->hasFormat(MimeListToken);
    const bool isPiece  = md->hasFormat(MimeListPiece);
    const bool isDialog = md->hasFormat(MimeDialogPiece);

    if (!isToken && !isPiece && !isDialog) {
        QGraphicsView::dragMoveEvent(e);
        return;
    }

    if ((isPiece || isDialog) && !canDropToLand(e->position().toPoint())) {
        e->ignore();
        return;
    }

    e->setDropAction(Qt::MoveAction);
    e->accept();
}

void GraphicsView::dragLeaveEvent(QDragLeaveEvent *e)
{
    e->accept();
}

void GraphicsView::dropEvent(QDropEvent *e)
{
    const auto *md = e->mimeData();
    const bool isToken  = md->hasFormat(MimeListToken);
    const bool isDialog = md->hasFormat(MimeDialogPiece);
    const bool isPiece  = md->hasFormat(MimeListPiece);

    if (!isToken && !isDialog && !isPiece) {
        QGraphicsView::dropEvent(e);
        return;
    }

    if ((isDialog || isPiece) && !canDropToLand(e->position().toPoint())) {
        e->ignore();
        return;
    }

    const QString fmt = isToken ? MimeListToken : (isDialog ? MimeDialogPiece : MimeListPiece);

    const QString pixPath = QString::fromUtf8(md->data(fmt));
    const QPointF scenePos = mapToScene(e->position().toPoint());

    emit dropPieceToScene(scenePos, pixPath);

    e->setDropAction(Qt::MoveAction);
    e->accept();
}
