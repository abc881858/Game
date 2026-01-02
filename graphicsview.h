#pragma once

#include <QGraphicsView>
#include <QSet>
#include "util.h"

class GraphicsFrame;
class QRubberBand;
class SlotManager;

class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit GraphicsView(GraphicsFrame *graphicsFrame);
    void setEventDropSlots(const QSet<int>& allowedSlotIds) { eventAllowedSlotIds = allowedSlotIds; }
    void clearEventDropSlots() { eventAllowedSlotIds.clear(); }
    void setSlotManager(SlotManager* mgr) { m_slotMgr = mgr; }

protected:
    void wheelEvent(QWheelEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;

private:
    GraphicsFrame *m_graphicsFrame = nullptr;
    QRubberBand *rubberBand = nullptr;
    bool selecting = false;
    QPoint origin; // view坐标起点
    QSet<int> eventAllowedSlotIds;
    SlotManager* m_slotMgr = nullptr;

signals:
    void eventPiecePlaced(const QString& eventId, const QString& pixPath, int slotId);
    void piecePlaced(const QString& pixResPath, int slotId);
    void pieceMovedCityToCity(int fromSlotId, int toSlotId, Side side);
    void actionTokenDropped(const QString& pixPath);

    void pieceDropped(const QString& pixPath, const QString& eventId, int slotId, bool isEvent);
};
