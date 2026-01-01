#include "piecelistwidget.h"
#include <QDrag>
#include <QMimeData>
#include <QPixmap>

PieceListWidget::PieceListWidget(QWidget* parent) : QListWidget(parent)
{
    setDragEnabled(true);
}

void PieceListWidget::startDrag(Qt::DropActions supportedActions)
{
    auto *it = currentItem();
    if (!it) return;

    QString pixPath = it->data(Qt::UserRole).toString();

    auto *mime = new QMimeData;
    mime->setData("application/x-piece", pixPath.toUtf8());

    auto *drag = new QDrag(this);
    drag->setMimeData(mime);

    // 拖拽时的预览图
    QPixmap pm(pixPath);
    drag->setPixmap(pm.scaled(64,64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    drag->setHotSpot(QPoint(drag->pixmap().width()/2, drag->pixmap().height()/2));

    drag->exec(Qt::CopyAction);
}
