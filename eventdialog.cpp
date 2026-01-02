#include "eventdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDrag>
#include <QMimeData>
#include <QPixmap>
#include "dragdrop.h"

class EventPieceList : public QListWidget
{
public:
    using QListWidget::QListWidget;

protected:
    void startDrag(Qt::DropActions) override
    {
        auto *it = currentItem();
        if (!it) return;

        const QString eventId = it->data(Qt::UserRole + 1).toString();
        const QString pixPath = it->data(Qt::UserRole).toString();

        auto *mime = new QMimeData;
        mime->setData(DragDrop::MimeEventPiece, DragDrop::packEventPiece(eventId, pixPath).toUtf8());

        auto *drag = new QDrag(this);
        drag->setMimeData(mime);

        QPixmap pm(pixPath);
        drag->setPixmap(pm.scaled(64,64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        drag->setHotSpot(QPoint(drag->pixmap().width()/2, drag->pixmap().height()/2));

        drag->exec(Qt::MoveAction);
    }
};

EventDialog::EventDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("事件");
    resize(420, 260);

    auto* lay = new QVBoxLayout(this);

    auto* text = new QLabel(
        "事件：苏军预备队\n"
        "效果：苏联获得3个4级兵团，放置在任意苏联占领格",
        this
    );
    text->setWordWrap(true);
    lay->addWidget(text);

    list = new EventPieceList(this);
    list->setViewMode(QListView::IconMode);
    list->setIconSize(QSize(72,72));
    list->setResizeMode(QListView::Adjust);
    list->setMovement(QListView::Static);
    list->setSpacing(8);
    list->setDragEnabled(true);
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    lay->addWidget(list);
}

void EventDialog::addEventPiece(const QString& name, const QString& pixResPath, int count)
{
    for (int i=0;i<count;++i) {
        auto *it = new QListWidgetItem(QIcon(pixResPath), name);
        it->setData(Qt::UserRole, pixResPath);
        it->setData(Qt::UserRole + 1, m_eventId);
        list->addItem(it);
    }
}

void EventDialog::onEventPiecePlaced(const QString& eventId, const QString& pixPath, int)
{
    if (eventId != m_eventId) return;

    // 删掉一个同 pixPath 的条目（对应“对话框里兵团消失”）
    for (int i=0;i<list->count();++i) {
        auto* it = list->item(i);
        if (it->data(Qt::UserRole).toString() == pixPath) {
            delete list->takeItem(i);
            break;
        }
    }

    if (list->count() == 0) accept();
}
