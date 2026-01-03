#include "gamecontroller.h"

#include <QGraphicsScene>
#include <QPixmap>
#include <QDebug>

#include "placementmanager.h"
#include "util.h"

static inline bool isActionTokenPath(const QString& pixPath)
{
    return pixPath.contains("_XDQ", Qt::CaseSensitive);
}

GameController::GameController(QGraphicsScene* scene, PlacementManager* placementManager, QObject* parent)
    : QObject(parent)
    , m_scene(scene)
    , m_placementManager(placementManager)
{
}

PieceItem* GameController::createPieceFromPixPath(const QString& pixPath)
{
    QPixmap pm(pixPath);
    if (pm.isNull()) return nullptr;

    auto* item = new PieceItem(pm);

    connect(item, &PieceItem::movedRegionToRegion, this, &GameController::onPieceMovedRegionToRegion);

    // meta
    Side side;
    int lvl;
    if (parseCorpsFromPixPath(pixPath, side, lvl)) {
        item->setUnitMeta(PieceKind::Corps, side, lvl, pixPath);
    } else {
        item->setUnitMeta(PieceKind::Other, Side::Unknown, 0, pixPath);
    }

    // 让 PieceItem 拥有 placementManager/回滚
    item->setPlacementManager(m_placementManager);

    // 让 controller 接住拆分请求（右键菜单）
    connect(item, &PieceItem::splitRequested, this, &GameController::onSplitRequested);

    return item;
}

void GameController::onPieceDropped(const QString& pixPath,
                                   const QString& eventId,
                                   int regionId,
                                   bool isEvent)
{
    // 行动签不走这里（保险）
    if (isActionTokenPath(pixPath)) return;

    placeNewPieceToRegion(regionId, pixPath, 20.0, eventId, isEvent);
}

void GameController::onActionTokenDropped(const QString& pixPath)
{
    if (!isActionTokenPath(pixPath)) return;

    Side side = Side::Unknown;
    if (pixPath.startsWith(":/D/")) side = Side::D;
    else if (pixPath.startsWith(":/S/")) side = Side::S;

    int ap = 0;
    if (pixPath.contains("XDQ2")) ap = 2;
    else if (pixPath.contains("XDQ6")) ap = 6;
    if (ap == 0) return;

    addAP(side, ap);

    emit logLine(QString("行动签：%1 AP +%2\n").arg(side==Side::D ? "德国" : "苏联").arg(ap), Qt::black, true);
}

void GameController::onSplitRequested(PieceItem* piece, int a, int b)
{
    if (!piece || !m_scene || !m_placementManager) return;

    const int regionId = piece->regionId();
    if (regionId < 0) return;

    const Side side = piece->side();

    // 1) 从 PlacementManager 移除（会重排原城市）
    m_placementManager->removePieceItem(piece);

    // 2) 从 scene 删除旧棋子
    m_scene->removeItem(piece);
    piece->deleteLater(); // ✅ 避免 delete this 风险

    // 3) 生成两个新兵团并放回同城
    auto spawnOne = [&](int lvl){
        const QString path = corpsPixPath(side, lvl);
        placeNewPieceToRegion(regionId, path, 20.0);
    };

    spawnOne(a);
    spawnOne(b);

    emit logLine(QString("拆分：region %1  %2 -> %3 + %4")
                 .arg(regionId)
                 .arg(piece->level())
                 .arg(a)
                 .arg(b), Qt::black, true);
}

