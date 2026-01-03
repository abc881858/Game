#pragma once

#include <QObject>
#include <QSet>
#include <QString>
#include "mapgraph.h"
#include "util.h"
#include "pieceitem.h"

class QGraphicsScene;
class PlacementManager;

struct GameState
{
    int turn = 1;  // 1..8（你可在 setter 里 clamp）

    // D
    int npD  = 0;
    int oilD = 0;
    int apD  = 0;
    int rpD  = 0;

    // S
    int npS  = 0;
    int oilS = 0;
    int apS  = 0;
    int rpS  = 0;

    // helper
    int& apRef(Side s)  { return (s==Side::D)? apD  : apS; }
    int& npRef(Side s)  { return (s==Side::D)? npD  : npS; }
    int& oilRef(Side s) { return (s==Side::D)? oilD : oilS; }
    int& rpRef(Side s)  { return (s==Side::D)? rpD  : rpS; }

    int ap(Side s)  const { return (s==Side::D)? apD  : (s==Side::S)? apS  : 0; }
    int np(Side s)  const { return (s==Side::D)? npD  : (s==Side::S)? npS  : 0; }
    int oil(Side s) const { return (s==Side::D)? oilD : (s==Side::S)? oilS : 0; }
    int rp(Side s)  const { return (s==Side::D)? rpD  : (s==Side::S)? rpS  : 0; }
};

class GameController : public QObject
{
    Q_OBJECT
public:
    // ===== 读状态（MainWindow用）=====
    const GameState& state() const { return m_state; }

    // ===== 改状态（统一入口）=====
    void addTurn(int delta);

    void addNationalPower(Side side, int delta);
    void addOil(Side side, int delta);
    void addReadyPoints(Side side, int delta);

    void addAP(Side side, int delta);          // 统一入口：加/扣 AP（含进入行动阶段触发）
    void clearAP(Side side);                  // 规则9.5清零用

    // ===== AP：现在由 controller 维护 =====
    int currentAP(Side side) const;

    // ===== 行动阶段机 =====
    void startActionPhase(Side side);     // 开始行动阶段（默认 Move）
    void advanceSegment();                // 结束当前环节 -> 进入下一个 or 结束阶段
    void endActionPhase();                // 结束整个行动阶段（清状态 + 限制拖拽）

    enum class ActionSeg { Move=0, Battle=1, Redeploy=2, Prepare=3, Supply=4 };

    struct ActionPhaseState {
        bool active = false;
        Side activeSide = Side::Unknown;
        ActionSeg seg = ActionSeg::Move;
        int segIndex = 0; // 0..4
    };

    explicit GameController(QGraphicsScene* scene, PlacementManager* placementManager, QObject* parent=nullptr);

    const ActionPhaseState& actionPhase() const { return m_phase; }

    // 事件落子许可格（事件对话框弹出时设置，关闭时清空）
    void setEventAllowedRegions(const QSet<int>& ids) { m_eventAllowedRegions = ids; }
    void clearEventAllowedRegions() { m_eventAllowedRegions.clear(); }

    PieceItem* placeNewPieceToRegion(int regionId,
                                     const QString& pixPath,
                                     qreal z = 20.0,
                                     const QString& eventId = QString(),
                                     bool isEvent = false);

    void resetAllPiecesMoveFlag();
    void refreshMovablePieces();
    bool canDragFromReserve(Side side) const;

    bool canDragUnitInMoveSeg(Side side) const;

public slots:
    // 由 GraphicsView 在 dropEvent 中转发过来
    void onPieceDropped(const QString& pixPath, const QString& eventId, int regionId, bool isEvent);

    // 由 GraphicsView 在 dropEvent 中转发过来（行动签）
    void onActionTokenDropped(const QString& pixPath);

    // 由 PieceItem 的右键菜单触发（拆分）
    void onSplitRequested(PieceItem* piece, int a, int b);

    void onPieceMovedRegionToRegion(PieceItem* piece, int fromRegionId, int toRegionId, Side side);

signals:
    // 给 EventDialog 订阅：用于从列表里删掉事件棋子计数
    void eventPiecePlaced(const QString& eventId, const QString& pixPath, int regionId);

    // ===== UI 控制用（或你也可以只用 actionPhaseChanged） =====
    void requestEndSegEnabled(bool enabled);  // 让 MainWindow 控制 actEndSeg
    void requestNavStep(int step1to5_or_0);   // 0表示不在行动阶段；1..5对应环节
    void actionPhaseChanged(bool active, Side activeSide, int segIndex, GameController::ActionSeg seg);

    // 你已有：日志/行动点变更（可以逐步废弃 actionPointsDelta）
    void logLine(const QString& line, const QColor &color, bool newLine);

    void stateChanged(const GameState& st);
private:
    PieceItem* createPieceFromPixPath(const QString& pixPath);

    QGraphicsScene* m_scene = nullptr;
    PlacementManager*    m_placementManager = nullptr;

    QSet<int> m_eventAllowedRegions;

    bool isCorps(PieceItem* p) const { return p && p->kind() == PieceKind::Corps; }

    bool isLargeCorps(PieceItem* p) const;  // 你按规则自己定
    int  moveCost(PieceItem* p, int distance) const;

    QSet<int> buildBlockedNodesForSide(Side moverSide) const;

    MapGraph m_graph;

    // 维护“占领方”（没有更完整规则时先这么做）
    QHash<int, Side> m_owner;

    void notifyState() { emit stateChanged(m_state); }
    void syncPhaseFlagsToMoveRules();
    bool phaseActive() const { return m_phase.active; }
    Side phaseSide()  const { return m_phase.activeSide; }
    bool inMoveSeg()  const { return m_phase.active && (m_phase.seg == ActionSeg::Move); }

    GameState m_state;
    ActionPhaseState m_phase; // 你已迁入的阶段机
};
