#pragma once

#include <QFrame>
#include <QSlider>
#include <QToolButton>
#include "graphicsview.h"

class View : public QFrame
{
    Q_OBJECT
public:
    explicit View(QWidget *parent = nullptr);
    QGraphicsView *view() const;
private:
    GraphicsView *graphicsView;
    QSlider *zoomSlider;
    QToolButton *resetButton;
public slots:
    void setupMatrix();
    void zoomIn(int level = 1);
    void zoomOut(int level = 1);
    void setResetButtonEnabled();
    void resetView();
signals:
};
