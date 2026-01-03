#pragma once

#include <QDialog>
#include <QListWidget>
#include <QSet>
#include <QPointer>
#include <functional>

class QLabel;
class QPushButton;
class PieceItem;

class AttackerListWidget : public QListWidget
{
    Q_OBJECT
public:
    enum class Kind { Candidates, Selected };
    explicit AttackerListWidget(Kind kind, QWidget* parent=nullptr);

    void setOther(AttackerListWidget* other) { m_other = other; }

signals:
    // drag/drop 成功后请求“加入/移除”
    void requestMovePiece(PieceItem* piece, Kind from, Kind to);

protected:
    void startDrag(Qt::DropActions supportedActions) override;
    void dragEnterEvent(QDragEnterEvent* e) override;
    void dragMoveEvent(QDragMoveEvent* e) override;
    void dropEvent(QDropEvent* e) override;

private:
    Kind m_kind;
    AttackerListWidget* m_other = nullptr;

    static constexpr const char* MimePtr = "application/x-pieceptr";

    static QByteArray packPtr(PieceItem* p);
    static PieceItem* unpackPtr(const QByteArray& b);
};

class BattleDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BattleDialog(int targetRegionId, QWidget* parent = nullptr);

    int targetRegionId() const { return m_targetRegionId; }

    // 候选兵团（可参战）
    void setCandidates(const QList<PieceItem*>& candidates);

    // 当前行动方 AP 提供器（由 controller 提供）
    // return 当前可用 AP（动态）
    void setAPProvider(std::function<int()> apProvider);

    // AP 费用计算器（由 controller 提供）
    // input: attackers; return: total ap cost
    void setCostProvider(std::function<int(const QSet<PieceItem*>&)> costProvider);

signals:
    // 用户点 OK 后发出：战场格、参战兵团列表、总AP消耗
    void landBattleConfirmed(int targetRegionId, QList<PieceItem*> attackers, int apCost);

private:
    void rebuildLists();
    void updateAPUI();
    void tryMove(PieceItem* piece, AttackerListWidget::Kind from, AttackerListWidget::Kind to);

    // 辅助：生成条目文本/图标
    static QString pieceTitle(PieceItem* p);
    static QIcon   pieceIcon(PieceItem* p);

private:
    int m_targetRegionId = -1;

    // 数据
    QList<PieceItem*> m_candidates;
    QSet<PieceItem*>  m_selected;

    // providers
    std::function<int()> m_apProvider;
    std::function<int(const QSet<PieceItem*>&)> m_costProvider;

    // UI
    QLabel* m_lbTarget = nullptr;
    QLabel* m_lbCurAP  = nullptr;
    QLabel* m_lbNeedAP = nullptr;

    AttackerListWidget* m_listCandidates = nullptr;
    AttackerListWidget* m_listSelected   = nullptr;

    QPushButton* m_btnOk = nullptr;
    QPushButton* m_btnCancel = nullptr;
};
