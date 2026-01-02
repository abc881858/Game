#pragma once

#include <QObject>
#include <QSet>
#include <QString>

class QGraphicsScene;
class PlacementManager;
class PieceItem;

enum class Side;

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

public slots:
    // 由 GraphicsView 在 dropEvent 中转发过来
    void onPieceDropped(const QString& pixPath, const QString& eventId, int regionId, bool isEvent);

    // 由 GraphicsView 在 dropEvent 中转发过来（行动签）
    void onActionTokenDropped(const QString& pixPath);

    // 由 PieceItem 的右键菜单触发（拆分）
    void onSplitRequested(PieceItem* piece, int a, int b);

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
};
