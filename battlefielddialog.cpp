#include "battlefielddialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

static QListWidget* makeIconList(QWidget* parent)
{
    auto* lw = new QListWidget(parent);
    lw->setViewMode(QListView::IconMode);
    lw->setIconSize(QSize(56,56));
    lw->setResizeMode(QListView::Adjust);
    lw->setMovement(QListView::Static);
    lw->setSpacing(6);
    lw->setSelectionMode(QAbstractItemView::NoSelection);
    return lw;
}

static void fillPix(QListWidget* lw, const QStringList& paths)
{
    lw->clear();
    for (auto& p: paths) lw->addItem(new QListWidgetItem(QIcon(p), ""));
}

static void fillStrike2(QListWidget* lw, const QList<StrikeGroupEntry>& v)
{
    lw->clear();
    for (auto& e: v) {
        lw->addItem(new QListWidgetItem(QIcon(e.pixPath), e.eliteBound ? "精锐✓" : ""));
    }
}

BattleFieldDialog::BattleFieldDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("战斗区");
    resize(720, 420);

    auto* root = new QVBoxLayout(this);

    auto* row1 = new QHBoxLayout;
    root->addLayout(row1, 1);
    auto* colA = new QVBoxLayout;
    auto* colD = new QVBoxLayout;
    row1->addLayout(colA, 1);
    row1->addLayout(colD, 1);

    colA->addWidget(new QLabel("攻方兵团", this));
    aU = makeIconList(this);
    colA->addWidget(aU, 1);

    colD->addWidget(new QLabel("守方兵团", this));
    dU = makeIconList(this);
    colD->addWidget(dU, 1);

    auto* row2 = new QHBoxLayout;
    root->addLayout(row2, 1);

    auto* colAS = new QVBoxLayout;
    auto* colDS = new QVBoxLayout;
    row2->addLayout(colAS, 1);
    row2->addLayout(colDS, 1);

    colAS->addWidget(new QLabel("攻方打击群", this));
    aS = makeIconList(this);
    colAS->addWidget(aS, 1);

    colDS->addWidget(new QLabel("守方打击群", this));
    dS = makeIconList(this);
    colDS->addWidget(dS, 1);

    auto* ok = new QPushButton("确认", this);
    root->addWidget(ok);
    connect(ok, &QPushButton::clicked, this, &QDialog::accept);
}

void BattleFieldDialog::setContent(const QStringList& atkUnits,
                                   const QStringList& defUnits,
                                   const QList<StrikeGroupEntry>& atkStr,
                                   const QList<StrikeGroupEntry>& defStr)
{
    fillPix(aU, atkUnits);
    fillPix(dU, defUnits);
    fillStrike2(aS, atkStr);
    fillStrike2(dS, defStr);
}
