#include "piecelistwidget.h"
#include <QDrag>
#include <QMimeData>
#include "pieceentrywidget.h"
#include "gamecontroller.h"
#include "dragdrop.h"

PieceListWidget::PieceListWidget(QWidget* parent) : QListWidget(parent)
{
}

void PieceListWidget::startDrag(Qt::DropActions)
{
    auto* it = currentItem();
    if (!it) return;

    int count = it->data(Qt::UserRole + 1).toInt();
    if (count <= 0) return;

    const QString pixPath = it->data(Qt::UserRole).toString();
    if (pixPath.contains("_XDQ", Qt::CaseSensitive)) {
        if (!m_controller->canDragActionToken(m_side)) return;
    } else {
        if (!(m_controller->canDragPieceInMoveSegment(m_side) ||
              m_controller->canDragPieceInBattleSegment(m_side)))
        return;
    }

    auto* mime = new QMimeData;
    mime->setData(DragDrop::MimePiece, pixPath.toUtf8());

    auto* drag = new QDrag(this);
    drag->setMimeData(mime);
    drag->setPixmap(QPixmap(pixPath).scaled(72,72, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    drag->setHotSpot(QPoint(drag->pixmap().width()/2, drag->pixmap().height()/2));

    Qt::DropAction r = drag->exec(Qt::MoveAction, Qt::MoveAction);
    if (r == Qt::MoveAction) {
        int newCount = count - 1;

        if (newCount <= 0) {
            if (auto* w = itemWidget(it)) {
                removeItemWidget(it);
                w->deleteLater();
            }
            delete takeItem(row(it));
        } else {
            it->setData(Qt::UserRole + 1, newCount);
            if (auto* w = qobject_cast<PieceEntryWidget*>(itemWidget(it))) {
                w->setCount(newCount);
            }
        }
    }
}
