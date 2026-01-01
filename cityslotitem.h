#pragma once

#include <QGraphicsRectItem>
#include <QPainterPath>
#include <QPen>
#include <QColor>

inline constexpr int CitySlotType = QGraphicsItem::UserType + 100;

class CitySlotItem : public QGraphicsRectItem
{
public:
    CitySlotItem(int id, const QRectF& slotRect)
        : QGraphicsRectItem(QRectF(0,0, slotRect.width(), slotRect.height()))
        , m_id(id)
    {
        setPos(slotRect.topLeft());
        // setPen(QPen(Qt::red, 3));
        // setBrush(QColor(255,0,0,40));
        setPen(QPen(Qt::NoPen));
        setBrush(Qt::transparent);
        setZValue(1);
        setFlag(QGraphicsItem::ItemIsMovable, false);
        setFlag(QGraphicsItem::ItemIsSelectable, false);
    }

    int type() const override { return CitySlotType; }
    int id() const { return m_id; }

    QPainterPath shape() const override {
        QPainterPath p;
        p.addRect(rect());
        return p;
    }

    QPointF centerScene() const {
        return mapToScene(rect().center());
    }

private:
    int m_id;
};
