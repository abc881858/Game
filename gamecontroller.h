#pragma once

#include <QObject>
#include <QSet>
#include <QString>
#include "mapgraph.h"
#include "util.h"
#include "battlecontext.h"
#include "pieceitem.h"
#include "regionitem.h"

class QGraphicsScene;
class PlacementManager;

struct GameState
{
    int turn = 1;  // 1..8（你可在 setter 里 clamp）

    // D
    int npD  = 32;
    int oilD = 10;
    int apD  = 0;
    int rpD  = 0;

    // S
    int npS  = 30;
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
    explicit GameController(QGraphicsScene* scene, QObject* parent=nullptr);

    // ===== 读状态（MainWindow用）=====
    const GameState& state() const { return m_state; }
    void addTurn(int delta);
    void addNationalPower(Side side, int delta);
    void addOil(Side side, int delta);
    void addReadyPoints(Side side, int delta);
    void addAP(Side side, int delta);          // 统一入口：加/扣 AP（含进入行动步骤触发）
    void clearAP(Side side);                  // 规则9.5清零用
    int currentAP(Side side) const;

    void startActionStep(Side side);     // 开始行动步骤（默认 Move）
    void goNextSegment();                // 结束当前环节 -> 进入下一个 or 结束阶段
    void endActionStep();                // 结束整个行动步骤（清状态 + 限制拖拽）

    enum class ActionSegment { Move=0, Battle=1, Redeploy=2, Prepare=3, Supply=4 };

    struct ActionStepState {
        bool active = false;
        Side activeSide = Side::Unknown;
        ActionSegment segment = ActionSegment::Move;
        int segmentIndex = 0; // 0..4
    };

    const ActionStepState& actionStep() const { return m_actionStep; }

    // 事件落子许可格（事件对话框弹出时设置，关闭时清空）
    void setEventAllowedRegions(const QSet<int>& ids) { m_eventAllowedRegions = ids; }
    void clearEventAllowedRegions() { m_eventAllowedRegions.clear(); }

    PieceItem* placeNewPieceToRegion(int regionId, const QString& pixPath, qreal z = 20.0, const QString& eventId = QString(), bool isEvent = false);

    void resetAllPiecesMoveFlag();
    bool canDragActionToken(Side side) const;
    bool canDragUnitInMoveSegment(Side side) const;
    bool canDragUnitInBattleSegment(Side side) const;
    void onPieceDropped(const QString& pixPath, const QString& eventId, int regionId, bool isEvent);
    void onActionTokenDropped(const QString& pixPath);
    void onPieceMovedRegionToRegion(PieceItem* piece, int fromRegionId, int toRegionId, Side side);
    bool inBattleSegment() const { return m_actionStep.active && (m_actionStep.segment == ActionSegment::Battle); }

private:
    PieceItem* createPieceFromPixPath(const QString& pixPath);
    void rollbackToRegion(PieceItem* piece, int regionId, const QString& reason = QString());

    QGraphicsScene* m_scene = nullptr;
    PlacementManager* m_placementManager = nullptr;

    QSet<int> m_eventAllowedRegions;

    bool isCorps(PieceItem* p) const { return p && p->kind() == PieceKind::Corps; }

    bool isLargeCorps(PieceItem* p) const;  // 你按规则自己定
    int  moveCost(PieceItem* p, int distance) const;

    QSet<int> buildBlockedNodesForSide(Side moverSide) const;

    MapGraph m_graph;

    // 维护“占领方”（没有更完整规则时先这么做）
    QHash<int, Side> m_owner;

    bool stepActive() const { return m_actionStep.active; }
    Side stepSide()  const { return m_actionStep.activeSide; }
    bool inMoveSegment()  const { return m_actionStep.active && (m_actionStep.segment == ActionSegment::Move); }

    GameState m_state;
    ActionStepState m_actionStep;

    Side m_nextActionTokenSide = Side::Unknown;

    int hitRegionIdAt(const QPointF& scenePos) const;
    void rollbackToLastValid(PieceItem* piece);
    void snapPieceToRegion(PieceItem* piece, int regionId); // 只负责“吸附+布局+lastValid”

    BattleContext m_battle;

    // 打击群轮流
    Side m_strikeTurn = Side::Unknown;
    bool m_strikeAtkPassed = false;
    bool m_strikeDefPassed = false;

    void tryStartBattleFromMove(PieceItem* piece, int from, int to);
    void openBattleSetupDialog();
    void openStrikeDialog();
    void openBattlefieldDialog();

    QStringList unitPixList(const QList<PieceItem*>& v) const;
    void syncBattleUnitsToDialog();

    enum class BattleStep { None, SetupUnits, SetupStrike, ShowField };
    BattleStep m_battleStep = BattleStep::None;

    int battleDeclareCost(PieceItem *p, int distance) const;

public slots:
    void setFirstPlayerD();
    void setFirstPlayerS();
    void onDropRequested(QPointF scenePos, QString pixPath, QString eventId, bool isEvent);
    void refreshMovablePieces();
    void onSplitRequested(PieceItem* piece, int a, int b);
    void onStrikePass();
    void onPieceDropReleased(PieceItem *piece, const QPointF &sceneCenter);

signals:
    // ===== UI 控制用（或你也可以只用 actionStepChanged） =====
    void setCurrentSegment(int step1to5_or_0);   // 0表示不在行动步骤；1..5对应环节
    void actionStepChanged(bool active, Side activeSide, int segmentIndex, GameController::ActionSegment segment);

    // 你已有：日志/行动点变更（可以逐步废弃 actionPointsDelta）
    void logLine(const QString& line, const QColor &color, bool newLine);

    void stateChanged();

    void battleUnitsChanged(const QStringList& atkPix, const QStringList& defPix);
    void strikeGroupsChanged(const QList<StrikeGroupEntry>& atk,
                             const QList<StrikeGroupEntry>& def,
                             Side currentTurn,
                             bool finished);
};
