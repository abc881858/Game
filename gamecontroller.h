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
    int turn = 1;

    int npD  = 32;
    int oilD = 10;
    int apD  = 0;
    int rpD  = 0;

    int npS  = 30;
    int apS  = 0;
    int rpS  = 0;

    int& apRef(Side s)  { return (s==Side::D)? apD : apS; }
    int& npRef(Side s)  { return (s==Side::D)? npD : npS; }
    int& oilRef() { return oilD; }
    int& rpRef(Side s)  { return (s==Side::D)? rpD : rpS; }
};

enum class ActionSegment { Move=0, Battle=1, Redeploy=2, Prepare=3, Supply=4 };

struct ActionStepState {
    bool active = false;
    Side activeSide = Side::Unknown;
    ActionSegment segment = ActionSegment::Move;
    int segmentIndex = 0; // 0,1,2,3,4
};

class GameController : public QObject
{
    Q_OBJECT
public:
    explicit GameController(QGraphicsScene* scene, QObject* parent=nullptr);
    PieceItem* createPieceToRegion(int regionId, const QString& pixPath);
    const GameState& gameState() const { return m_gameState; }
    void setTurn(int turn);
    void addNationalPower(Side side, int delta);
    void addOil(int delta);
    void addReadyPoints(Side side, int delta);
    void addAP(Side side, int delta);    // 统一入口：加/扣 AP（含进入行动步骤触发）
    void clearAP(Side side);             // 规则9.5清零用
    int currentAP(Side side) const;
    void setEventAllowedRegions(const QSet<int>& ids) { m_eventAllowedRegions = ids; }
    void clearEventAllowedRegions() { m_eventAllowedRegions.clear(); }
    void startActionStep(Side side);     // 开始行动步骤（默认 Move）
    void endActionStep();                // 结束整个行动步骤（清状态 + 限制拖拽）
    void goNextSegment();                // 结束当前环节 -> 进入下一个 or 结束阶段
    void rollbackPieceToRegion(PieceItem* piece, int regionId, const QString& reason = QString());
    bool canDragActionToken(Side side) const;
    bool canDragPieceInMoveSegment(Side side) const;
    bool canDragPieceInBattleSegment(Side side) const;

private:
    QHash<int, Side> m_owner;//开局占领国要修正
    MapGraph m_graph;
    QSet<int> m_eventAllowedRegions;
    GameState m_gameState;
    ActionStepState m_actionStep;
    QGraphicsScene* m_scene = nullptr;
    PlacementManager* m_placementManager = nullptr;
    Side m_nextActionTokenSide = Side::Unknown;


    // 打击群轮流
    BattleContext m_battle;
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

public slots:
    void refreshMovablePieces();
    void setFirstPlayerD();
    void setFirstPlayerS();
    void dropPieceToScene(QPointF scenePos, QString pixPath);
    void splitPieceToRegion(PieceItem* piece, int a, int b);
    void movePieceToRegion(PieceItem *piece, const QPointF &sceneCenter);

    void onStrikePass();

signals:
    void setCurrentSegment(int step1to5_or_0);   // 0表示不在行动步骤；1..5对应环节
    void logLine(const QString& line, const QColor &color, bool newLine);
    void stateChanged();

    void battleUnitsChanged(const QStringList& atkPix, const QStringList& defPix);
    void strikeGroupsChanged(const QList<StrikeGroupEntry>& atk, const QList<StrikeGroupEntry>& def, Side currentTurn, bool finished);
};
