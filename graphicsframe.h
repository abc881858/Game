#pragma once

#include <QFrame>
#include <QSlider>
#include <QToolButton>
#include "graphicsview.h"

class GraphicsFrame : public QFrame
{
    Q_OBJECT
public:
    explicit GraphicsFrame(QWidget *parent = nullptr);
    GraphicsView *graphicsView() const;
private:
    GraphicsView *m_graphicsView;
    QSlider *zoomSlider;
    QToolButton *resetButton;
public slots:
    void setupMatrix();
    void zoomIn(int level = 1);
    void zoomOut(int level = 1);
    void setResetButtonEnabled();
    void resetView();
signals:
    void pieceMovedCityToCity(int fromSlotId, int toSlotId, Side side);
};
