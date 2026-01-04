#include "piecelistwidget.h"
#include <QDrag>
#include <QMimeData>
#include "pieceentrywidget.h"
#include "dragdrop.h"
#include "gamecontroller.h"

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
    if (pixPath.isEmpty()) return;

    const bool isToken = pixPath.contains("_XDQ", Qt::CaseSensitive);

    if (m_controller) {
        if (isToken) {
            if (!m_controller->canDragActionToken(m_side)) return;
        } else {
            // Move 或 Battle 都允许
            if (!(m_controller->canDragUnitInMoveSeg(m_side) ||
                  m_controller->canDragUnitInBattleSeg(m_side)))
                return;
        }
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
        it->setData(Qt::UserRole + 1, newCount);

        if (auto* w = qobject_cast<PieceEntryWidget*>(itemWidget(it))) {
            w->setCount(newCount);
        }
    }
}
