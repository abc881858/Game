#pragma once

#include <QGraphicsView>
#include <QSet>
#include "util.h"

class GraphicsFrame;
class QRubberBand;
class PlacementManager;

class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit GraphicsView(GraphicsFrame *graphicsFrame);
    void setPlacementManager(PlacementManager* placementManager) { m_placementManager = placementManager; }

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
    QPoint origin;
    PlacementManager* m_placementManager = nullptr;

signals:
    void actionTokenDropped(const QString& pixPath);
    void pieceDropped(const QString& pixPath, const QString& eventId, int regionId, bool isEvent);
};
