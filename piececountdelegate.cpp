#include "piececountdelegate.h"
#include <QPainter>

static constexpr int RolePixPath = Qt::UserRole;
static constexpr int RoleCount   = Qt::UserRole + 1;

void PieceCountDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const
{
    QStyleOptionViewItem o(opt);
    initStyleOption(&o, idx);

    int count = idx.data(RoleCount).toInt();
    bool zero = (count <= 0);

    // 0 个时：灰掉图标/文字
    if (zero) {
        o.state &= ~QStyle::State_Enabled;
        o.palette.setColor(QPalette::Text, QColor(160,160,160));
        o.palette.setColor(QPalette::WindowText, QColor(160,160,160));
    }

    // 先画默认内容（图标+标题）
    QStyledItemDelegate::paint(p, o, idx);

    // 再画右侧 x N
    p->save();
    QString s = QString("x %1").arg(qMax(0, count));

    QRect r = opt.rect.adjusted(6, 0, -6, 0);
    QFont f = opt.font;
    f.setBold(true);
    p->setFont(f);

    QColor c = zero ? QColor(170,170,170) : QColor(30,30,30);
    p->setPen(c);

    p->drawText(r, Qt::AlignVCenter | Qt::AlignRight, s);
    p->restore();
}
