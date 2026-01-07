#include "segmentwidget.h"
#include "qfontdatabase.h"
#include "qpainter.h"
#include <QPainterPath>

SegmentWidget::SegmentWidget(QWidget *parent) : QWidget(parent)
{
    maxSegment = 5;
    currentSegment = 1;

    topInfo << "step1" << "step2" << "step3" << "step4" << "step5";
    bottomInfo << "2016-11-24 20:57:58" << "2016-11-24 21:55:56";
    navStyle = NavStyle_JD;

    background = QColor(100, 100, 100);
    foreground = QColor(255, 255, 255);
    currentBackground = QColor(100, 184, 255);
    currentForeground = QColor(255, 255, 255);

    setFont(QFont("Microsoft Yahei", 10));
    setCurrentBackground(QColor(24,189,155));
}

void SegmentWidget::paintEvent(QPaintEvent *)
{
    //绘制准备工作,启用反锯齿
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    //根据不一样的样式绘制
    if (navStyle == NavStyle_JD) {
        //绘制背景
        drawBg_JD(&painter);
        //绘制文字
        drawText_JD(&painter);
        //绘制当前背景
        drawCurrentBg_JD(&painter);
        //绘制当前文字
        drawCurrentText_JD(&painter);
    } else if (navStyle == NavStyle_TB) {
        //绘制背景
        drawBg_TB(&painter);
        //绘制文字
        drawText_TB(&painter);
        //绘制当前背景
        drawCurrentBg_TB(&painter);
    } else if (navStyle == NavStyle_ZFB) {
        //绘制背景
        drawBg_ZFB(&painter);
        //绘制文字
        drawText_ZFB(&painter);
        //绘制当前背景
        drawCurrentBg_ZFB(&painter);
    }
}

void SegmentWidget::drawBg_JD(QPainter *painter)
{
    painter->save();

    //圆半径为高度一定比例,计算宽度,将宽度等分
    int width = this->width() / maxSegment;
    int height = this->height() / 2;
    int radius = height / 2;
    int initX = 0;
    int initY = height / 2 + radius / 5;

    //逐个绘制连接线条
    initX = width / 2;

    QPen pen;
    pen.setWidthF((double)radius / 4);
    pen.setCapStyle(Qt::RoundCap);
    pen.setColor(background);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    for (int i = 0; i < maxSegment - 1; i++) {
        painter->drawLine(QPoint(initX, initY), QPoint(initX + width, initY));
        initX += width;
    }

    //逐个绘制圆
    initX = width / 2;
    painter->setPen(Qt::NoPen);
    painter->setBrush(background);

    for (int i = 0; i < maxSegment; i++) {
        painter->drawEllipse(QPoint(initX, initY), radius, radius);
        initX += width;
    }

    //逐个绘制圆中的数字
    initX = width / 2;
    QFont font;
    font.setPixelSize(radius);
    painter->setFont(font);
    painter->setPen(foreground);
    painter->setBrush(Qt::NoBrush);

    for (int i = 0; i < maxSegment; i++) {
        QRect textRect(initX - radius, initY - radius, radius * 2, radius * 2);
        painter->drawText(textRect, Qt::AlignCenter, QString::number(i + 1));
        initX += width;
    }

    painter->restore();
}

void SegmentWidget::drawText_JD(QPainter *painter)
{
    int width = this->width() / maxSegment;
    int height = this->height() / 2;
    int initX = 0;
    int initY = height;

    painter->save();
    QFont font;
    font.setPixelSize(height / 3);
    painter->setFont(font);
    painter->setPen(background);
    painter->setBrush(Qt::NoBrush);

    for (int i = 0; i < maxSegment; i++) {
        QRect textRect(initX, initY, width, height);
        painter->drawText(textRect, Qt::AlignCenter, topInfo.at(i));
        initX += width;
    }

    painter->restore();
}