PieceItem* GameController::placeNewPieceToRegion(int regionId,
                                                 const QString& pixPath,
                                                 qreal z,
                                                 const QString& eventId,
                                                 bool isEvent)
{
    if (!m_scene || !m_placementManager) return nullptr;
    if (regionId < 0) return nullptr;
    if (pixPath.isEmpty()) return nullptr;

    // 行动签不生成棋子
    if (isActionTokenPath(pixPath)) return nullptr;

    // 事件落子合法性（如果你希望只有 drop 入口才检查，也可以把这段挪出去）
    if (isEvent && !m_eventAllowedRegions.contains(regionId)) {
        emit logLine(QString("事件落子失败：region %1 不在允许列表").arg(regionId), Qt::black, true);
        return nullptr;
    }

    auto* item = createPieceFromPixPath(pixPath);
    if (!item) return nullptr;

    item->setZValue(z);
    m_scene->addItem(item);

    m_placementManager->movePieceToRegion(item, regionId);
    item->markLastValid(regionId);

    if (isEvent) {
        emit eventPiecePlaced(eventId, pixPath, regionId);
    }

    return item;
}

bool GameController::isLargeCorps(PieceItem* p) const
{
    // 3-4级算“大兵团”，1-2级算“小兵团”
    return p && p->level() >= 3;
}

int GameController::moveCost(PieceItem* p, int distance) const
{
    // 10.2：距离0按距离1算
    int d = (distance <= 0 ? 1 : distance);

    if (isLargeCorps(p)) {
        // 大兵团：每距离1消耗1AP
        return d;
    } else {
        // 小兵团：每距离2消耗1AP，距离1也要1AP
        // 等价：ceil(d / 2.0)
        return (d + 1) / 2;
    }
}

QSet<int> GameController::buildBlockedNodesForSide(Side moverSide) const
{
    QSet<int> blocked;
    if (!m_placementManager) return blocked;

    // “只能进入没有敌方兵团或要塞的格”
    // 这里先按“敌方兵团存在则阻挡”，要塞你之后做 PieceKind::Fortress 再加上。
    for (int rid = 0; rid < 35; ++rid) {
        const auto pieces = m_placementManager->piecesInRegion(rid);
        for (auto* p : pieces) {
            if (!p) continue;
            if (p->kind() == PieceKind::Corps && p->side() != moverSide) {
                blocked.insert(rid);
                break;
            }
            if (p->kind() == PieceKind::Fortress && p->side() != moverSide) {
                blocked.insert(rid);
                break;
            }
        }
    }
    return blocked;
}

void GameController::resetAllPiecesMoveFlag()
{
    if (!m_scene) return;
    for (auto* gi : m_scene->items()) {
        if (gi->type() != PieceType) continue;
        auto* p = static_cast<PieceItem*>(gi);
        p->setMovedThisActionPhase(false);
    }
}

void GameController::refreshMovablePieces()
{
    if (!m_scene) return;

    const Side side = phaseSide();
    const int ap = currentAP(side);

    for (auto* gi : m_scene->items()) {
        if (gi->type() != PieceType) continue;

        auto* p = static_cast<PieceItem*>(gi);

        bool movable =
            phaseActive() &&
            inMoveSeg() &&
            (side != Side::Unknown) &&
            (p->kind() == PieceKind::Corps) &&
            (p->side() == side) &&
            (ap > 0);

        if (movable && p->movedThisActionPhase())
            movable = false;

        p->setFlag(QGraphicsItem::ItemIsMovable, movable);
        p->setFlag(QGraphicsItem::ItemIsSelectable, true);
    }
}

bool GameController::canDragFromReserve(Side side) const
{
    if (side == Side::Unknown) return false;

    // 没进入行动阶段：不允许
    if (!phaseActive()) return false;

    // 不是陆上移动环节：不允许
    if (!inMoveSeg()) return false;

    // 不是当前行动方：不允许
    if (side != phaseSide()) return false;

    // AP=0：不允许
    if (currentAP(phaseSide()) <= 0) return false;

    return true;
}

void GameController::rollbackToRegion(PieceItem* piece, int regionId, const QString& reason)
{
    if (!piece || !m_placementManager) return;
    if (regionId < 0) return;

    if (!reason.isEmpty()) {
        emit logLine(reason, Qt::black, true);
    }

    // 1) 图元位置：回到 lastValidPos（你已有 snap/mark 体系）
    piece->setInLayout(true);
    piece->setPos(piece->lastValidPos());
    piece->setInLayout(false);

    // 2) 逻辑位置：放回 region（会触发该 region 的重排布局）
    m_placementManager->movePieceToRegion(piece, regionId);

    // 3) lastValid 仍然保持为 regionId（避免出现 lastValid 与当前 region 不一致）
    piece->markLastValid(regionId);
}

