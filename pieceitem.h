#pragma once

#include <QGraphicsPixmapItem>
#include "util.h"

inline constexpr int PieceType = QGraphicsItem::UserType + 200;

class CitySlotItem;

class PieceItem : public QGraphicsPixmapItem
{
public:
    explicit PieceItem(const QPixmap& pm);

    void setUnitMeta(UnitKind kind, Side side, int level, const QString& pixPath) {
        m_kind = kind;
        m_side = side;
        m_level = level;
        m_pixPath = pixPath;
    }

    UnitKind kind() const { return m_kind; }
    Side side() const { return m_side; }
    int level() const { return m_level; }
    QString pixPath() const { return m_pixPath; }

    int type() const override { return PieceType; }

    int slotId() const { return m_slotId; }
    void setSlotId(int id) { m_slotId = id; }
    void snapToNearestCity();
    void placeToSlot(class CitySlotItem* slot);  // 新增

protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* e) override;

private:
    void relayoutSlot(CitySlotItem* slot);

    int m_slotId = -1;
    bool m_inLayout = false;

    qreal snapRadius = 100.0; // 吸附半径（scene单位）

    int m_lastValidSlotId = -1;
    QPointF m_lastValidPos;

    UnitKind m_kind = UnitKind::Other;
    Side m_side = Side::Unknown;
    int m_level = 0;
    QString m_pixPath; // 原资源路径
};
