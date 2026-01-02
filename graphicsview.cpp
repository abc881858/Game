#include "graphicsview.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QRubberBand>
#include <QDebug>
#include <QDragEnterEvent>
#include <QDropEvent>
#include "graphicsframe.h"
#include "dragdrop.h"
#include "placementmanager.h"

inline bool isActionTokenPath(const QString& pixPath)
{
    return pixPath.contains("_XDQ", Qt::CaseSensitive);
}

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
    // 不交给 QGraphicsView 默认实现（它会在 enter/leave 不配对时 qWarning）
    e->accept();
}

void GraphicsView::dropEvent(QDropEvent *e)
{
    if (!scene()) { QGraphicsView::dropEvent(e); return; }

    const bool isNormal = e->mimeData()->hasFormat(DragDrop::MimePiece);
    const bool isEvent  = e->mimeData()->hasFormat(DragDrop::MimeEventPiece);

    if (!isNormal && !isEvent) {
        QGraphicsView::dropEvent(e);
        return;
    }

    QString pixPath, eventId;
    if (isNormal) {
        pixPath = QString::fromUtf8(e->mimeData()->data(DragDrop::MimePiece));
    } else {
        const QString payload = QString::fromUtf8(e->mimeData()->data(DragDrop::MimeEventPiece));
        if (!DragDrop::unpackEventPiece(payload, eventId, pixPath)) {
            e->ignore();
            return;
        }
    }

    if (pixPath.isEmpty()) { e->ignore(); return; }

    const QPointF scenePos = mapToScene(e->position().toPoint());

    if (isNormal && pixPath.contains("_XDQ", Qt::CaseSensitive)) {
        if (scene()->sceneRect().contains(scenePos)) {
            emit actionTokenDropped(pixPath);
            e->setDropAction(Qt::MoveAction);
            e->accept();
        } else {
            e->ignore();
        }
        return;
    }

    if (!m_placementManager) { e->ignore(); return; }

    const int regionId = m_placementManager->hitTestRegionId(scenePos);
    if (regionId < 0) { e->ignore(); return; }

    emit pieceDropped(pixPath, eventId, regionId, isEvent);

    e->setDropAction(Qt::MoveAction);
    e->accept();
}
