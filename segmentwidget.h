#pragma once

#include <QWidget>
#include <QPainterPath>

class SegmentWidget : public QWidget
{
    Q_OBJECT
    Q_ENUMS(NavStyle)

public:
    enum NavStyle {
        NavStyle_JD = 0,    //京东订单流程样式
        NavStyle_TB = 1,    //淘宝订单流程样式
        NavStyle_ZFB = 2    //支付宝订单流程样式
    };

    explicit SegmentWidget(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *);
    void drawBg_JD(QPainter *painter);
    void drawText_JD(QPainter *painter);
    void drawCurrentBg_JD(QPainter *painter);
    void drawCurrentText_JD(QPainter *painter);
    void drawBg_TB(QPainter *painter);
    void drawText_TB(QPainter *painter);
    void drawCurrentBg_TB(QPainter *painter);
    void drawBg_ZFB(QPainter *painter);
    void drawText_ZFB(QPainter *painter);
    void drawCurrentBg_ZFB(QPainter *painter);

private:
    QStringList topInfo;            //导航顶部标签数据
    QStringList bottomInfo;         //导航底部标签数据

    int maxSegment;                    //最大步数
    int currentSegment;                //当前第几步
    NavStyle navStyle;              //导航样式

    QColor background;              //背景色
    QColor foreground;              //前景色
    QColor currentBackground;       //当前背景色
    QColor currentForeground;       //当前前景色

    QFont iconFont;                 //图形字体

public:
    QStringList getTopInfo()        const;
    QStringList getBottomInfo()     const;

    int getMaxStep()                const;
    int getCurrentStep()            const;
    NavStyle getNavStyle()          const;

    QColor getBackground()          const;
    QColor getForeground()          const;
    QColor getCurrentBackground()   const;
    QColor getCurrentForeground()   const;

    QSize sizeHint()                const;
    QSize minimumSizeHint()         const;

public slots:
    //设置导航顶部标签数据
    void setBottomText(const QStringList &topInfo);
    //设置导航底部标签数据
    void setBottomInfo(const QStringList &bottomInfo);

    //设置最大步数
    void setMaxSegment(int maxSegment);
    //设置当前第几步
    void setCurrentSegment(int currentStep);
    //设置导航样式
    void setNavStyle(const NavStyle &navStyle);

    //设置前景色
    void setBackground(const QColor &background);
    //设置前景色
    void setForeground(const QColor &foreground);
    //设置当前前景色
    void setCurrentBackground(const QColor &currentBackground);
    //设置当前前景色
    void setCurrentForeground(const QColor &currentForeground);
};
