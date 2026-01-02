#pragma once

#include <QGraphicsRectItem>
#include <QPainterPath>
#include <QPen>

inline constexpr int RegionType = QGraphicsItem::UserType + 100;

class RegionItem : public QGraphicsRectItem
{
public:
    RegionItem(int id, const QRectF& regionRect);
    int type() const override;
    int id() const;
    QPainterPath shape() const override;
    QPointF centerScene() const;

private:
    int m_id;
};
