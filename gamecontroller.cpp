#include "gamecontroller.h"
#include <QGraphicsScene>
#include <QPixmap>
#include <QMessageBox>
#include "placementmanager.h"
#include "util.h"
#include "battlesetupdialog.h"
#include "strikegroupdialog.h"
#include "battlefielddialog.h"

GameController::GameController(QGraphicsScene* scene, QObject* parent)
    : QObject(parent)
    , m_scene(scene)
    , m_placementManager(new PlacementManager(this))
{
    // ======= 城市格子 =======
    auto addRegion = [&](int id, const QRectF& rc, Nation occ) {
        auto* r = new RegionItem(id, rc);
        r->occupy = occ;
        m_scene->addItem(r);
        m_placementManager->addRegionItem(r);
        return r;
    };

    // 挪威北部 0 D
    addRegion(0, QRectF(972.46, 1166.95, 198.72, 209.29), Nation::Germany);

    // 芬兰北部 1 D
    addRegion(1, QRectF(1310.70, 1169.06, 198.72, 211.40), Nation::Germany);

    // 摩尔曼斯克 2 S
    addRegion(2, QRectF(1593.98, 1179.63, 200.83, 211.40), Nation::Soviet);

    // 卡累利阿 3 S
    addRegion(3, QRectF(1448.11, 1443.89, 198.72, 202.95), Nation::Soviet);

    // 阿尔汉格尔斯克 4 S
    addRegion(4, QRectF(1995.65, 1422.75, 202.95, 205.06), Nation::Soviet);

    // 基洛夫 5 S
    addRegion(5, QRectF(2382.52, 1585.53, 205.06, 207.18), Nation::Soviet);

    // 列宁格勒 6 S
    addRegion(6, QRectF(1308.59, 1722.94, 198.72, 202.95), Nation::Soviet);

    // 季赫温 7 S
    addRegion(7, QRectF(1581.30, 1687.00, 198.72, 205.06), Nation::Soviet);

    // 雅罗斯拉夫尔 8 S
    addRegion(8, QRectF(1851.90, 1714.48, 200.83, 205.06), Nation::Soviet);

    // 高尔基 9 S
    addRegion(9, QRectF(2097.12, 1866.69, 205.06, 200.83), Nation::Soviet);

    // 喀山 10 S
    addRegion(10, QRectF(2367.72, 1862.47, 205.06, 198.72), Nation::Soviet);

    // 波罗的海国家 11 S
    addRegion(11, QRectF(1031.65, 1980.85, 200.83, 207.18), Nation::Soviet);

    // 斯摩棱斯克 12 S
    addRegion(12, QRectF(1369.90, 2016.79, 200.83, 207.18), Nation::Soviet);

    // 莫斯科 13 S
    addRegion(13, QRectF(1701.80, 1978.74, 196.61, 207.18), Nation::Soviet);

    // 东普鲁士 14 D
    addRegion(14, QRectF(731.46, 2130.95, 200.83, 200.83), Nation::Germany);

    // 波兰北部 15 D
    addRegion(15, QRectF(710.87, 2399.38, 201.59, 204.62), Nation::Germany);

    // 白俄罗斯 16 S
    addRegion(16, QRectF(1097.18, 2257.79, 196.61, 202.95), Nation::Soviet);

    // 布良斯克 17 S
    addRegion(17, QRectF(1498.85, 2270.47, 198.72, 202.95), Nation::Soviet);

    // 沃洛涅日 18 S
    addRegion(18, QRectF(1972.40, 2285.27, 202.95, 205.06), Nation::Soviet);

    // 萨拉托夫 19 S
    addRegion(19, QRectF(2352.92, 2390.97, 205.06, 205.06), Nation::Soviet);

    // 哈尔科夫 20 S
    addRegion(20, QRectF(1706.03, 2500.90, 198.72, 207.18), Nation::Soviet);

    // 波兰南部 21 D
    addRegion(21, QRectF(735.68, 2691.17, 200.83, 200.83), Nation::Germany);

    // 利沃夫 22 S
    addRegion(22, QRectF(1021.08, 2619.29, 200.83, 200.83), Nation::Soviet);

    // 基辅 23 S
    addRegion(23, QRectF(1302.25, 2564.33, 200.83, 202.95), Nation::Soviet);

    // 斯大林格勒 24 S
    addRegion(24, QRectF(2196.48, 2651.00, 209.29, 209.29), Nation::Soviet);

    // 第聂伯罗彼得罗夫斯克 25 S
    addRegion(25, QRectF(1593.98, 2765.16, 200.83, 202.95), Nation::Soviet);

    // 斯大林诺 26 S
    addRegion(26, QRectF(1881.49, 2784.19, 202.95, 207.18), Nation::Soviet);

    // 阿斯特拉罕 27 S
    addRegion(27, QRectF(2426.91, 2866.63, 205.06, 205.06), Nation::Soviet);

    // 敖德萨 28 S
    addRegion(28, QRectF(1342.41, 2898.34, 198.72, 205.06), Nation::Soviet);

    // 南斯拉夫 29 D
    addRegion(29, QRectF(703.97, 3023.07, 198.72, 202.95), Nation::Germany);

    // 罗马尼亚 30 D
    addRegion(30, QRectF(1107.75, 3063.24, 198.72, 207.18), Nation::Germany);

    // 克里米亚 31 S
    addRegion(31, QRectF(1613.01, 3088.61, 200.83, 205.06), Nation::Soviet);

    // 格罗兹尼 32 S
    addRegion(32, QRectF(2251.45, 3090.72, 207.18, 207.18), Nation::Soviet);

    // 巴统 33 S
    addRegion(33, QRectF(1961.83, 3232.36, 205.06, 211.40), Nation::Soviet);

    // 巴库 34 S
    addRegion(34, QRectF(2401.54, 3409.94, 205.06, 205.06), Nation::Soviet);


    createPieceToRegion(1,  ":/D/D_2JBT.png"); // 挪威北部
    createPieceToRegion(2,  ":/S/S_2JBT.png"); // 摩尔曼斯克
    createPieceToRegion(6,  ":/S/S_4JYS.png"); // 列宁格勒
    createPieceToRegion(11, ":/D/D_4JBT.png"); // 波罗的海国家
    createPieceToRegion(11, ":/D/D_4JBT.png");
    createPieceToRegion(14, ":/D/D_4JBT.png"); // 东普鲁士
    createPieceToRegion(16, ":/D/D_4JBT.png"); // 白俄罗斯
    createPieceToRegion(16, ":/D/D_4JBT.png");
    createPieceToRegion(16, ":/D/D_2JBT.png");
    createPieceToRegion(21, ":/D/D_2JBT.png"); // 波兰南部
    createPieceToRegion(22, ":/D/D_4JBT.png"); // 利沃夫
    createPieceToRegion(22, ":/D/D_4JBT.png");
    createPieceToRegion(23, ":/S/S_4JBT.png"); // 基辅
    createPieceToRegion(28, ":/S/S_4JBT.png"); // 敖德萨
    createPieceToRegion(30, ":/L/L_4JBT.png"); // 罗马尼亚
    createPieceToRegion(30, ":/L/L_3JBT.png");
    createPieceToRegion(30, ":/D/D_4JBT.png");
    createPieceToRegion(31, ":/S/S_4JYS.png"); // 克里米亚
    createPieceToRegion(34, ":/S/S_3JBT.png"); // 巴库

    refreshMovablePieces();
}

