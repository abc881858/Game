#include "slotmanager.h"
#include "cityslotitem.h"
#include "pieceitem.h"
#include <QGraphicsScene>
#include <QtMath>

SlotManager::SlotManager(QGraphicsScene* scene, QObject* parent)
    : QObject(parent), m_scene(scene)
{
}

void SlotManager::addSlot(CitySlotItem* slot)
{
    m_slots.insert(slot->id(), slot);
}

CitySlotItem* SlotManager::slot(int slotId) const
{
    return m_slots.value(slotId, nullptr);
}

int SlotManager::hitTestSlotId(const QPointF& scenePos) const
{
    // 用 scene()->items(scenePos) 可行，但只取 CitySlotItem
    const auto items = m_scene->items(scenePos, Qt::IntersectsItemShape, Qt::DescendingOrder);
    for (auto* it : items) {
        if (it->type() == CitySlotType) {
            return static_cast<CitySlotItem*>(it)->id();
        }
    }
    return -1;
}

void SlotManager::removePiece(PieceItem* piece)
{
    const int sid = piece->slotId();
    if (sid < 0) return;
    auto& vec = m_pieces[sid];
    vec.removeAll(piece);
    relayout(sid);
}

void SlotManager::movePieceToSlot(PieceItem* piece, int newSlotId)
{
    const int old = piece->slotId();
    if (old == newSlotId) {
        relayout(newSlotId);
        return;
    }

    if (old >= 0) {
        auto& vOld = m_pieces[old];
        vOld.removeAll(piece);
        relayout(old);
    }

    piece->setSlotId(newSlotId);
    m_pieces[newSlotId].push_back(piece);
    relayout(newSlotId);
}

QVector<QPointF> SlotManager::makeOffsets(int n, qreal w, qreal h, qreal gap) const
{
    QVector<QPointF> off;
    if (n <= 0) return off;
    if (n == 1) return { {0,0} };
    if (n == 2) return { {-(w+gap)/2,0}, {+(w+gap)/2,0} };

    int cols = qCeil(qSqrt(n));
    int rows = qCeil(double(n)/cols);
    qreal totalW = cols*w + (cols-1)*gap;
    qreal totalH = rows*h + (rows-1)*gap;

    int k=0;
    for (int r=0;r<rows && k<n;++r) {
        for (int c=0;c<cols && k<n;++c,++k) {
            qreal x = -totalW/2 + c*(w+gap) + w/2;
            qreal y = -totalH/2 + r*(h+gap) + h/2;
            off << QPointF(x,y);
        }
    }
    return off;
}

void SlotManager::relayout(int slotId)
{
    auto* s = slot(slotId);
    if (!s) return;

    auto& vec = m_pieces[slotId];
    if (vec.isEmpty()) return;

    std::sort(vec.begin(), vec.end(), [](PieceItem* a, PieceItem* b){ return a < b; });

    qreal w = vec.first()->boundingRect().width();
    qreal h = vec.first()->boundingRect().height();
    qreal gap = 8.0;

    const auto offsets = makeOffsets(vec.size(), w, h, gap);
    const QPointF center = s->centerScene();

    for (int i=0;i<vec.size();++i) {
        auto* p = vec[i];
        p->setInLayout(true);
        QPointF targetCenter = center + offsets[i];
        p->setPos(targetCenter - p->boundingRect().center());
        p->setZValue(20 + i * 0.1);
        p->setInLayout(false);
    }
}
