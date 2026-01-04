#include "battlesetupdialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

static QString sideName(Side s){
    if (s==Side::D) return "德国";
    if (s==Side::S) return "苏联";
    if (s==Side::L) return "仆从";
    return "未知";
}

BattleSetupDialog::BattleSetupDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("陆战：投入兵团");
    resize(620, 360);

    auto* root = new QVBoxLayout(this);

    auto* title = new QLabel(this);
    title->setWordWrap(true);
    root->addWidget(title);

    auto* row = new QHBoxLayout;
    root->addLayout(row, 1);

    auto makeList = [&](const QString& cap){
        auto* box = new QVBoxLayout;
        auto* lb = new QLabel(cap, this);
        auto* lw = new QListWidget(this);
        lw->setViewMode(QListView::IconMode);
        lw->setIconSize(QSize(56,56));
        lw->setResizeMode(QListView::Adjust);
        lw->setMovement(QListView::Static);
        lw->setSpacing(6);
        lw->setSelectionMode(QAbstractItemView::NoSelection);
        box->addWidget(lb);
        box->addWidget(lw, 1);
        row->addLayout(box, 1);
        return lw;
    };

    m_atkList = makeList("攻方兵团");
    m_defList = makeList("守方兵团");

    m_ok = new QPushButton("确认", this);
    root->addWidget(m_ok);

    connect(m_ok, &QPushButton::clicked, this, &QDialog::accept);
}

void BattleSetupDialog::setSides(Side attacker, Side defender)
{
    m_atk = attacker; m_def = defender;
    setWindowTitle(QString("陆战：%1 攻击 %2").arg(sideName(m_atk)).arg(sideName(m_def)));
}

void BattleSetupDialog::setRegionId(int rid)
{
    m_rid = rid;
}

void BattleSetupDialog::refreshUnits(const QStringList& atkPix, const QStringList& defPix)
{
    auto fill = [](QListWidget* lw, const QStringList& paths){
        lw->clear();
        for (auto& p : paths) {
            auto* it = new QListWidgetItem(QIcon(p), "");
            lw->addItem(it);
        }
    };

    fill(m_atkList, atkPix);
    fill(m_defList, defPix);
}
