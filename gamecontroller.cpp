#include "gamecontroller.h"

#include <QGraphicsScene>
#include <QPixmap>
#include <QDebug>

#include "slotmanager.h"
#include "pieceitem.h"
#include "util.h"

static inline bool isActionTokenPath(const QString& pixPath)
{
    return pixPath.contains("_XDQ", Qt::CaseSensitive);
}

GameController::GameController(QGraphicsScene* scene, SlotManager* slotMgr, QObject* parent)
    : QObject(parent)
    , m_scene(scene)
    , m_slotMgr(slotMgr)
{
}

PieceItem* GameController::spawnToSlot(int slotId, const QString& pixPath, qreal z)
{
    if (!m_scene || !m_slotMgr) return nullptr;
    if (slotId < 0) return nullptr;

    // 行动签不应该走这里（初始化也不会用行动签）
    if (isActionTokenPath(pixPath)) return nullptr;

    auto* item = createPieceFromPixPath(pixPath);
    if (!item) return nullptr;

    item->setZValue(z);
    m_scene->addItem(item);

    m_slotMgr->movePieceToSlot(item, slotId);
    item->markLastValid(slotId);

    return item;
}

PieceItem* GameController::createPieceFromPixPath(const QString& pixPath)
{
    QPixmap pm(pixPath);
    if (pm.isNull()) return nullptr;

    auto* item = new PieceItem(pm);

    // meta
    Side side; int lvl;
    if (parseCorpsFromPixPath(pixPath, side, lvl)) {
        item->setUnitMeta(UnitKind::Corps, side, lvl, pixPath);
    } else {
        item->setUnitMeta(UnitKind::Other, Side::Unknown, 0, pixPath);
    }

    // 让 PieceItem 拥有 slotMgr，用于拖动松手吸附/回滚
    item->setSlotManager(m_slotMgr);

    // 让 controller 接住拆分请求（右键菜单）
    connect(item, &PieceItem::splitRequested, this, &GameController::onSplitRequested);

    return item;
}

PieceItem* GameController::createCorpsPiece(Side side, int level)
{
    const QString path = corpsPixPath(side, level);
    return createPieceFromPixPath(path);
}

void GameController::onPieceDropped(const QString& pixPath,
                                   const QString& eventId,
                                   int slotId,
                                   bool isEvent)
{
    if (!m_scene || !m_slotMgr) return;

    // 规则：事件棋子必须在允许格
    if (isEvent) {
        if (!m_eventAllowedSlots.contains(slotId)) {
            emit logLine(QString("事件落子失败：slot %1 不在允许列表").arg(slotId));
            return;
        }
    }

    // 行动签不走这里（应该由 GraphicsView/Controller 直接识别）
    if (isActionTokenPath(pixPath)) return;

    auto* item = createPieceFromPixPath(pixPath);
    if (!item) return;

    item->setZValue(20);
    m_scene->addItem(item);

    // ✅ 统一由 SlotManager 维护归属 + 重排
    m_slotMgr->movePieceToSlot(item, slotId);

    // ✅ 初始化回滚点（第一次拖动松手才能回滚）
    item->markLastValid(slotId);

    // 事件对话框计数扣减
    if (isEvent) {
        emit eventPiecePlaced(eventId, pixPath, slotId);
    }
}

void GameController::onActionTokenDropped(const QString& pixPath)
{
    qDebug() << "onActionTokenDropped";
    // 行动签效果：交给 MainWindow 改状态（你已有 addActionPoints）
    if (!isActionTokenPath(pixPath)) return;

    Side side = Side::Unknown;
    if (pixPath.startsWith(":/D/")) side = Side::D;
    else if (pixPath.startsWith(":/S/")) side = Side::S;

    int ap = 0;
    if (pixPath.contains("XDQ2")) ap = 2;
    else if (pixPath.contains("XDQ6")) ap = 6;

    if (side == Side::Unknown || ap == 0) return;

    emit actionPointsDelta(side, ap);
    emit logLine(QString("行动签：%1 AP +%2").arg(side==Side::D ? "德国" : "苏联").arg(ap));
}

void GameController::onSplitRequested(PieceItem* piece, int a, int b)
{
    if (!piece || !m_scene || !m_slotMgr) return;

    const int slotId = piece->slotId();
    if (slotId < 0) return;

    const Side side = piece->side();

    // 1) 从 SlotManager 移除（会重排原城市）
    m_slotMgr->removePiece(piece);

    // 2) 从 scene 删除旧棋子
    m_scene->removeItem(piece);
    piece->deleteLater(); // ✅ 避免 delete this 风险

    // 3) 生成两个新兵团并放回同城
    auto spawnOne = [&](int lvl){
        auto* ni = createCorpsPiece(side, lvl);
        if (!ni) return;

        ni->setZValue(20);
        m_scene->addItem(ni);

        m_slotMgr->movePieceToSlot(ni, slotId);
        ni->markLastValid(slotId);
    };

    spawnOne(a);
    spawnOne(b);

    emit logLine(QString("拆分：slot %1  %2 -> %3 + %4")
                 .arg(slotId)
                 .arg(piece->level())
                 .arg(a)
                 .arg(b));
}
