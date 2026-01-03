#pragma once

#include <QObject>
#include <QSet>
#include <QString>
#include "mapgraph.h"
#include "util.h"
#include "pieceitem.h"

class QGraphicsScene;
class PlacementManager;

class GameController : public QObject
{
    Q_OBJECT
public:
    explicit GameController(QGraphicsScene* scene, PlacementManager* placementManager, QObject* parent=nullptr);

    // 事件落子许可格（事件对话框弹出时设置，关闭时清空）
    void setEventAllowedRegions(const QSet<int>& ids) { m_eventAllowedRegions = ids; }
    void clearEventAllowedRegions() { m_eventAllowedRegions.clear(); }

    PieceItem* placeNewPieceToRegion(int regionId,
                                     const QString& pixPath,
                                     qreal z = 20.0,
                                     const QString& eventId = QString(),
                                     bool isEvent = false);

    void setActionPhaseActive(bool on) { m_actionActive = on; }
    void setActionActiveSide(Side s) { m_activeSide = s; }
    void setInMoveSegment(bool on) { m_inMoveSegment = on; }
    void setCurrentAP(Side side, int ap) { if (side==Side::D) m_apD=ap; else if (side==Side::S) m_apS=ap; }

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
    // 给 MainWindow 订阅：扣/加行动点，或者写日志
    void actionPointsDelta(Side side, int delta);
    void logLine(const QString& line, const QColor &color, bool newLine);

    // 给 EventDialog 订阅：用于从列表里删掉事件棋子计数
    void eventPiecePlaced(const QString& eventId, const QString& pixPath, int regionId);

private:
    PieceItem* createPieceFromPixPath(const QString& pixPath);

    QGraphicsScene* m_scene = nullptr;
    PlacementManager*    m_placementManager = nullptr;

    QSet<int> m_eventAllowedRegions;

    int apOf(Side side) const { return side==Side::D? m_apD : side==Side::S? m_apS : 0; }
    bool isCorps(PieceItem* p) const { return p && p->kind() == PieceKind::Corps; }

    bool isLargeCorps(PieceItem* p) const;  // 你按规则自己定
    int  moveCost(PieceItem* p, int distance) const;

    QSet<int> buildBlockedNodesForSide(Side moverSide) const;

    MapGraph m_graph;

    bool m_actionActive = false;
    bool m_inMoveSegment = false;
    Side m_activeSide = Side::Unknown;
    int m_apD = 0;
    int m_apS = 0;

    // 维护“占领方”（没有更完整规则时先这么做）
    QHash<int, Side> m_owner;

};