void SegmentWidget::drawCurrentBg_JD(QPainter *painter)
{
    if (currentSegment <= 0) {
        return;
    }

    painter->save();

    //圆半径为高度一定比例,计算宽度,将宽度等分
    int width = this->width() / maxSegment;
    int height = this->height() / 2;
    int radius = height / 2;
    int initX = 0;
    int initY = height / 2 + radius / 5;
    radius -= radius / 5;

    //逐个绘制连接线条
    initX = width / 2;

    QPen pen;
    pen.setWidthF((double)radius / 7);
    pen.setCapStyle(Qt::RoundCap);
    pen.setColor(currentBackground);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    for (int i = 0; i < currentSegment - 1; i++) {
        painter->drawLine(QPoint(initX, initY), QPoint(initX + width, initY));
        initX += width;
    }

    //如果当前进度超过一个步数且小于最大步数则增加半个线条
    if (currentSegment > 0 && currentSegment < maxSegment) {
        painter->drawLine(QPoint(initX, initY), QPoint(initX + width / 2, initY));
    }

    //逐个绘制圆
    initX = width / 2;
    painter->setPen(Qt::NoPen);
    painter->setBrush(currentBackground);

    for (int i = 0; i < currentSegment; i++) {
        painter->drawEllipse(QPoint(initX, initY), radius, radius);
        initX += width;
    }

    //逐个绘制圆中的数字
    initX = width / 2;
    QFont font;
    font.setPixelSize(radius);
    painter->setFont(font);
    painter->setPen(currentForeground);
    painter->setBrush(Qt::NoBrush);

    for (int i = 0; i < currentSegment; i++) {
        QRect textRect(initX - radius, initY - radius, radius * 2, radius * 2);
        painter->drawText(textRect, Qt::AlignCenter, QString::number(i + 1));
        initX += width;
    }

    painter->restore();
}

void SegmentWidget::drawCurrentText_JD(QPainter *painter)
{
    if (currentSegment <= 0) {
        return;
    }

    int width = this->width() / maxSegment;
    int height = this->height() / 2;
    int initX = 0;
    int initY = height;

    painter->save();
    QFont font;
    font.setPixelSize(height / 3);
    painter->setFont(font);
    painter->setPen(currentBackground);
    painter->setBrush(Qt::NoBrush);

    for (int i = 0; i < currentSegment; i++) {
        QRect textRect(initX, initY, width, height);
        painter->drawText(textRect, Qt::AlignCenter, topInfo.at(i));
        initX += width;
    }

    painter->restore();
}

void SegmentWidget::drawBg_TB(QPainter *painter)
{
    painter->save();

    //圆半径为高度一定比例,计算宽度,将宽度等分
    int width = this->width() / maxSegment;
    int height = this->height() / 3;
    int radius = height / 2;
    int initX = 0;
    int initY = this->height() / 2;

    //逐个绘制连接线条
    initX = width / 2;

    QPen pen;
    pen.setWidthF((double)radius / 4);
    pen.setCapStyle(Qt::RoundCap);
    pen.setColor(background);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    for (int i = 0; i < maxSegment - 1; i++) {
        painter->drawLine(QPoint(initX, initY), QPoint(initX + width, initY));
        initX += width;
    }

    //逐个绘制圆
    initX = width / 2;
    painter->setPen(Qt::NoPen);
    painter->setBrush(background);

    for (int i = 0; i < maxSegment; i++) {
        painter->drawEllipse(QPoint(initX, initY), radius, radius);
        initX += width;
    }

    //逐个绘制圆中的数字
    initX = width / 2;
    QFont font;
    font.setPixelSize(radius);
    painter->setFont(font);
    painter->setPen(foreground);
    painter->setBrush(Qt::NoBrush);

    for (int i = 0; i < maxSegment; i++) {
        QRect textRect(initX - radius, initY - radius, radius * 2, radius * 2);
        painter->drawText(textRect, Qt::AlignCenter, QString::number(i + 1));
        initX += width;
    }

    painter->restore();
}

void SegmentWidget::drawText_TB(QPainter *painter)
{
    int width = this->width() / maxSegment;
    int height = this->height() / 3;
    int initX = 0;
    int initY = 0;

    painter->save();
    QFont font;
    font.setPixelSize(height / 3);
    painter->setFont(font);
    painter->setPen(background);
    painter->setBrush(Qt::NoBrush);

    //绘制上部分文字
    for (int i = 0; i < maxSegment; i++) {
        QRect textRect(initX, initY, width, height);
        painter->drawText(textRect, Qt::AlignCenter, topInfo.at(i));
        initX += width;
    }

    //绘制下部分文字
    initX = 0;
    initY = this->height() / 3 * 2;

    for (int i = 0; i < currentSegment; i++) {
        QRect textRect(initX, initY, width, height);
        painter->drawText(textRect, Qt::AlignCenter, bottomInfo.at(i));
        initX += width;
    }

    painter->restore();
}

