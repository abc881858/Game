#include "gamecontroller.h"

#include <QGraphicsScene>
#include <QPixmap>
#include <QDebug>

#include "placementmanager.h"
#include "util.h"

#include "battlesetupdialog.h"
#include "strikegroupdialog.h"
#include "battlefielddialog.h"
#include <QMessageBox>

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
    if (isActionTokenPath(pixPath)) return;

    // ===== 战斗：打击群投入阶段 =====
    if (m_battle.active && m_battleStep == BattleStep::SetupStrike) {
        // 只有轮到的一方能投入
        if (phaseSide() == Side::Unknown) {} // 不依赖行动阶段也行
        if (m_strikeTurn == Side::Unknown) return;

        // 过滤兵团（兵团不走这里，兵团靠拖进敌占区触发）
        Side s; int lvl;
        if (parseCorpsFromPixPath(pixPath, s, lvl)) {
            emit logLine("当前是打击群投入阶段：请拖入打击群（飞机等），不是兵团。\n", Qt::black, true);
            return;
        }

        // 必须把打击群拖到战斗区（同一个 region）
        if (regionId != m_battle.battleRegionId) {
            emit logLine("打击群投入失败：请拖到当前战斗区。\n", Qt::black, true);
            return;
        }

        // 判定精锐绑定
        bool elite = pixPath.contains("JR", Qt::CaseInsensitive);
        bool bind = false;
        if (elite) {
            auto r = QMessageBox::question(nullptr, "精锐",
                                          "检测到精锐单位，是否绑定精锐？",
                                          QMessageBox::Yes|QMessageBox::No);
            bind = (r == QMessageBox::Yes);
        }

        StrikeGroupEntry e{pixPath, bind};

        if (m_strikeTurn == m_battle.attacker) {
            m_battle.strikeA.push_back(e);
            m_strikeAtkPassed = false; // 一旦投入，撤销 pass
        } else {
            m_battle.strikeD.push_back(e);
            m_strikeDefPassed = false;
        }

        emit logLine(QString("打击群投入：%1 -> region %2\n").arg(pixPath).arg(regionId), Qt::black, true);

        // 投入后切换回合
        m_strikeTurn = (m_strikeTurn == m_battle.attacker) ? m_battle.defender : m_battle.attacker;
        emit strikeGroupsChanged(m_battle.strikeA, m_battle.strikeD, m_strikeTurn, false);
        return;
    }

    // ===== 原逻辑：落子生成 PieceItem（普通/事件）=====
    placeNewPieceToRegion(regionId, pixPath, 20.0, eventId, isEvent);
}


