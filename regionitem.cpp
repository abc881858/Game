#include "regionitem.h"

RegionItem::RegionItem(int id, const QRectF &regionRect)
    : QGraphicsRectItem(QRectF(0,0, regionRect.width(), regionRect.height()))
    , m_id(id)
{
    setPos(regionRect.topLeft());
    setPen(QPen(Qt::NoPen));
    setBrush(Qt::transparent);
    setZValue(1);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
}



int RegionItem::id() const
{
    return m_id;
}

QPainterPath RegionItem::shape() const
{
    QPainterPath p;
    p.addRect(rect());
    return p;
}

QPointF RegionItem::centerScene() const
{
    return mapToScene(rect().center());
}