PieceItem* GameController::createPieceToRegion(int regionId, const QString& pixPath)
{
    // 行动签不生成棋子
    if (pixPath.contains("_XDQ", Qt::CaseSensitive)) return nullptr;

    // if (!m_eventAllowedRegions.isEmpty()) {
    //     if (!m_eventAllowedRegions.contains(regionId)) {
    //         emit logLine(QString("事件落子失败：region %1 不在允许列表").arg(regionId), Qt::black, true);
    //         return nullptr;
    //     }
    // }

    PieceItem* item = new PieceItem(QPixmap(pixPath));

    Side side;
    int lvl;
    if (parseCorpsFromPixPath(pixPath, side, lvl)) {
        item->setUnitMeta(PieceKind::Corps, side, lvl, pixPath);
    } else if (parseFortressFromPixPath(pixPath, side, lvl)) {
        item->setUnitMeta(PieceKind::Fortress, side, lvl, pixPath);
    } else if (parseFortificationFromPixPath(pixPath, side, lvl)) {
        item->setUnitMeta(PieceKind::Fortification, side, lvl, pixPath);
    }
    m_scene->addItem(item);

    connect(item, &PieceItem::splitRequested, this, &GameController::splitPieceToRegion);
    connect(item, &PieceItem::dropReleased, this, &GameController::movePieceToRegion);

    m_placementManager->movePieceToRegion(item, regionId);
    item->setRegionId(regionId);

    item->markLastValid(regionId);

    if ((item->kind() == PieceKind::Corps || item->kind() == PieceKind::Fortress))
    {
        if (!m_owner.contains(regionId)) {
            m_owner[regionId] = item->side();
        }
    }

    return item;
}