void GameController::onActionTokenDropped(const QString& pixPath)
{
    if (!isActionTokenPath(pixPath)) return;

    // 行动阶段中不处理行动签
    if (m_phase.active) return;

    Side side = Side::Unknown;
    if (pixPath.startsWith(":/D/")) side = Side::D;
    else if (pixPath.startsWith(":/S/")) side = Side::S;

    if (side == Side::Unknown) return;

    // ✅ 只允许轮到的那方打签
    if (side != m_nextActionTokenSide) {
        emit logLine("当前不轮到该方打出行动签。\n", Qt::black, true);
        return;
    }

    int ap = 0;
    if (pixPath.contains("XDQ2")) ap = 2;
    else if (pixPath.contains("XDQ6")) ap = 6;
    if (ap == 0) return;

    // 打出行动签 -> 获得AP -> addAP 内会 startActionPhase(side)
    addAP(side, ap);

    emit logLine(QString("行动签：%1 AP +%2\n").arg(side==Side::D ? "德国" : "苏联").arg(ap),
                 Qt::black, true);

    // ✅ 打完这张签后，下一张签轮到对方（等本行动阶段结束后再打）
    m_nextActionTokenSide = (side==Side::D ? Side::S : Side::D);
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

    // 如果这是兵团/要塞，且该 region 没写过 owner，则用该单位阵营初始化
    if (item->kind() == PieceKind::Corps || item->kind() == PieceKind::Fortress) {
        if (!m_owner.contains(regionId) || m_owner.value(regionId) == Side::Unknown) {
            if (item->side() != Side::Unknown)
                m_owner[regionId] = item->side();
        }
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

    for (auto* gi : m_scene->items()) {
        if (gi->type() != PieceType) continue;

        auto* p = static_cast<PieceItem*>(gi);

        bool movable =
            phaseActive() &&
            ( (m_phase.seg == ActionSeg::Move) || (m_phase.seg == ActionSeg::Battle) ) &&
            (side != Side::Unknown) &&
            (p->kind() == PieceKind::Corps) &&
            (p->side() == side);

        if (movable && p->movedThisActionPhase())
            movable = false;

        p->setFlag(QGraphicsItem::ItemIsMovable, movable);
        p->setFlag(QGraphicsItem::ItemIsSelectable, true);
    }
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

int GameController::battleDeclareCost(PieceItem* p, int distance) const
{
    int d = (distance <= 0 ? 1 : distance);
    if (isLargeCorps(p)) return 2 * d;  // 大兵团 2N
    return d;                            // 小兵团 N
}

void GameController::onPieceMovedRegionToRegion(PieceItem *piece, int fromRegionId, int toRegionId, Side side)
{
    if (!piece) return;
    if (!phaseActive()) return;
    if (side != phaseSide()) return;
    if (!m_placementManager) return;

    // ====== 1) 陆战环节：拖进敌占区 => 发起/加入战斗 ======
    if (inBattleSeg()) {
        Side owner = m_owner.value(toRegionId, Side::Unknown);
        if (owner != Side::Unknown && owner != side) {
            // 1) 距离 N（宣战不走“blocked”限制的话就给空集合）
            QSet<int> blocked;
            int dist = m_graph.shortestDistance(fromRegionId, toRegionId, blocked);
            if (dist < 0) {
                rollbackToRegion(piece, fromRegionId, "宣战失败：目标敌占区不可达，已回滚。\n");
                return;
            }

            // 2) 计算宣战成本
            const int cost = battleDeclareCost(piece, dist);
            const int apNow = currentAP(side);

            if (apNow < cost) {
                rollbackToRegion(piece, fromRegionId,
                                 QString("宣战失败：距离=%1 需AP=%2 当前AP=%3，不足，已回滚。\n")
                                 .arg(dist).arg(cost).arg(apNow));
                return;
            }

            // 3) 扣 AP
            addAP(side, -cost);
            emit logLine(QString("宣战：%1 -> region %2 距离=%3 扣AP=%4（不发生实际移动）\n")
                         .arg(fromRegionId).arg(toRegionId).arg(dist).arg(cost),
                         Qt::black, true);

            // 4) ✅ 不发生实际移动：回滚回原区
            rollbackToRegion(piece, fromRegionId, QString());

            // 5) 开战（战斗发生在 toRegionId）
            tryStartBattleFromMove(piece, fromRegionId, toRegionId);
            return;
        }

        // 若不是敌占区：默认不允许随便挪（防误拖）
        rollbackToRegion(piece, fromRegionId, "陆战环节：只能把兵团拖入敌占区以发起战斗，已回滚。\n");
        return;
    }

    // ====== 2) 陆上移动环节：走你原有逻辑 ======
    if (!inMoveSeg()) return;

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

    emit logLine(QString("陆上移动：%1 -> %2 距离=%3 扣AP=%4\n").arg(fromRegionId).arg(toRegionId).arg(dist).arg(cost), Qt::black, true);

    addAP(side, -cost);

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
    refreshMovablePieces();

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
        refreshMovablePieces();

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

    clearAP(finishedSide);

    Side nextSide = Side::Unknown;
    if (finishedSide == Side::D) nextSide = Side::S;
    else if (finishedSide == Side::S) nextSide = Side::D;

    m_nextActionTokenSide = nextSide;

    m_phase.active = false;
    m_phase.activeSide = Side::Unknown;
    m_phase.segIndex = 0;
    m_phase.seg = ActionSeg::Move;

    refreshMovablePieces();

    emit requestNavStep(0);
    emit requestEndSegEnabled(false);
    emit actionPhaseChanged(false, Side::Unknown, 0, ActionSeg::Move);

    emit logLine(QString("行动阶段结束：下一张行动签由【%1】打出。\n")
                 .arg(nextSide==Side::D ? "德国" : "苏联"),
                 Qt::black, true);
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

void GameController::setFirstPlayer(Side side)
{
    if (side!=Side::D && side!=Side::S) return;

    // 若当前还在行动阶段，可选择强制结束（按你需要）
    if (m_phase.active) {
        endActionPhase();
    }

    // 先手只是决定谁打第一张行动签，不给AP
    clearAP(Side::D);
    clearAP(Side::S);

    m_nextActionTokenSide = side;

    // UI：不在行动阶段
    emit requestNavStep(0);
    emit requestEndSegEnabled(false);

    emit logLine(QString("先手确定：%1先打出行动签。\n").arg(side==Side::D?"德国":"苏联"),
                 Qt::black, true);
}

bool GameController::canDragActionToken(Side side) const
{
    if (side!=Side::D && side!=Side::S) return false;

    // 行动阶段中不允许再打行动签
    if (m_phase.active) return false;

    // 只有轮到的那方可以拖行动签
    return side == m_nextActionTokenSide;
}

// 收集 pixPath
QStringList GameController::unitPixList(const QList<PieceItem*>& v) const {
    QStringList out;
    for (auto* p : v) if (p) out << p->pixPath();
    return out;
}

void GameController::syncBattleUnitsToDialog() {
    emit battleUnitsChanged(unitPixList(m_battle.attackers), unitPixList(m_battle.defenders));
}

void GameController::tryStartBattleFromMove(PieceItem* piece, int from, int to)
{
    // 防守方=该格现有兵团所属（取第一个敌方兵团）
    Side defSide = Side::Unknown;
    auto defPieces = m_placementManager->piecesInRegion(to);
    for (auto* p : defPieces) {
        if (p && p->kind()==PieceKind::Corps && p->side()!=piece->side()) { defSide = p->side(); break; }
    }
    if (defSide == Side::Unknown) {
        rollbackToRegion(piece, from, "该敌占区没有可识别的守军，已回滚。\n");
        return;
    }

    // 如果当前没有战斗：创建战斗上下文
    if (!m_battle.active) {
        m_battle.active = true;
        m_battle.battleRegionId = to;
        m_battle.attacker = piece->side();
        m_battle.defender = defSide;

        // defenders = 该格所有 defender side 的兵团
        m_battle.defenders.clear();
        for (auto* p : defPieces) {
            if (p && p->kind()==PieceKind::Corps && p->side()==defSide)
                m_battle.defenders.push_back(p);
        }

        m_battle.attackers.clear();
        m_battle.strikeA.clear();
        m_battle.strikeD.clear();

        // strike turn：默认攻方先
        m_strikeTurn = m_battle.attacker;
        m_strikeAtkPassed = false;
        m_strikeDefPassed = false;

        emit logLine(QString("宣战：region %1 开始陆战。\n").arg(to), Qt::black, true);

        // 把当前 piece 作为第一支攻方投入
        m_battle.attackers.push_back(piece);

        // 打开 BattleSetupDialog
        openBattleSetupDialog();
        syncBattleUnitsToDialog();
        return;
    }

    // 如果已有战斗：只能加入同一 battleRegion 且同一 attacker
    if (to != m_battle.battleRegionId || piece->side() != m_battle.attacker) {
        rollbackToRegion(piece, from, "已有战斗进行中：只能继续把攻方兵团投入同一战斗区，已回滚。\n");
        return;
    }

    // 避免重复加入
    if (!m_battle.attackers.contains(piece))
        m_battle.attackers.push_back(piece);

    syncBattleUnitsToDialog();
}

void GameController::openBattleSetupDialog()
{
    m_battleStep = BattleStep::SetupUnits;

    // 用 parentWidget：从 scene/view 取一个窗口即可，这里简单用 nullptr 也行
    auto* dlg = new BattleSetupDialog(nullptr);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setSides(m_battle.attacker, m_battle.defender);
    dlg->setRegionId(m_battle.battleRegionId);

    connect(this, &GameController::battleUnitsChanged, dlg, &BattleSetupDialog::refreshUnits);

    connect(dlg, &QDialog::finished, this, [this](int){
        // 结束兵团投入 -> 进入打击群投入
        openStrikeDialog();
    });

    dlg->show();

    m_battleStep = BattleStep::SetupStrike;
}

void GameController::openStrikeDialog()
{
    m_battleStep = BattleStep::SetupUnits;

    auto* dlg = new StrikeGroupDialog(nullptr);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setSides(m_battle.attacker, m_battle.defender);

    connect(this, &GameController::strikeGroupsChanged,
            dlg, &StrikeGroupDialog::refreshStrikeGroups);

    connect(dlg, &StrikeGroupDialog::passClicked, this, &GameController::onStrikePass);

    // 初次刷新
    emit strikeGroupsChanged(m_battle.strikeA, m_battle.strikeD, m_strikeTurn, false);

    connect(dlg, &QDialog::finished, this, [this](int){
        // 打击群结束 -> 战斗区展示
        openBattlefieldDialog();

        // 清战斗上下文（你也可以留着用于结算）
        m_battle.active = false;
    });

    dlg->show();

    m_battleStep = BattleStep::SetupStrike;
}

void GameController::openBattlefieldDialog()
{
    auto* dlg = new BattleFieldDialog(nullptr);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    dlg->setContent(unitPixList(m_battle.attackers),
                    unitPixList(m_battle.defenders),
                    m_battle.strikeA,
                    m_battle.strikeD);

    dlg->show();
}

void GameController::onStrikePass()
{
    if (!m_battle.active) return;

    if (m_strikeTurn == m_battle.attacker) m_strikeAtkPassed = true;
    else if (m_strikeTurn == m_battle.defender) m_strikeDefPassed = true;

    // 如果双方都 pass：结束（让 StrikeGroupDialog accept）
    if (m_strikeAtkPassed && m_strikeDefPassed) {
        emit strikeGroupsChanged(m_battle.strikeA, m_battle.strikeD, m_strikeTurn, true);
        // 这里不直接 accept，因为 dialog 在外部；
        // 简单方式：发 finished 标志后，让 dialog 自己 close：
        // 你也可以在 StrikeGroupDialog 收到 finished=true 时自动 accept()
        return;
    }

    // 切换回合
    m_strikeTurn = (m_strikeTurn == m_battle.attacker) ? m_battle.defender : m_battle.attacker;
    emit strikeGroupsChanged(m_battle.strikeA, m_battle.strikeD, m_strikeTurn, false);
}

bool GameController::canDragUnitInBattleSeg(Side side) const
{
    if (!phaseActive()) return false;
    if (!inBattleSeg()) return false;
    if (side != phaseSide()) return false; // 只允许当前行动方
    return true;
}