void GameController::onPieceMovedRegionToRegion(PieceItem *piece, int fromRegionId, int toRegionId, Side side)
{
    if (!piece) return;
    if (!phaseActive() || !inMoveSeg()) return;// 只在“陆上移动环节”生效
    if (side != phaseSide()) return;// 只能当前行动方移动
    if (!m_placementManager) return;

    // 已因“进入敌占格”被锁住，则回滚
    if (piece->movedThisActionPhase()) {
        rollbackToRegion(piece, fromRegionId, "本行动阶段该兵团已进入敌占格，不能继续移动，已回滚。\n");
        return;
    }

    // 最短路距离（中途不能经过 blocked）
    QSet<int> blocked = buildBlockedNodesForSide(side);
    blocked.remove(fromRegionId); // 起点不算阻挡
    int dist = m_graph.shortestDistance(fromRegionId, toRegionId, blocked);
    if (dist < 0) {
        rollbackToRegion(piece, fromRegionId, "移动失败：无可达陆上路线（可能被敌方兵团/要塞阻挡），已回滚。\n");
        return;
    }

    const int cost = moveCost(piece, dist);
    const int apNow = currentAP(side);

    if (apNow < cost) {
        rollbackToRegion(piece, fromRegionId, QString("移动失败：距离=%1 需AP=%2 当前AP=%3，不足，已回滚。\n").arg(dist).arg(cost).arg(apNow));
        return;
    }

    addAP(side, -cost);
    emit logLine(QString("陆上移动：%1 -> %2 距离=%3 扣AP=%4\n").arg(fromRegionId).arg(toRegionId).arg(dist).arg(cost), Qt::black, true);

    // 占领判定：如果进入敌占格，则占领并锁住本阶段继续移动
    Side owner = m_owner.value(toRegionId, Side::Unknown);
    if (owner != Side::Unknown && owner != side) {
        m_owner[toRegionId] = side;
        piece->setMovedThisActionPhase(true);
        emit logLine("进入敌占格：占领完成，本行动阶段该兵团不能继续移动。\n", Qt::black, true);
    } else if (owner == Side::Unknown) {
        // 没维护过占领方时，至少写入当前方
        m_owner[toRegionId] = side;
    }

    // 这次移动成功，更新 lastValid
    piece->markLastValid(toRegionId);
}

bool GameController::canDragUnitInMoveSeg(Side side) const
{
    if (!phaseActive()) return false;
    if (!inMoveSeg()) return false;
    if (side != phaseSide()) return false;
    if (currentAP(phaseSide()) <= 0) return false;
    return true;
}

void GameController::startActionPhase(Side side)
{
    if (side == Side::Unknown) return;
    if (m_phase.active) return;// 如果已经在行动阶段：一般不允许再次 start（按你规则也可以直接 return）

    m_phase.active = true;
    m_phase.activeSide = side;
    m_phase.segIndex = 0;
    m_phase.seg = ActionSeg::Move;

    // 进入行动阶段：重置“本阶段锁定继续移动”标记
    resetAllPiecesMoveFlag();

    // 同步移动规则开关 + 刷新可拖拽
    syncPhaseFlagsToMoveRules();

    // 通知 UI
    emit requestEndSegEnabled(true);
    emit requestNavStep(1); // Move=1
    emit actionPhaseChanged(true, m_phase.activeSide, m_phase.segIndex, m_phase.seg);

    // 日志（走你已有信号）
    emit logLine(QString("%1打出行动签：进入行动阶段（从【陆上移动】开始）\n").arg(side==Side::D ? "德国" : "苏联"), Qt::black, true);
}

