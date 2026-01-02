#pragma once

#include <QObject>
#include <QHash>
#include <QVector>

class QGraphicsScene;
class CitySlotItem;
class PieceItem;

class SlotManager : public QObject
{
    Q_OBJECT
public:
    explicit SlotManager(QGraphicsScene* scene, QObject* parent=nullptr);

    void addSlot(CitySlotItem* slot);               // 注册slot
    CitySlotItem* slot(int slotId) const;

    int  hitTestSlotId(const QPointF& scenePos) const; // 命中slotId，-1表示未命中

    void movePieceToSlot(PieceItem* piece, int newSlotId); // 维护piecesInSlot并重排
    void removePiece(PieceItem* piece);                    // 删除/拆分前调用

    void relayout(int slotId); // 只重排一个城

private:
    QGraphicsScene* m_scene{};
    QHash<int, CitySlotItem*> m_slots;
    QHash<int, QVector<PieceItem*>> m_pieces;

    QVector<QPointF> makeOffsets(int n, qreal w, qreal h, qreal gap) const;
};