void SegmentWidget::drawCurrentBg_TB(QPainter *painter)
{
    if (currentSegment <= 0) {
        return;
    }

    painter->save();

    //圆半径为高度一定比例,计算宽度,将宽度等分
    int width = this->width() / maxSegment;
    int height = this->height() / 3;
    int radius = height / 2;
    int initX = 0;
    int initY = this->height() / 2;
    radius -= radius / 5;

    //逐个绘制连接线条
    initX = width / 2;

    QPen pen;
    pen.setWidthF((double)radius / 7);
    pen.setCapStyle(Qt::RoundCap);
    pen.setColor(currentBackground);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    for (int i = 0; i < currentSegment - 1; i++) {
        painter->drawLine(QPoint(initX, initY), QPoint(initX + width, initY));
        initX += width;
    }

    //如果当前进度超过一个步数且小于最大步数则增加半个线条
    if (currentSegment > 0 && currentSegment < maxSegment) {
        painter->drawLine(QPoint(initX, initY), QPoint(initX + width / 2, initY));
    }

    //逐个绘制圆
    initX = width / 2;
    painter->setPen(Qt::NoPen);
    painter->setBrush(currentBackground);

    for (int i = 0; i < currentSegment; i++) {
        painter->drawEllipse(QPoint(initX, initY), radius, radius);
        initX += width;
    }

    //逐个绘制圆中的字符串
    initX = width / 2;
    iconFont.setPixelSize(radius);
    painter->setFont(iconFont);
    painter->setPen(currentForeground);
    painter->setBrush(Qt::NoBrush);

    //完成字符,可以查看表格更换图形字符
    QString finshStr = QChar(0xf00c);

    for (int i = 0; i < currentSegment; i++) {
        QRect textRect(initX - radius, initY - radius, radius * 2, radius * 2);
        painter->drawText(textRect, Qt::AlignCenter, finshStr);
        initX += width;
    }

    painter->restore();
}

void SegmentWidget::drawBg_ZFB(QPainter *painter)
{
    if (currentSegment <= 0) {
        return;
    }

    painter->save();

    //圆半径为高度一定比例,计算宽度,将宽度等分
    int width = this->width() / maxSegment;
    int height = this->height() / 3;
    int radius = height / 3;
    int initX = 0;
    int initY = this->height() / 2;

    //逐个绘制连接线条
    initX = width / 2;

    QPen pen;
    pen.setWidthF((double)radius / 4);
    pen.setCapStyle(Qt::RoundCap);
    pen.setColor(background);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    for (int i = 0; i < maxSegment - 1; i++) {
        painter->drawLine(QPoint(initX, initY), QPoint(initX + width, initY));
        initX += width;
    }

    //逐个绘制圆
    initX = width / 2;
    painter->setPen(Qt::NoPen);
    painter->setBrush(background);

    for (int i = 0; i < maxSegment; i++) {
        painter->drawEllipse(QPoint(initX, initY), radius, radius);
        initX += width;
    }

    painter->restore();
}

void SegmentWidget::drawText_ZFB(QPainter *painter)
{
    int width = this->width() / maxSegment;
    int height = this->height() / 3;
    int initX = 0;
    int initY = 0;

    painter->save();
    QFont font;
    font.setPixelSize(height / 3);
    painter->setFont(font);
    painter->setPen(background);
    painter->setBrush(Qt::NoBrush);

    //绘制上部分文字
    for (int i = 0; i < maxSegment; i++) {
        QRect textRect(initX, initY, width, height);
        painter->drawText(textRect, Qt::AlignCenter, topInfo.at(i));
        initX += width;
    }

    //绘制下部分文字
    initX = 0;
    initY = this->height() / 3 * 2;

    for (int i = 0; i < maxSegment; i++) {
        QRect textRect(initX, initY, width, height);

        if (i == 0) {
            painter->drawText(textRect, Qt::AlignCenter, bottomInfo.first());
        } else if (i == maxSegment - 1) {
            painter->drawText(textRect, Qt::AlignCenter, bottomInfo.last());
        }

        initX += width;
    }

    painter->restore();
}

