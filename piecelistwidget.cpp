#include "piecelistwidget.h"
#include <QDrag>
#include <QMimeData>
#include "pieceentrywidget.h"
#include "dragdrop.h"
#include "gamecontroller.h"

PieceListWidget::PieceListWidget(QWidget* parent) : QListWidget(parent)
{
}

void PieceListWidget::setItemCount(QListWidgetItem* it, int c)
{
    if (!it) return;
    it->setData(RoleCount, c);
    viewport()->update(visualItemRect(it));
}

int PieceListWidget::itemCount(const QListWidgetItem* it) const
{
    if (!it) return 0;
    return it->data(RoleCount).toInt();
}

void PieceListWidget::startDrag(Qt::DropActions)
{
    auto* it = currentItem();
    if (!it) return;

    int count = it->data(Qt::UserRole + 1).toInt();
    if (count <= 0) return;

    const QString pixPath = it->data(Qt::UserRole).toString();
    if (pixPath.isEmpty()) return;

    // ✅ 关键：行动签和单位走不同规则
    if (m_controller) {
        if (pixPath.contains("_XDQ", Qt::CaseSensitive)) {
            if (!m_controller->canPlayActionToken(m_side)) return;
        } else {
            if (!m_controller->canDragUnitInMoveSeg(m_side)) return;
        }
    }

    auto* mime = new QMimeData;
    mime->setData(DragDrop::MimePiece, pixPath.toUtf8());

    auto* drag = new QDrag(this);
    drag->setMimeData(mime);
    drag->setPixmap(QPixmap(pixPath).scaled(72,72, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    Qt::DropAction r = drag->exec(Qt::MoveAction, Qt::MoveAction);
    if (r == Qt::MoveAction) {
        int newCount = count - 1;
        it->setData(Qt::UserRole + 1, newCount);

        if (auto* w = qobject_cast<PieceEntryWidget*>(itemWidget(it))) {
            w->setCount(newCount);
        }
    }
}
