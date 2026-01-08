#pragma once

#include <QObject>
#include <QGraphicsPixmapItem>
#include "util.h"

inline constexpr int PieceType = QGraphicsItem::UserType + 200;

class RegionItem;

class PieceItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit PieceItem(const QPixmap& pm);

    void setUnitMeta(PieceKind kind, Side side, int level, const QString& pixPath) {
        m_kind = kind;
        m_side = side;
        m_level = level;
        m_pixPath = pixPath;
    }

    PieceKind kind() const { return m_kind; }
    Side side() const { return m_side; }
    int level() const { return m_level; }
    QString pixPath() const { return m_pixPath; }

    int type() const override { return PieceType; }

    int regionId() const { return m_regionId; }
    void setRegionId(int id) { m_regionId = id; }

    void markLastValid(int regionId) { m_lastValidRegionId = regionId; m_lastValidPos = pos(); }

    void setInLayout(bool v) { m_inLayout = v; }

    QPointF lastValidPos() const { return m_lastValidPos; }
    int lastValidRegionId() const { return m_lastValidRegionId; }

    bool movedThisActionStep() const { return m_movedThisActionStep; }
    void setMovedThisActionStep(bool v) { m_movedThisActionStep = v; }

protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* e) override;

private:
    int m_regionId = -1;
    bool m_inLayout = false;

    int m_lastValidRegionId = -1;
    QPointF m_lastValidPos;

    PieceKind m_kind = PieceKind::Other;
    int m_level = 0;
    QString m_pixPath;

    RegionItem* m_regionItem = nullptr; // 当前所在格（没有则 nullptr）
    Side m_side = Side::Unknown;    // 你 setUnitMeta 时保存的阵营

    bool m_movedThisActionStep = false;

signals:
    void splitRequested(PieceItem* piece, int a, int b);
    void dropReleased(PieceItem* self, const QPointF& sceneCenter);
};
