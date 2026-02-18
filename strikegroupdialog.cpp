#include "strikegroupdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

static QString sideName2(Side s){
    if (s==Side::D) return "德国";
    if (s==Side::S) return "苏联";
}

static void fillStrike(QListWidget* lw, const QList<StrikeGroupEntry>& v)
{
    lw->clear();
    for (auto& e : v) {
        QString txt = e.eliteBound ? "精锐✓" : "";
        auto* it = new QListWidgetItem(QIcon(e.pixPath), txt);
        lw->addItem(it);
    }
}

StrikeGroupDialog::StrikeGroupDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("陆战：投入打击群");
    resize(620, 360);

    auto* root = new QVBoxLayout(this);

    m_tip = new QLabel(this);
    m_tip->setWordWrap(true);
    root->addWidget(m_tip);

    auto* row = new QHBoxLayout;
    root->addLayout(row, 1);

    auto mk = [&](const QString& cap){
        auto* col = new QVBoxLayout;
        col->addWidget(new QLabel(cap, this));
        auto* lw = new QListWidget(this);
        lw->setViewMode(QListView::IconMode);
        lw->setIconSize(QSize(56,56));
        lw->setResizeMode(QListView::Adjust);
        lw->setMovement(QListView::Static);
        lw->setSpacing(6);
        lw->setSelectionMode(QAbstractItemView::NoSelection);
        col->addWidget(lw, 1);
        row->addLayout(col, 1);
        return lw;
    };

    m_atk = mk("攻方打击群");
    m_def = mk("守方打击群");

    m_pass = new QPushButton("让过", this);
    root->addWidget(m_pass);

    connect(m_pass, &QPushButton::clicked, this, &StrikeGroupDialog::passClicked);
}

void StrikeGroupDialog::setSides(Side attacker, Side defender)
{
    m_atkSide = attacker;
    m_defSide = defender;
}

void StrikeGroupDialog::refreshStrikeGroups(const QList<StrikeGroupEntry>& atk,
                                            const QList<StrikeGroupEntry>& def,
                                            Side currentTurnSide,
                                            bool finished)
{
    fillStrike(m_atk, atk);
    fillStrike(m_def, def);

    if (finished) {
        m_tip->setText("双方均已让过，打击群投入结束。");
        return;
    }

    m_tip->setText(QString("轮到【%1】拖入打击群；也可以点击“让过”。")
                   .arg(sideName2(currentTurnSide)));
}