void GameController::refreshMovablePieces()
{
    const Side side = m_actionStep.activeSide;

    for (auto* gi : m_scene->items()) {
        if (gi->type() != PieceType) continue;

        auto* p = static_cast<PieceItem*>(gi);

        bool movable =
            m_actionStep.active &&
            ( (m_actionStep.segment == ActionSegment::Move) || (m_actionStep.segment == ActionSegment::Battle) ) &&
            (p->kind() == PieceKind::Corps) &&
            (p->side() == side);

        if (movable && p->movedThisActionStep())
            movable = false;

        p->setFlag(QGraphicsItem::ItemIsMovable, movable);
        p->setFlag(QGraphicsItem::ItemIsSelectable, true);
    }
}

void GameController::setTurn(int turn)
{
    m_gameState.turn = turn;
    // 限制 1..8
    if (m_gameState.turn < 1) m_gameState.turn = 1;
    if (m_gameState.turn > 8) m_gameState.turn = 8;

    emit stateChanged();
}

void GameController::addNationalPower(Side side, int delta)
{
    if (side!=Side::D && side!=Side::S) return;
    m_gameState.npRef(side) += delta;

    emit stateChanged();
}

void GameController::addOil(int delta)
{
    m_gameState.oilRef() += delta;

    emit stateChanged();
}

void GameController::addReadyPoints(Side side, int delta)
{
    if (side!=Side::D && side!=Side::S) return;
    m_gameState.rpRef(side) += delta;

    emit stateChanged();
}

void GameController::addAP(Side side, int delta)
{
    if (side!=Side::D && side!=Side::S) return;

    m_gameState.apRef(side) += delta;

    emit stateChanged();

    // 行动步骤：delta>0 且未active
    if (delta > 0 && !m_actionStep.active) {
        startActionStep(side);
    } else {
        refreshMovablePieces();
    }

    const QString who = (side == Side::D ? "德国" : "苏联");
    emit logLine(QString("%1行动点 %2%3，当前：D=%4 S=%5\n")
                 .arg(who)
                 .arg(delta >= 0 ? "+" : "")
                 .arg(delta)
                 .arg(m_gameState.apD)
                 .arg(m_gameState.apS),
                 Qt::black, true);
}

void GameController::clearAP(Side side)
{
    if (side!=Side::D && side!=Side::S) return;
    m_gameState.apRef(side) = 0;

    emit stateChanged();

    refreshMovablePieces();
}

int GameController::currentAP(Side side) const
{
    return (side==Side::D) ? m_gameState.apD : m_gameState.apS;
}

void GameController::startActionStep(Side side)
{
    if (m_actionStep.active) return;// 如果已经在行动步骤：一般不允许再次 start

    m_actionStep.active = true;
    m_actionStep.activeSide = side;
    m_actionStep.segmentIndex = 0;
    m_actionStep.segment = ActionSegment::Move;

    // 进入行动步骤：重置“本阶段锁定继续移动”标记
    for (auto* gi : m_scene->items()) {
        if (gi->type() != PieceType) continue;
        PieceItem* p = static_cast<PieceItem*>(gi);
        p->setMovedThisActionStep(false);
    }

    // 同步移动规则开关 + 刷新可拖拽
    refreshMovablePieces();

    emit setCurrentSegment(1);

    // 日志（走你已有信号）
    emit logLine(QString("%1打出行动签：进入行动步骤（从【陆上移动】开始）\n").arg(side==Side::D ? "德国" : "苏联"), Qt::black, true);
}

