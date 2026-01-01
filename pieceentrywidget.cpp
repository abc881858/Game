#include "pieceentrywidget.h"
#include <QHBoxLayout>
#include <QPixmap>

PieceEntryWidget::PieceEntryWidget(const QIcon& icon, const QString& name, QWidget* parent)
    : QWidget(parent)
{
    m_iconLabel  = new QLabel;
    m_nameLabel  = new QLabel(name);
    m_countLabel = new QLabel;

    m_iconLabel->setFixedSize(72,72);
    m_iconLabel->setPixmap(icon.pixmap(72,72));

    m_nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QFont f = m_countLabel->font();
    f.setBold(true);
    m_countLabel->setFont(f);
    m_countLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_countLabel->setMinimumWidth(50); // 右侧列宽，自己调

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(6,4,6,4);
    lay->setSpacing(8);
    lay->addWidget(m_iconLabel);
    lay->addWidget(m_nameLabel, 1);
    lay->addWidget(m_countLabel);

    setCount(0);
}

void PieceEntryWidget::setCount(int c)
{
    m_count = c;
    m_countLabel->setText(QString("x %1").arg(qMax(0, c)));
    setDisabledVisual(m_count <= 0);
}

void PieceEntryWidget::setDisabledVisual(bool dis)
{
    // 视觉灰掉
    setEnabled(!dis);              // 也会让鼠标/选中变“不可用”
    if (dis) {
        m_countLabel->setStyleSheet("color: rgb(160,160,160);");
        m_nameLabel->setStyleSheet("color: rgb(160,160,160);");
    } else {
        m_countLabel->setStyleSheet("");
        m_nameLabel->setStyleSheet("");
    }
}
