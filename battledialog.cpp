#include "battledialog.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDrag>
#include <QMimeData>
#include <QDataStream>
#include <QMessageBox>
#include <QHeaderView>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

#include "pieceitem.h"

// ===================== AttackerListWidget =====================

AttackerListWidget::AttackerListWidget(Kind kind, QWidget* parent)
    : QListWidget(parent), m_kind(kind)
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);

    setViewMode(QListView::IconMode);
    setIconSize(QSize(56,56));
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Static);
    setSpacing(8);
}

QByteArray AttackerListWidget::packPtr(PieceItem* p)
{
    QByteArray b;
    QDataStream ds(&b, QIODevice::WriteOnly);
    ds.setVersion(QDataStream::Qt_5_15);
    quintptr v = reinterpret_cast<quintptr>(p);
    ds << v;
    return b;
}

PieceItem* AttackerListWidget::unpackPtr(const QByteArray& b)
{
    QDataStream ds(b);
    ds.setVersion(QDataStream::Qt_5_15);
    quintptr v = 0;
    ds >> v;
    return reinterpret_cast<PieceItem*>(v);
}

void AttackerListWidget::startDrag(Qt::DropActions)
{
    auto* it = currentItem();
    if (!it) return;

    PieceItem* p = reinterpret_cast<PieceItem*>(it->data(Qt::UserRole).value<quintptr>());
    if (!p) return;

    auto* mime = new QMimeData;
    mime->setData(MimePtr, packPtr(p));

    auto* drag = new QDrag(this);
    drag->setMimeData(mime);
    drag->setPixmap(it->icon().pixmap(56,56));
    drag->exec(Qt::MoveAction, Qt::MoveAction);
}

void AttackerListWidget::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->mimeData()->hasFormat(MimePtr)) {
        e->setDropAction(Qt::MoveAction);
        e->accept();
        return;
    }
    QListWidget::dragEnterEvent(e);
}

void AttackerListWidget::dragMoveEvent(QDragMoveEvent* e)
{
    if (e->mimeData()->hasFormat(MimePtr)) {
        e->setDropAction(Qt::MoveAction);
        e->accept();
        return;
    }
    QListWidget::dragMoveEvent(e);
}

void AttackerListWidget::dropEvent(QDropEvent* e)
{
    if (!e->mimeData()->hasFormat(MimePtr)) {
        QListWidget::dropEvent(e);
        return;
    }

    PieceItem* p = unpackPtr(e->mimeData()->data(MimePtr));
    if (!p) { e->ignore(); return; }

    // 来源 list：用 source 判断
    auto* src = qobject_cast<AttackerListWidget*>(e->source());
    if (!src) { e->ignore(); return; }

    const Kind from = src->m_kind;
    const Kind to   = this->m_kind;

    emit requestMovePiece(p, from, to);

    e->setDropAction(Qt::MoveAction);
    e->accept();
}

// ===================== BattleDialog =====================

static QLabel* makeTitle(const QString& s)
{
    auto* lb = new QLabel(s);
    QFont f = lb->font();
    f.setBold(true);
    lb->setFont(f);
    return lb;
}

BattleDialog::BattleDialog(int targetRegionId, QWidget* parent)
    : QDialog(parent), m_targetRegionId(targetRegionId)
{
    setWindowTitle("宣告陆战");
    resize(760, 420);

    auto* root = new QVBoxLayout(this);

    // 顶部信息栏
    auto* rowInfo = new QHBoxLayout;
    m_lbTarget = makeTitle(QString("战场格：%1").arg(m_targetRegionId));
    m_lbCurAP  = new QLabel("当前AP：-");
    m_lbNeedAP = new QLabel("需要AP：-");
    rowInfo->addWidget(m_lbTarget);
    rowInfo->addStretch(1);
    rowInfo->addWidget(m_lbCurAP);
    rowInfo->addSpacing(16);
    rowInfo->addWidget(m_lbNeedAP);
    root->addLayout(rowInfo);

    // 中间两个列表
    auto* rowLists = new QHBoxLayout;

    auto* colL = new QVBoxLayout;
    colL->addWidget(makeTitle("可参战兵团"));
    m_listCandidates = new AttackerListWidget(AttackerListWidget::Kind::Candidates, this);
    colL->addWidget(m_listCandidates, 1);

    auto* colR = new QVBoxLayout;
    colR->addWidget(makeTitle("已选择参战兵团"));
    m_listSelected = new AttackerListWidget(AttackerListWidget::Kind::Selected, this);
    colR->addWidget(m_listSelected, 1);

    m_listCandidates->setOther(m_listSelected);
    m_listSelected->setOther(m_listCandidates);

    rowLists->addLayout(colL, 1);
    rowLists->addSpacing(12);
    rowLists->addLayout(colR, 1);
    root->addLayout(rowLists, 1);

    // 底部按钮
    auto* rowBtns = new QHBoxLayout;
    rowBtns->addStretch(1);
    m_btnOk = new QPushButton("OK");
    m_btnCancel = new QPushButton("取消");
    m_btnOk->setEnabled(false);
    rowBtns->addWidget(m_btnOk);
    rowBtns->addWidget(m_btnCancel);
    root->addLayout(rowBtns);

    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    connect(m_btnOk, &QPushButton::clicked, this, [this]{
        const int need = m_costProvider ? m_costProvider(m_selected) : 0;
        QList<PieceItem*> attackers = m_selected.values();
        emit landBattleConfirmed(m_targetRegionId, attackers, need);
        accept();
    });

    connect(m_listCandidates, &AttackerListWidget::requestMovePiece,
            this, &BattleDialog::tryMove);

    connect(m_listSelected, &AttackerListWidget::requestMovePiece,
            this, &BattleDialog::tryMove);

    updateAPUI();
}

