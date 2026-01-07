#include "graphicsframe.h"
#include <QToolButton>
#include <QtWidgets>
#include <QtMath>

GraphicsFrame::GraphicsFrame(QWidget *parent)
    : QFrame{parent}
{
    setFrameStyle(Sunken | StyledPanel);
    m_graphicsView = new GraphicsView(this);
    m_graphicsView->setRenderHint(QPainter::Antialiasing, true);
    m_graphicsView->setOptimizationFlags(QGraphicsView::DontSavePainterState);
    m_graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    m_graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

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
    layout->addWidget(m_graphicsView);
    layout->addLayout(zoomSliderLayout);

    setLayout(layout);

    connect(resetButton, &QAbstractButton::clicked, this, &GraphicsFrame::resetView);
    connect(zoomSlider, &QAbstractSlider::valueChanged, this, &GraphicsFrame::setupMatrix);
    connect(m_graphicsView->verticalScrollBar(), &QAbstractSlider::valueChanged, this, &GraphicsFrame::setResetButtonEnabled);
    connect(m_graphicsView->horizontalScrollBar(), &QAbstractSlider::valueChanged, this, &GraphicsFrame::setResetButtonEnabled);
    connect(zoomInIcon, &QAbstractButton::clicked, this, [=](){ zoomIn(1); });
    connect(zoomOutIcon, &QAbstractButton::clicked, this, [=](){ zoomOut(1); });

    setupMatrix();
}

GraphicsView *GraphicsFrame::graphicsView() const
{
    return m_graphicsView;
}

void GraphicsFrame::setupMatrix()
{
    qreal scale = qPow(qreal(2), (zoomSlider->value() - 250) / qreal(50));

    QTransform matrix;
    matrix.scale(scale, scale);

    m_graphicsView->setTransform(matrix);

    setResetButtonEnabled();
}

void GraphicsFrame::zoomIn(int level)
{
    zoomSlider->setValue(zoomSlider->value() + level);
}

void GraphicsFrame::zoomOut(int level)
{
    zoomSlider->setValue(zoomSlider->value() - level);
}

void GraphicsFrame::resetView()
{
    zoomSlider->setValue(250);
    setupMatrix();
    m_graphicsView->centerOn(0, 0);

    resetButton->setEnabled(false);
}

void GraphicsFrame::setResetButtonEnabled()
{
    bool changed = (m_graphicsView->transform() != QTransform()) ||
                   m_graphicsView->horizontalScrollBar()->value() != m_graphicsView->horizontalScrollBar()->minimum() ||
                   m_graphicsView->verticalScrollBar()->value() != m_graphicsView->verticalScrollBar()->minimum();

    resetButton->setEnabled(changed);
}
