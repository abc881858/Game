#pragma once

#include <QGraphicsView>
#include <QSet>

class GraphicsFrame;

class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit GraphicsView(GraphicsFrame *graphicsFrame);

protected:
    void wheelEvent(QWheelEvent *) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;

private:
    GraphicsFrame *m_graphicsFrame = nullptr;

signals:
    void dropRequested(QPointF scenePos, QString pixPath, QString eventId, bool isEvent);
};
