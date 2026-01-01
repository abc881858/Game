#include "view.h"
#include <QToolButton>
#include <QtWidgets>
#include <QtMath>

View::View(QWidget *parent)
    : QWidget{parent}
{
    graphicsView = new GraphicsView(this);
    // graphicsView->setRenderHint(QPainter::Antialiasing, false);
    // graphicsView->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget);
    graphicsView->setOptimizationFlags(QGraphicsView::DontSavePainterState);
    graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    QSize iconSize(size, size);

    QToolButton *zoomInIcon = new QToolButton;
    zoomInIcon->setAutoRepeat(true);
    zoomInIcon->setAutoRepeatInterval(33);
    zoomInIcon->setAutoRepeatDelay(0);
    zoomInIcon->setIcon(QPixmap(":/res/zoomin.png"));
    zoomInIcon->setIconSize(iconSize);
    QToolButton *zoomOutIcon = new QToolButton;
    zoomOutIcon->setAutoRepeat(true);
    zoomOutIcon->setAutoRepeatInterval(33);
    zoomOutIcon->setAutoRepeatDelay(0);
    zoomOutIcon->setIcon(QPixmap(":/res/zoomout.png"));
    zoomOutIcon->setIconSize(iconSize);
    zoomSlider = new QSlider;
    zoomSlider->setMinimum(0);
    zoomSlider->setMaximum(500);
    zoomSlider->setValue(250);
    zoomSlider->setTickPosition(QSlider::TicksRight);

    resetButton = new QToolButton;
    zoomOutIcon->setIconSize(iconSize);
    resetButton->setText(tr("0"));
    resetButton->setEnabled(false);

    QVBoxLayout *zoomSliderLayout = new QVBoxLayout;
    zoomSliderLayout->addWidget(resetButton);
    zoomSliderLayout->addWidget(zoomInIcon);
    zoomSliderLayout->addWidget(zoomSlider);
    zoomSliderLayout->addWidget(zoomOutIcon);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(graphicsView);
    layout->addLayout(zoomSliderLayout);

    setLayout(layout);

    connect(resetButton, &QAbstractButton::clicked, this, &View::resetView);
    connect(zoomSlider, &QAbstractSlider::valueChanged, this, &View::setupMatrix);
    connect(graphicsView->verticalScrollBar(), &QAbstractSlider::valueChanged, this, &View::setResetButtonEnabled);
    connect(graphicsView->horizontalScrollBar(), &QAbstractSlider::valueChanged, this, &View::setResetButtonEnabled);
    connect(zoomInIcon, &QAbstractButton::clicked, this, [=](){ zoomIn(1); });
    connect(zoomOutIcon, &QAbstractButton::clicked, this, [=](){ zoomOut(1); });

    setupMatrix();
}

QGraphicsView *View::view() const
{
    return static_cast<QGraphicsView *>(graphicsView);
}

void View::setupMatrix()
{
    qreal scale = qPow(qreal(2), (zoomSlider->value() - 250) / qreal(50));

    QTransform matrix;
    matrix.scale(scale, scale);

    graphicsView->setTransform(matrix);

    setResetButtonEnabled();
}

void View::zoomIn(int level)
{
    zoomSlider->setValue(zoomSlider->value() + level);
    // qDebug() << __func__ << level << zoomSlider->value() + level;
}

void View::zoomOut(int level)
{
    zoomSlider->setValue(zoomSlider->value() - level);
    // qDebug() << __func__ << level << zoomSlider->value() - level;
}

void View::resetView()
{
    zoomSlider->setValue(250);
    setupMatrix();
    graphicsView->centerOn(0, 0);

    resetButton->setEnabled(false);
}

void View::setResetButtonEnabled()
{
    bool changed = (graphicsView->transform() != QTransform()) ||
                   graphicsView->horizontalScrollBar()->value() != graphicsView->horizontalScrollBar()->minimum() ||
                   graphicsView->verticalScrollBar()->value() != graphicsView->verticalScrollBar()->minimum();

    resetButton->setEnabled(changed);
}
