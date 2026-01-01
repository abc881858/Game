#pragma once

#include <QGraphicsView>
#include <QSet>

class View;
class QRubberBand;

class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit GraphicsView(View *v);

    void setEventDropSlots(const QSet<int>& allowedSlotIds) { eventAllowedSlotIds = allowedSlotIds; }
    void clearEventDropSlots() { eventAllowedSlotIds.clear(); }

signals:
    void eventPiecePlaced(const QString& eventId, const QString& pixPath, int slotId);

protected:
    void wheelEvent(QWheelEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
protected:
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;
private:
    View *view = nullptr;
    QRubberBand *rubberBand = nullptr;
    bool selecting = false;
    QPoint origin; // view坐标起点
    QSet<int> eventAllowedSlotIds;
};