void SegmentWidget::drawCurrentBg_ZFB(QPainter *painter)
{
    if (currentSegment <= 0) {
        return;
    }

    painter->save();

    //圆半径为高度一定比例,计算宽度,将宽度等分
    int width = this->width() / maxSegment;
    int height = this->height() / 3;
    int radius = height / 3;
    int initX = 0;
    int initY = this->height() / 2;

    //绘制当前圆
    initX = width / 2;
    painter->setPen(Qt::NoPen);
    painter->setBrush(currentBackground);

    for (int i = 0; i < currentSegment - 1; i++) {
        initX += width;
    }

    painter->drawEllipse(QPoint(initX, initY), radius, radius);


    initX = initX - width / 4;
    initY = 0;
    height = this->height() / 4;

    //绘制当前上部提示信息背景
    QRect bgRect(initX, initY, width / 2, height);
    painter->setBrush(currentBackground);
    painter->drawRoundedRect(bgRect, height / 2, height / 2);

    //绘制当前上部提示信息
    QFont font;
    font.setPixelSize((double)height / 1.9);
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(currentForeground);
    painter->drawText(bgRect, Qt::AlignCenter, topInfo.at(currentSegment - 1));

    //绘制倒三角
    int centerX = initX + width / 4;
    int offset = 10;
    QPolygon pts;
    pts.append(QPoint(centerX - offset, height));
    pts.append(QPoint(centerX + offset, height));
    pts.append(QPoint(centerX, height + offset));

    painter->setPen(Qt::NoPen);
    painter->drawPolygon(pts);

    painter->restore();
}

QStringList SegmentWidget::getTopInfo() const
{
    return this->topInfo;
}

QStringList SegmentWidget::getBottomInfo() const
{
    return this->bottomInfo;
}

int SegmentWidget::getMaxStep() const
{
    return this->maxSegment;
}

int SegmentWidget::getCurrentStep() const
{
    return this->currentSegment;
}

SegmentWidget::NavStyle SegmentWidget::getNavStyle() const
{
    return this->navStyle;
}

QColor SegmentWidget::getBackground() const
{
    return this->background;
}

QColor SegmentWidget::getForeground() const
{
    return this->foreground;
}

QColor SegmentWidget::getCurrentBackground() const
{
    return this->currentBackground;
}

QColor SegmentWidget::getCurrentForeground() const
{
    return this->currentForeground;
}

QSize SegmentWidget::sizeHint() const
{
    return QSize(500, 80);
}

QSize SegmentWidget::minimumSizeHint() const
{
    return QSize(50, 20);
}

void SegmentWidget::setBottomText(const QStringList &topInfo)
{
    if (this->topInfo != topInfo) {
        this->topInfo = topInfo;
        this->update();
    }
}

void SegmentWidget::setBottomInfo(const QStringList &bottomInfo)
{
    if (this->bottomInfo != bottomInfo) {
        this->bottomInfo = bottomInfo;
        this->update();
    }
}

void SegmentWidget::setMaxSegment(int maxSegment)
{
    if (this->maxSegment != maxSegment && maxSegment <= topInfo.count()) {
        this->maxSegment = maxSegment;
        this->update();
    }
}

void SegmentWidget::setCurrentSegment(int currentStep)
{
    if (this->currentSegment != currentStep && currentStep <= maxSegment && currentStep >= 0) {
        this->currentSegment = currentStep;
        this->update();
    }
}

void SegmentWidget::setNavStyle(const SegmentWidget::NavStyle &navStyle)
{
    if (this->navStyle != navStyle) {
        this->navStyle = navStyle;
        this->update();
    }
}

void SegmentWidget::setBackground(const QColor &background)
{
    if (this->background != background) {
        this->background = background;
        this->update();
    }
}

void SegmentWidget::setForeground(const QColor &foreground)
{
    if (this->foreground != foreground) {
        this->foreground = foreground;
        this->update();
    }
}

void SegmentWidget::setCurrentBackground(const QColor &currentBackground)
{
    if (this->currentBackground != currentBackground) {
        this->currentBackground = currentBackground;
        this->update();
    }
}

void SegmentWidget::setCurrentForeground(const QColor &currentForeground)
{
    if (this->currentForeground != currentForeground) {
        this->currentForeground = currentForeground;
        this->update();
    }
}
