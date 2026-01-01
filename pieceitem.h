#pragma once

#include <QGraphicsPixmapItem>

inline constexpr int PieceType = QGraphicsItem::UserType + 200;

class CitySlotItem;

class PieceItem : public QGraphicsPixmapItem
{
public:
    explicit PieceItem(const QPixmap& pm);

    int type() const override { return PieceType; }

    int slotId() const { return m_slotId; }
    void setSlotId(int id) { m_slotId = id; }
    void snapToNearestCity();
    void placeToSlot(class CitySlotItem* slot);  // 新增

protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;

private:
    void relayoutSlot(CitySlotItem* slot);

    int m_slotId = -1;
    bool m_inLayout = false;

    qreal snapRadius = 100.0; // 吸附半径（scene单位）

    int m_lastValidSlotId = -1;
    QPointF m_lastValidPos;
};
