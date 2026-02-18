#pragma once

#include <QGraphicsRectItem>
#include <QPainterPath>
#include <QPen>
#include "util.h"

class RegionItem : public QGraphicsRectItem
{
public:
    RegionItem(int id, const QRectF& regionRect);
    int type() const override { return RegionType; }
    int id() const;
    QPainterPath shape() const override;
    QPointF centerScene() const;

    int mining;//工矿
    int oilfield;//油田
    bool is_supply_source;//补给源
    int scoring;//计分
    Nation occupying;//占领国
    Nation local;//本土

private:
    int m_id;
};