void BattleDialog::setCandidates(const QList<PieceItem*>& candidates)
{
    m_candidates = candidates;

    // 如果候选变化，把已选中不在候选里的清掉（避免悬空）
    QSet<PieceItem*> candSet;
    for (auto* p : m_candidates) candSet.insert(p);

    for (auto* sel : m_selected) {
        if (!candSet.contains(sel)) {
            m_selected.remove(sel);
        }
    }

    rebuildLists();
    updateAPUI();
}

void BattleDialog::setAPProvider(std::function<int()> apProvider)
{
    m_apProvider = std::move(apProvider);
    updateAPUI();
}

void BattleDialog::setCostProvider(std::function<int(const QSet<PieceItem*>&)> costProvider)
{
    m_costProvider = std::move(costProvider);
    updateAPUI();
}

QString BattleDialog::pieceTitle(PieceItem* p)
{
    if (!p) return QString();
    // 你可以按自己喜好改显示
    return QString("Lv%1  from %2").arg(p->level()).arg(p->regionId());
}

QIcon BattleDialog::pieceIcon(PieceItem* p)
{
    if (!p) return QIcon();
    return QIcon(p->pixmap()); // 直接用场上 pixmap
}

void BattleDialog::rebuildLists()
{
    m_listCandidates->clear();
    m_listSelected->clear();

    // 候选：排除已选
    for (auto* p : m_candidates) {
        if (!p) continue;
        if (m_selected.contains(p)) continue;

        auto* it = new QListWidgetItem(pieceIcon(p), pieceTitle(p));
        it->setData(Qt::UserRole, QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(p)));
        m_listCandidates->addItem(it);
    }

    // 已选
    for (auto* p : m_selected) {
        if (!p) continue;
        auto* it = new QListWidgetItem(pieceIcon(p), pieceTitle(p));
        it->setData(Qt::UserRole, QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(p)));
        m_listSelected->addItem(it);
    }
}

void BattleDialog::updateAPUI()
{
    const int cur  = m_apProvider ? m_apProvider() : 0;
    const int need = m_costProvider ? m_costProvider(m_selected) : 0;

    m_lbCurAP->setText(QString("当前AP：%1").arg(cur));
    m_lbNeedAP->setText(QString("需要AP：%1").arg(need));

    const bool ok = (!m_selected.isEmpty() && need <= cur);
    m_btnOk->setEnabled(ok);
}

void BattleDialog::tryMove(PieceItem* piece,
                           AttackerListWidget::Kind from,
                           AttackerListWidget::Kind to)
{
    if (!piece) return;
    if (from == to) return;

    // Candidates -> Selected：尝试加入（AP gate）
    if (from == AttackerListWidget::Kind::Candidates &&
        to   == AttackerListWidget::Kind::Selected)
    {
        QSet<PieceItem*> next = m_selected;
        next.insert(piece);

        const int cur  = m_apProvider ? m_apProvider() : 0;
        const int need = m_costProvider ? m_costProvider(next) : 0;

        if (need > cur) {
            QMessageBox::information(this, "行动点不足",
                                     QString("无法加入参战：需要AP=%1，当前AP=%2。").arg(need).arg(cur));
            return;
        }

        m_selected.insert(piece);
        rebuildLists();
        updateAPUI();
        return;
    }

    // Selected -> Candidates：移除
    if (from == AttackerListWidget::Kind::Selected &&
        to   == AttackerListWidget::Kind::Candidates)
    {
        m_selected.remove(piece);
        rebuildLists();
        updateAPUI();
        return;
    }
}