void GameController::advanceSegment()
{
    if (!m_phase.active) return;

    if (m_phase.segIndex < 4) {
        m_phase.segIndex++;
        m_phase.seg = ActionSeg(m_phase.segIndex);

        // 切换环节：同步移动规则（只有 Move 环节允许移动）
        syncPhaseFlagsToMoveRules();

        // UI 更新：NavProgress 用 1..5
        emit requestNavStep(m_phase.segIndex + 1);
        emit actionPhaseChanged(true, m_phase.activeSide, m_phase.segIndex, m_phase.seg);

        // 日志提示
        switch (m_phase.segIndex) {
        case 1: emit logLine("陆上移动结束，进入陆战环节。\n", Qt::black, true); break;
        case 2: emit logLine("陆战结束，进入调动环节。\n", Qt::black, true); break;
        case 3: emit logLine("调动结束，进入准备环节。\n", Qt::black, true); break;
        case 4: emit logLine("准备结束，进入补给环节。\n", Qt::black, true); break;
        default: break;
        }
        return;
    }

    // segIndex==4: 结束补给 -> 行动阶段结束
    emit logLine("补给结束：本行动阶段结束。\n", Qt::black, true);
    endActionPhase();
}

void GameController::endActionPhase()
{
    if (!m_phase.active) return;

    const Side finishedSide = m_phase.activeSide;

    // 规则9.5：AP清零
    clearAP(finishedSide);

    Side nextSide = Side::Unknown;
    if (finishedSide == Side::D) nextSide = Side::S;
    else if (finishedSide == Side::S) nextSide = Side::D;

    // 清阶段状态
    m_phase.active = false;
    m_phase.activeSide = Side::Unknown;
    m_phase.segIndex = 0;
    m_phase.seg = ActionSeg::Move;

    syncPhaseFlagsToMoveRules();

    emit requestNavStep(0);
    emit requestEndSegEnabled(false);
    emit actionPhaseChanged(false, Side::Unknown, 0, ActionSeg::Move);

    emit logLine(QString("行动阶段结束：下一张行动签由【%1】打出。\n")
                 .arg(nextSide==Side::D ? "德国" : "苏联"),
                 Qt::black, true);
}

void GameController::syncPhaseFlagsToMoveRules()
{
    refreshMovablePieces();
}

int GameController::currentAP(Side side) const
{
    return m_state.ap(side);
}

void GameController::addTurn(int delta)
{
    m_state.turn += delta;
    // 可选：限制 1..8
    if (m_state.turn < 1) m_state.turn = 1;
    if (m_state.turn > 8) m_state.turn = 8;

    emit stateChanged();
}

void GameController::addNationalPower(Side side, int delta)
{
    if (side!=Side::D && side!=Side::S) return;
    m_state.npRef(side) += delta;

    emit stateChanged();
}

void GameController::addOil(Side side, int delta)
{
    if (side!=Side::D && side!=Side::S) return;
    m_state.oilRef(side) += delta;

    emit stateChanged();
}

void GameController::addReadyPoints(Side side, int delta)
{
    if (side!=Side::D && side!=Side::S) return;
    m_state.rpRef(side) += delta;

    emit stateChanged();
}

void GameController::addAP(Side side, int delta)
{
    if (side!=Side::D && side!=Side::S) return;

    m_state.apRef(side) += delta;

    emit stateChanged();

    // 行动阶段：delta>0 且未active -> start
    if (delta > 0 && !m_phase.active) {
        startActionPhase(side);
    } else {
        refreshMovablePieces();
    }

    const QString who = (side == Side::D ? "德国" : "苏联");
    emit logLine(QString("%1行动点 %2%3，当前：D=%4 S=%5\n")
                 .arg(who)
                 .arg(delta >= 0 ? "+" : "")
                 .arg(delta)
                 .arg(m_state.apD)
                 .arg(m_state.apS),
                 Qt::black, true);
}

void GameController::clearAP(Side side)
{
    if (side!=Side::D && side!=Side::S) return;
    m_state.apRef(side) = 0;

    emit stateChanged();

    refreshMovablePieces();
}
