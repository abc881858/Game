#pragma once

#include <QObject>
#include <QGraphicsPixmapItem>
#include "util.h"
#include "slotmanager.h"

inline constexpr int PieceType = QGraphicsItem::UserType + 200;

class CitySlotItem;

class PieceItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
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

    void setSlotManager(SlotManager* mgr) { m_slots = mgr; }
    SlotManager* slotManager() const { return m_slots; }
    void markLastValid(int slotId) { m_lastValidSlotId = slotId; m_lastValidPos = pos(); }

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
    int m_level = 0;
    QString m_pixPath;

    CitySlotItem* m_slot = nullptr; // 当前所在格（没有则 nullptr）
    Side m_side = Side::Unknown;    // 你 setUnitMeta 时保存的阵营

    SlotManager* m_slots = nullptr;

public:
    void setInLayout(bool v) { m_inLayout = v; }
    bool inLayout() const { return m_inLayout; }

signals:
    void movedCityToCity(int fromSlotId, int toSlotId, Side side);

    // ✅ 新增：拆分请求（把业务交给 controller）
    void splitRequested(PieceItem* piece, int a, int b);
};