void GameController::endActionStep()
{
    if (!m_actionStep.active) return;

    const Side finishedSide = m_actionStep.activeSide;

    clearAP(finishedSide);

    Side nextSide;
    if (finishedSide == Side::D) nextSide = Side::S;
    else if (finishedSide == Side::S) nextSide = Side::D;

    m_nextActionTokenSide = nextSide;

    m_actionStep.active = false;
    m_actionStep.segmentIndex = 0;
    m_actionStep.segment = ActionSegment::Move;

    refreshMovablePieces();

    emit setCurrentSegment(0);

    emit logLine(QString("行动步骤结束：下一张行动签由【%1】打出。\n")
                 .arg(nextSide==Side::D ? "德国" : "苏联"),
                 Qt::black, true);
}

void GameController::goNextSegment()
{
    if (!m_actionStep.active) return;

    if (m_actionStep.segmentIndex < 4) {
        m_actionStep.segmentIndex++;
        m_actionStep.segment = ActionSegment(m_actionStep.segmentIndex);

        // 切换环节：同步移动规则（只有 Move 环节允许移动）
        refreshMovablePieces();

        // UI 更新：NavProgress 用 1..5
        emit setCurrentSegment(m_actionStep.segmentIndex + 1);

        // 日志提示
        switch (m_actionStep.segmentIndex) {
        case 1: emit logLine("陆上移动结束，进入陆战环节。\n", Qt::black, true); break;
        case 2: emit logLine("陆战结束，进入调动环节。\n", Qt::black, true); break;
        case 3: emit logLine("调动结束，进入准备环节。\n", Qt::black, true); break;
        case 4: emit logLine("准备结束，进入补给环节。\n", Qt::black, true); break;
        default: break;
        }
        return;
    }

    // segmentIndex==4: 结束补给 -> 行动步骤结束
    emit logLine("补给结束：本行动步骤结束。\n", Qt::black, true);
    endActionStep();
}

void GameController::setFirstPlayerD()
{
    // 若当前还在行动步骤，可选择强制结束
    if (m_actionStep.active) {
        endActionStep();
    }

    // 先手只是决定谁打第一张行动签，不给AP
    clearAP(Side::D);
    clearAP(Side::S);

    m_nextActionTokenSide = Side::D;

    emit setCurrentSegment(0);
    emit logLine(QString("先手确定：德国先打出行动签。\n"), Qt::black, true);
}

void GameController::setFirstPlayerS()
{
    if (m_actionStep.active) {
        endActionStep();
    }

    clearAP(Side::D);
    clearAP(Side::S);

    m_nextActionTokenSide = Side::S;

    emit setCurrentSegment(0);
    emit logLine(QString("先手确定：苏联先打出行动签。\n"), Qt::black, true);
}

void GameController::dropPieceToScene(QPointF scenePos, QString pixPath)
{
    // 行动签
    if (pixPath.contains("_XDQ", Qt::CaseSensitive)) {
        if (m_scene->sceneRect().contains(scenePos)) {
            if (!(pixPath.contains("_XDQ", Qt::CaseSensitive))) return;

            // 行动步骤中不处理行动签
            if (m_actionStep.active) return;

            Side side;
            if (pixPath.startsWith(":/D/")) side = Side::D;
            else if (pixPath.startsWith(":/S/")) side = Side::S;

            // ✅ 只允许轮到的那方打签
            if (side != m_nextActionTokenSide) {
                emit logLine("当前不轮到该方打出行动签。\n", Qt::black, true);
                return;
            }

            int ap = 0;
            if (pixPath.contains("XDQ2")) ap = 2;
            else if (pixPath.contains("XDQ6")) ap = 6;
            if (ap == 0) return;

            addAP(side, ap);

            emit logLine(QString("行动签：%1 AP +%2\n").arg(side==Side::D ? "德国" : "苏联").arg(ap),
                         Qt::black, true);

            // ✅ 打完这张签后，下一张签轮到对方（等本行动步骤结束后再打）
            m_nextActionTokenSide = (side==Side::D ? Side::S : Side::D);
        } else {
            emit logLine("行动签必须丢在地图范围内。\n", Qt::black, true);
        }
        return;
    }

    int regionId = -1;
    QList<QGraphicsItem *> items = m_scene->items(scenePos, Qt::IntersectsItemShape, Qt::DescendingOrder);
    for (QGraphicsItem* it : items) {
        if (it->type() == RegionType) {
            regionId = static_cast<RegionItem*>(it)->id();
            break;
        }
    }
    if (regionId < 0) return;

    createPieceToRegion(regionId, pixPath);
}

void GameController::splitPieceToRegion(PieceItem* piece, int a, int b)
{
    if (!piece) return;

    const int regionId = piece->regionId();
    if (regionId < 0) return;

    const Side side = piece->side();

    // 1) 从 PlacementManager 移除（会重排原城市）
    m_placementManager->removePieceItem(piece);

    // 2) 从 scene 删除旧棋子
    m_scene->removeItem(piece);
    piece->deleteLater();

    // 3) 生成两个新兵团并放回同城
    auto spawnOne = [&](int lvl){
        const QString pixPath = corpsPixPath(side, lvl);
        createPieceToRegion(regionId, pixPath);
    };

    spawnOne(a);
    spawnOne(b);

    emit logLine(QString("拆分：region %1  %2 -> %3 + %4")
                 .arg(regionId)
                 .arg(piece->level())
                 .arg(a)
                 .arg(b), Qt::black, true);
}

void GameController::rollbackPieceToRegion(PieceItem* piece, int regionId, const QString& reason)
{
    if (!piece) return;
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

void GameController::movePieceToRegion(PieceItem* piece, const QPointF& sceneCenter)
{
    if (!piece) return;

    int toRegionId = -1;
    QList<QGraphicsItem *> items = m_scene->items(sceneCenter, Qt::IntersectsItemShape, Qt::DescendingOrder);
    for (QGraphicsItem* it : items) {
        if (it->type() == RegionType) {
            toRegionId = static_cast<RegionItem*>(it)->id();
        }
    }

    if (toRegionId < 0) {
        const int last = piece->lastValidRegionId();
        if (last < 0) return;

        piece->setInLayout(true);
        piece->setPos(piece->lastValidPos());
        piece->setInLayout(false);

        m_placementManager->relayoutRegion(last);
        return;
    }

    if (toRegionId >= 0) {
        m_placementManager->movePieceToRegion(piece, toRegionId);
        piece->markLastValid(toRegionId);
    }

    const int fromRegionId = piece->regionId();
    if (fromRegionId < 0 || fromRegionId == toRegionId) {
        return;
    }

    // 触发你的规则
    if (!m_actionStep.active) return;
    if (piece->side() != m_actionStep.activeSide) return;

    // ====== 1) 陆战环节：拖进敌占区 => 发起/加入战斗 ======
    if (m_actionStep.active && (m_actionStep.segment == ActionSegment::Battle)) {
        Side owner = m_owner.value(toRegionId);
        if (owner != piece->side()) {
            // 1) 距离 N（宣战不走“blocked”限制的话就给空集合）
            QSet<int> blocked;
            int distance = m_graph.shortestDistance(fromRegionId, toRegionId, blocked);
            if (distance < 0) {
                rollbackPieceToRegion(piece, fromRegionId, "宣战失败：目标敌占区不可达，已回滚。\n");
                return;
            }

            // 2) 计算宣战成本
            int cost = (distance <= 0 ? 1 : distance);
            if (piece->level() >= 3) {
                cost *= 2;
            }

            const int apNow = currentAP(piece->side());

            if (apNow < cost) {
                rollbackPieceToRegion(piece, fromRegionId,
                                 QString("宣战失败：距离=%1 需AP=%2 当前AP=%3，不足，已回滚。\n")
                                 .arg(distance).arg(cost).arg(apNow));
                return;
            }

            // 3) 扣 AP
            addAP(piece->side(), -cost);
            emit logLine(QString("宣战：%1 -> region %2 距离=%3 扣AP=%4（不发生实际移动）\n")
                         .arg(fromRegionId).arg(toRegionId).arg(distance).arg(cost),
                         Qt::black, true);

            // 4) ✅ 不发生实际移动：回滚回原区
            rollbackPieceToRegion(piece, fromRegionId, QString());

            // 5) 开战（战斗发生在 toRegionId）
            tryStartBattleFromMove(piece, fromRegionId, toRegionId);
            return;
        }

        // 若不是敌占区：默认不允许随便挪（防误拖）
        rollbackPieceToRegion(piece, fromRegionId, "陆战环节：只能把兵团拖入敌占区以发起战斗，已回滚。\n");
        return;
    }

    // ====== 2) 陆上移动环节：走你原有逻辑 ======
    if (!(m_actionStep.active && (m_actionStep.segment == ActionSegment::Move))) return;

    // 已因“进入敌占格”被锁住，则回滚
    if (piece->movedThisActionStep()) {
        rollbackPieceToRegion(piece, fromRegionId, "本行动步骤该兵团已进入敌占格，不能继续移动，已回滚。\n");
        return;
    }

    // 最短路距离（中途不能经过 blocked）
    QSet<int> blocked;
    for (int rid = 0; rid < 35; ++rid) {
        const auto pieces = m_placementManager->piecesInRegion(rid);
        for (auto* p : pieces) {
            if (!p) continue;
            if (p->kind() == PieceKind::Corps && p->side() != piece->side()) {
                blocked.insert(rid);
                break;
            }
            if (p->kind() == PieceKind::Fortress && p->side() != piece->side()) {
                blocked.insert(rid);
                break;
            }
        }
    }

    blocked.remove(fromRegionId); // 起点不算阻挡
    int distance = m_graph.shortestDistance(fromRegionId, toRegionId, blocked);
    if (distance < 0) {
        rollbackPieceToRegion(piece, fromRegionId, "移动失败：无可达陆上路线（可能被敌方兵团/要塞阻挡），已回滚。\n");
        return;
    }

    int cost;

    // 10.2：距离0按距离1算
    int d = (distance <= 0 ? 1 : distance);

    if (piece->level() >= 3) {
        // 大兵团：每距离1消耗1AP
        cost = d;
    } else {
        // 小兵团：每距离2消耗1AP，距离1也要1AP
        // 等价：ceil(d / 2.0)
        cost = (d + 1) / 2;
    }

    const int apNow = currentAP(piece->side());

    if (apNow < cost) {
        rollbackPieceToRegion(piece, fromRegionId, QString("移动失败：距离=%1 需AP=%2 当前AP=%3，不足，已回滚。\n").arg(distance).arg(cost).arg(apNow));
        return;
    }

    emit logLine(QString("陆上移动：%1 -> %2 距离=%3 扣AP=%4\n").arg(fromRegionId).arg(toRegionId).arg(distance).arg(cost), Qt::black, true);

    addAP(piece->side(), -cost);

    // 占领判定：如果进入敌占格，则占领并锁住本阶段继续移动
    Side owner = m_owner.value(toRegionId);
    if (owner != piece->side()) {
        m_owner[toRegionId] = piece->side();
        piece->setMovedThisActionStep(true);
        emit logLine("进入敌占格：占领完成，本行动步骤该兵团不能继续移动。\n", Qt::black, true);
    }

    // 这次移动成功，更新 lastValid
    piece->markLastValid(toRegionId);

    // 如果规则允许，这里 lastValid 需要以规则结果为准：
    // - 若 movePieceToRegion 内回滚到 fromId，会 markLastValid(fromId)
    // - 若允许停在 toId，你已经 markLastValid(toId) 了
}

bool GameController::canDragActionToken(Side side) const
{
    if (side!=Side::D && side!=Side::S) return false;

    // 行动步骤中不允许再打行动签
    if (m_actionStep.active) return false;

    // 只有轮到的那方可以拖行动签
    return side == m_nextActionTokenSide;
}

bool GameController::canDragPieceInMoveSegment(Side side) const
{
    if (!m_actionStep.active) return false;
    if (!(m_actionStep.active && (m_actionStep.segment == ActionSegment::Move))) return false;
    if (side != m_actionStep.activeSide) return false;
    if (currentAP(m_actionStep.activeSide) <= 0) return false;
    return true;
}

bool GameController::canDragPieceInBattleSegment(Side side) const
{
    if (!m_actionStep.active) return false;
    if (!m_actionStep.active || m_actionStep.segment != ActionSegment::Battle) return false;
    if (side != m_actionStep.activeSide) return false; // 只允许当前行动方
    return true;
}



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
    Side defSide;
    auto defPieces = m_placementManager->piecesInRegion(to);
    for (auto* p : defPieces) {
        if (p && p->kind()==PieceKind::Corps && p->side()!=piece->side()) { defSide = p->side(); break; }
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
        rollbackPieceToRegion(piece, from, "已有战斗进行中：只能继续把攻方兵团投入同一战斗区，已回滚。\n");
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

    connect(this, &GameController::strikeGroupsChanged, dlg, &StrikeGroupDialog::refreshStrikeGroups);

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
