#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsPixmapItem>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QPixmap>
#include <QActionGroup>
#include <QTextEdit>
#include <QTimer>
#include <QRandomGenerator>
#include <QGridLayout>
#include <QLabel>
#include "regionitem.h"
#include "pieceitem.h"
#include "graphicsframe.h"
#include "eventdialog.h"
#include "pieceentrywidget.h"
#include "battledialog.h"

// ======= 小工具：城市格子矩形表 =======
static QList<QRectF> buildRegionRects()
{
    return {
        QRectF(972.46, 1166.95, 198.72, 209.29),  //挪威北部 0 D
        QRectF(1310.70, 1169.06, 198.72, 211.40), //芬兰北部 1 D
        QRectF(1593.98, 1179.63, 200.83, 211.40), //摩尔曼斯克 2 S
        QRectF(1448.11, 1443.89, 198.72, 202.95), //卡累利阿 3 S
        QRectF(1995.65, 1422.75, 202.95, 205.06), //阿尔汉格尔斯克 4 S
        QRectF(2382.52, 1585.53, 205.06, 207.18), //基洛夫 5 S
        QRectF(1308.59, 1722.94, 198.72, 202.95), //列宁格勒 6 S
        QRectF(1581.30, 1687.00, 198.72, 205.06), //季赫温 7 S
        QRectF(1851.90, 1714.48, 200.83, 205.06), //雅罗斯拉夫尔 8 S
        QRectF(2097.12, 1866.69, 205.06, 200.83), //高尔基 9 S
        QRectF(2367.72, 1862.47, 205.06, 198.72), //喀山 10 S
        QRectF(1031.65, 1980.85, 200.83, 207.18), //波罗的海国家 11 S
        QRectF(1369.90, 2016.79, 200.83, 207.18), //斯摩棱斯克 12 S
        QRectF(1701.80, 1978.74, 196.61, 207.18), //莫斯科 13 S
        QRectF(731.46, 2130.95, 200.83, 200.83),  //东普鲁士 14 D
        QRectF(710.87, 2399.38, 201.59, 204.62),  //波兰北部 15 D
        QRectF(1097.18, 2257.79, 196.61, 202.95), //白俄罗斯 16 S
        QRectF(1498.85, 2270.47, 198.72, 202.95), //布良斯克 17 S
        QRectF(1972.40, 2285.27, 202.95, 205.06), //沃洛涅日 18 S
        QRectF(2352.92, 2390.97, 205.06, 205.06), //萨拉托夫 19 S
        QRectF(1706.03, 2500.90, 198.72, 207.18), //哈尔科夫 20 S
        QRectF(735.68, 2691.17, 200.83, 200.83),  //波兰南部 21 D
        QRectF(1021.08, 2619.29, 200.83, 200.83), //利沃夫 22 S
        QRectF(1302.25, 2564.33, 200.83, 202.95), //基辅 23 S
        QRectF(2196.48, 2651.00, 209.29, 209.29), //斯大林格勒 24 S
        QRectF(1593.98, 2765.16, 200.83, 202.95), //第聂伯罗彼得罗夫斯克 25 S
        QRectF(1881.49, 2784.19, 202.95, 207.18), //斯大林诺 26 S
        QRectF(2426.91, 2866.63, 205.06, 205.06), //阿斯特拉罕 27 S
        QRectF(1342.41, 2898.34, 198.72, 205.06), //敖德萨 28 S
        QRectF(703.97, 3023.07, 198.72, 202.95),  //南斯拉夫 29 D
        QRectF(1107.75, 3063.24, 198.72, 207.18), //罗马尼亚 30 D
        QRectF(1613.01, 3088.61, 200.83, 205.06), //克里米亚 31 S
        QRectF(2251.45, 3090.72, 207.18, 207.18), //格罗兹尼 32 S
        QRectF(1961.83, 3232.36, 205.06, 211.40), //巴统 33 S
        QRectF(2401.54, 3409.94, 205.06, 205.06), //巴库 34 S
    };
}

// =====================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->D->setCurrentIndex(0);
    ui->S->setCurrentIndex(0);

    initActions();
    initDockSystem();
    initCentralView();
    initScene();
    initControllers();
    initLogDock();
    initStatusDock();
    initRegionItems();
    initPieceLists();
    initGameBoardPieces();
    m_gameController->refreshMovablePieces();
    initEventActions();
    refreshStatusUI();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// =====================================================
// 初始化阶段
// =====================================================

void MainWindow::initActions()
{
    auto *actionGroup = new QActionGroup(this);
    actionGroup->addAction(ui->action1);
    actionGroup->addAction(ui->action2);
    actionGroup->addAction(ui->action3);
    actionGroup->addAction(ui->action4);
    actionGroup->addAction(ui->action5);
    actionGroup->addAction(ui->action6);

    auto* tbSeg = addToolBar("行动环节");
    tbSeg->setMovable(false);
    tbSeg->setIconSize(QSize(36,36));
    tbSeg->setFont(QFont(QString("MicrosoftYaHei UI"), 14));

    m_navProgress = new NavProgress(this);
    tbSeg->addWidget(m_navProgress);
    m_navProgress->setTopInfo(QStringList() << "陆上移动" << "陆战" << "调动" << "准备" << "补给");
    m_navProgress->setCurrentStep(0);
    m_navProgress->setCurrentBackground(QColor(24,189,155));

    actEndSeg = tbSeg->addAction("结束环节");
    actEndSeg->setIcon(QIcon(":/res/end.png"));
    actEndSeg->setEnabled(false);
}

void MainWindow::initDockSystem()
{
    auto *hostLayout = new QVBoxLayout(ui->dockWidget);
    hostLayout->setContentsMargins(0,0,0,0);
    hostLayout->setSpacing(0);

    m_dockManager = new ads::CDockManager(ui->dockWidget);
    hostLayout->addWidget(m_dockManager);

    m_centralDock = new ads::CDockWidget("场上");
    m_dockManager->setCentralWidget(m_centralDock);
}

void MainWindow::initCentralView()
{
    m_graphicsFrame = new GraphicsFrame;
    m_graphicsView = m_graphicsFrame->graphicsView();

    if (m_centralDock)
        m_centralDock->setWidget(m_graphicsFrame);
}

void MainWindow::initScene()
{
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 3164, 4032);

    auto *backItem = new QGraphicsPixmapItem(QPixmap::fromImage(QImage(":/res/back.png")));
    backItem->setPos(0, 0);
    backItem->setZValue(0);
    scene->addItem(backItem);

    m_graphicsView->setScene(scene);
}

void MainWindow::initControllers()
{
    m_placementManager = new PlacementManager(scene, this);
    m_graphicsView->setPlacementManager(m_placementManager);

    m_gameController = new GameController(scene, m_placementManager, this);

    connect(m_graphicsView, &GraphicsView::pieceDropped, m_gameController, &GameController::onPieceDropped);
    connect(m_graphicsView, &GraphicsView::actionTokenDropped, m_gameController, &GameController::onActionTokenDropped);

    connect(m_gameController, &GameController::logLine, this, &MainWindow::appendLog);

    connect(actEndSeg, &QAction::triggered, m_gameController, &GameController::advanceSegment);

    connect(m_gameController, &GameController::requestEndSegEnabled, this, [this](bool en){
        if (actEndSeg) actEndSeg->setEnabled(en);
    });

    connect(m_gameController, &GameController::requestNavStep, this, [this](int step){
        if (m_navProgress) m_navProgress->setCurrentStep(step);
    });

    connect(m_gameController, &GameController::stateChanged, this, [this](const GameState&){
        refreshStatusUI();
    });
}

void MainWindow::initLogDock()
{
    auto *logDock = new ads::CDockWidget("日志");
    logTextEdit = new QTextEdit;
    logDock->setWidget(logTextEdit);

    m_dockManager->addDockWidget(ads::BottomDockWidgetArea, logDock);
    m_dockManager->setSplitterSizes(m_centralDock->dockAreaWidget(), {800, 200});

    QAction *logAction = logDock->toggleViewAction();
    logAction->setIcon(QIcon(":/res/log.png"));
    logAction->setText("日志");
    ui->toolBar->addAction(logAction);

    logDock->toggleView(false);
}

// =====================================================
// 状态 Dock
// =====================================================

void MainWindow::initStatusDock()
{
    auto *statusDock = new ads::CDockWidget("状态");
    auto *w = new QWidget;
    statusDock->setWidget(w);

    auto *grid = new QGridLayout(w);
    grid->setContentsMargins(8, 8, 8, 8);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(6);

    m_turnLabel = new QLabel("-");
    m_npLabelD  = new QLabel("-");
    m_oilLabelD = new QLabel("-");
    m_apLabelD  = new QLabel("-");
    m_rpLabelD  = new QLabel("-");
    m_npLabelS  = new QLabel("-");
    m_oilLabelS = new QLabel("-");
    m_apLabelS  = new QLabel("-");
    m_rpLabelS  = new QLabel("-");

    // 左侧：回合
    auto *lbTurn = new QLabel("回合");
    lbTurn->setAlignment(Qt::AlignCenter);
    m_turnLabel->setAlignment(Qt::AlignCenter);

    grid->addWidget(lbTurn, 0, 0, 1, 1);
    grid->addWidget(m_turnLabel, 1, 0, 2, 1);

    // 表头
    grid->addWidget(new QLabel(""), 0, 1);
    grid->addWidget(new QLabel("国力"), 0, 2);
    grid->addWidget(new QLabel("石油"), 0, 3);
    grid->addWidget(new QLabel("行动点"), 0, 4);
    grid->addWidget(new QLabel("准备点"), 0, 5);

    // 德国行
    grid->addWidget(new QLabel("德国"), 1, 1);
    grid->addWidget(m_npLabelD, 1, 2);
    grid->addWidget(m_oilLabelD, 1, 3);
    grid->addWidget(m_apLabelD, 1, 4);
    grid->addWidget(m_rpLabelD, 1, 5);

    // 苏联行
    grid->addWidget(new QLabel("苏联"), 2, 1);
    grid->addWidget(m_npLabelS, 2, 2);
    grid->addWidget(m_oilLabelS, 2, 3);
    grid->addWidget(m_apLabelS, 2, 4);
    grid->addWidget(m_rpLabelS, 2, 5);

    m_dockManager->addDockWidget(ads::TopDockWidgetArea, statusDock);

    QAction *statusAction = statusDock->toggleViewAction();
    statusAction->setIcon(QIcon(":/res/status.png"));
    statusAction->setText("状态");
    ui->toolBar->addAction(statusAction);

    statusDock->toggleView(false);
}

void MainWindow::initRegionItems()
{
    const auto regionRects = buildRegionRects();

    for (int i = 0; i < regionRects.size(); ++i) {
        auto *regionItem = new RegionItem(i, regionRects[i]);
        scene->addItem(regionItem);
        m_placementManager->addRegionItem(regionItem);
    }
}

PieceListWidget* MainWindow::createPieceList(QWidget* host)
{
    auto *list = new PieceListWidget(host);
    list->setViewMode(QListView::IconMode);
    list->setIconSize(QSize(72,72));
    list->setResizeMode(QListView::Adjust);
    list->setMovement(QListView::Static);
    list->setSpacing(8);
    list->setDragEnabled(true);
    list->setSelectionMode(QAbstractItemView::SingleSelection);

    auto *layout = new QVBoxLayout(host);
    layout->setContentsMargins(4,4,4,4);
    layout->addWidget(list);

    return list;
}

void MainWindow::initPieceLists()
{
    pieceListWidget_D = createPieceList(ui->D_DMQ);
    pieceListWidget_S = createPieceList(ui->S_DMQ);

    pieceListWidget_D->setSide(Side::D);
    pieceListWidget_S->setSide(Side::S);

    pieceListWidget_D->setGameController(m_gameController);
    pieceListWidget_S->setGameController(m_gameController);

    using Def = std::tuple<const char*, const char*, int>;

    const Def D[] = {
        {"德国大行动签",":/D/D_XDQ6.png", 1},
        {"德国小行动签",":/D/D_XDQ2.png", 1},
        {"1级兵团",":/D/D_1JBT.png", 0},
        {"2级兵团",":/D/D_2JBT.png", 0},
        {"3级兵团",":/D/D_3JBT.png", 0},
        {"4级兵团",":/D/D_4JBT.png", 0},
        {"1级兵团",":/L/L_1JBT.png", 0},
        {"2级兵团",":/L/L_2JBT.png", 0},
        {"3级兵团",":/L/L_3JBT.png", 0},
        {"4级兵团",":/L/L_4JBT.png", 0},
        {"1级海军",":/D/D_1JHJ.png", 0},
        {"2级海军",":/D/D_2JHJ.png", 0},
        {"1级航空",":/D/D_1JHK.png", 0},
        {"2级航空",":/D/D_2JHK.png", 0},
        {"Bf-109",":/D/D_B002.png", 5},
        {"Fw-190A 百舌鸟",":/D/D_B103.png", 0},
        {"Bf-110 破坏者",":/D/D_C102.png", 1},
        {"Hs-129B-2",":/D/D_D4T.png", 0},
        {"Ju-87 斯图卡",":/D/D_D57.png", 0},
        {"He-111P",":/D/D_D1002.png", 0},
        {"He-111H6",":/D/D_D1502.png", 0},
        {"Do-17Z",":/D/D_D2102.png", 0},
        {"Do-217",":/D/D_D2304.png", 0},
        {"Ju-88A",":/D/D_D3402.png", 0},
        {"潜艇舰队",":/D/D_DJ2.png", 0},
        {"战列巡洋舰队+++",":/D/D_E131.png", 0},
        {"战列舰队+++",":/D/D_E141.png", 0},
        {"重巡舰队+++",":/D/D_F131.png", 0},
        {"PzVI 虎I",":/D/D_G6.png", 0},
        {"PzIIIJ 三号J",":/D/D_H3.png", 0},
        {"PzIVG 四号G",":/D/D_H4.png", 0},
        {"StuGIIIF 三突F",":/D/D_HI3T.png", 0},
        {"StuGIIIG 三突G",":/D/D_HI4T.png", 0},
        {"PzIII 三号",":/D/D_I3.png", 1},
        {"PzIV 四号",":/D/D_I4.png", 0},
        {"PzIIIN 三号N",":/D/D_I5.png", 0},
        {"Pz38(t)",":/D/D_J3.png", 0},
        {"装甲步兵41",":/D/D_J25.png", 0},
        {"精锐航空兵",":/D/D_JRHKB.png", 1},
        {"精锐潜艇兵",":/D/D_JRQTB.png", 0},
        {"精锐掷弹兵",":/D/D_JRZDB.png", 0},
        {"精锐装甲兵",":/D/D_JRZJB.png", 4},
    };

    const Def S[] = {
        {"苏联大行动签",":/S/S_XDQ6.png", 1},
        {"苏联小行动签",":/S/S_XDQ2.png", 1},
        {"1级兵团",":/S/S_1JBT.png", 0},
        {"2级兵团",":/S/S_2JBT.png", 0},
        {"3级兵团",":/S/S_3JBT.png", 0},
        {"4级兵团",":/S/S_4JBT.png", 0},
        {"1级海军",":/S/S_1JHJ.png", 0},
        {"2级海军",":/S/S_2JHJ.png", 0},
        {"1级航空",":/S/S_1JHK.png", 0},
        {"2级航空",":/S/S_2JHK.png", 0},
        {"雅克-1",":/S/S_B002.png", 1},
        {"米格-3",":/S/S_B002~.png", 4},
        {"雅克-9",":/S/S_B003.png", 0},
        {"伊-16",":/S/S_C101.png", 7},
        {"斯勃-2",":/S/S_D1.png", 7},
        {"伊尔-2",":/S/S_D2T.png", 1},
        {"伊尔-2M",":/S/S_D3T.png", 0},
        {"德勃-3",":/S/S_D1201.png", 1},
        {"伊尔-4",":/S/S_D1302.png", 1},
        {"佩-2",":/S/S_D2302.png", 1},
        {"战列舰队++",":/S/S_E121.png", 0},
        {"战列舰队++",":/S/S_E131.png", 0},
        {"KV-1",":/S/S_H3.png", 0},
        {"T-34/76",":/S/S_H4.png", 1},
        {"KV-2",":/S/S_I6.png", 0},
        {"BT-7",":/S/S_J3.png", 0},
        {"T-70",":/S/S_J3~.png", 0},
        {"精锐航空兵",":/S/S_JRHKB.png", 0},
        {"精锐装甲兵",":/S/S_JRZJB.png", 0},
    };

    auto addAll = [this](PieceListWidget* list, const Def* arr, int n){
        for (int i = 0; i < n; ++i) {
            const auto& [name, path, cnt] = arr[i];
            addPiece(list, QString::fromUtf8(name), QString::fromUtf8(path), cnt);
        }
    };

    addAll(pieceListWidget_D, D, int(std::size(D)));
    addAll(pieceListWidget_S, S, int(std::size(S)));
}

void MainWindow::initGameBoardPieces()
{
    auto spawn = [this](int rid, const QString& path, qreal z = 20.0){
        if (!m_gameController) return;
        m_gameController->placeNewPieceToRegion(rid, path, z);
    };

    spawn(1,  ":/D/D_2JBT.png"); // 挪威北部
    spawn(2,  ":/S/S_2JBT.png"); // 摩尔曼斯克
    spawn(6,  ":/S/S_F4.png");   // 列宁格勒
    spawn(11, ":/D/D_4JBT.png"); // 波罗的海国家
    spawn(11, ":/D/D_4JBT.png");
    spawn(14, ":/D/D_4JBT.png"); // 东普鲁士
    spawn(16, ":/D/D_4JBT.png"); // 白俄罗斯
    spawn(16, ":/D/D_4JBT.png");
    spawn(16, ":/D/D_2JBT.png");
    spawn(21, ":/D/D_2JBT.png"); // 波兰南部
    spawn(22, ":/D/D_4JBT.png"); // 利沃夫
    spawn(22, ":/D/D_4JBT.png");
    spawn(23, ":/S/S_4JBT.png"); // 基辅
    spawn(28, ":/S/S_4JBT.png"); // 敖德萨
    spawn(30, ":/L/L_4JBT.png"); // 罗马尼亚
    spawn(30, ":/L/L_3JBT.png");
    spawn(30, ":/D/D_4JBT.png");
    spawn(31, ":/S/S_F4.png");   // 克里米亚
    spawn(34, ":/S/S_3JBT.png"); // 巴库
}

void MainWindow::initEventActions()
{
    // action1：事件投放
    connect(ui->action1, &QAction::triggered, this, [this](){
        QSet<int> allowed = { 2,3,4,5,6,7,8,9,10,11,12,13,16,17,18,19,20,22,23,24,25,26,27,28,31,32,33,34 };
        m_gameController->setEventAllowedRegions(allowed);

        auto* dlg = new EventDialog(this);

        connect(m_gameController, &GameController::eventPiecePlaced, dlg, &EventDialog::onEventPiecePlaced);

        connect(dlg, &QDialog::finished, this, [this, dlg](int){
            m_gameController->clearEventAllowedRegions();
            dlg->deleteLater();
        });

        dlg->setEventId("SLYBD");//事件-苏联预备队
        dlg->addEventPiece("4级兵团", ":/S/S_4JBT.png", 3);
        dlg->show();
    });
}

void MainWindow::refreshStatusUI()
{
    if (!m_gameController) return;
    const auto& st = m_gameController->state();

    if (m_turnLabel) m_turnLabel->setText(QString::number(st.turn));

    if (m_npLabelD)  m_npLabelD->setText(QString::number(st.npD));
    if (m_oilLabelD) m_oilLabelD->setText(QString::number(st.oilD));
    if (m_apLabelD)  m_apLabelD->setText(QString::number(st.apD));
    if (m_rpLabelD)  m_rpLabelD->setText(QString::number(st.rpD));

    if (m_npLabelS)  m_npLabelS->setText(QString::number(st.npS));
    if (m_oilLabelS) m_oilLabelS->setText(QString::number(st.oilS));
    if (m_apLabelS)  m_apLabelS->setText(QString::number(st.apS));
    if (m_rpLabelS)  m_rpLabelS->setText(QString::number(st.rpS));
}

// =====================================================
// 逻辑函数
// =====================================================

void MainWindow::on_action_DTZ_triggered()
{
    int num = QRandomGenerator::global()->bounded(1, 7);
    appendLog(QString("德国掷骰子：%1\n").arg(num), Qt::black, true);
}

void MainWindow::on_action_STZ_triggered()
{
    int num = QRandomGenerator::global()->bounded(1, 7);
    appendLog(QString("苏联掷骰子：%1\n").arg(num), QColor(183,100,50), true);
}

void MainWindow::addPiece(PieceListWidget* list,
                          const QString& name,
                          const QString& pixResPath,
                          int count)
{
    if (!list) return;

    auto* it = new QListWidgetItem;
    it->setSizeHint(QSize(220, 80));
    it->setData(Qt::UserRole, pixResPath);
    it->setData(Qt::UserRole + 1, count);

    list->addItem(it);

    auto* w = new PieceEntryWidget(QIcon(pixResPath), name, list);
    w->setCount(count);
    list->setItemWidget(it, w);
}

void MainWindow::appendLog(const QString& line, const QColor& color, bool newLine)
{
    if (!logTextEdit) return;

    QTextCharFormat fmt;
    fmt.setForeground(QBrush(color));

    QTextCursor cursor = logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(newLine ? (line + "\n") : line, fmt);
    logTextEdit->setTextCursor(cursor);
}

void MainWindow::on_action_D_triggered()
{
    if (!m_gameController) return;
    m_gameController->addAP(Side::D, 6);
}

void MainWindow::on_action_S_triggered()
{
    if (!m_gameController) return;
    m_gameController->addAP(Side::S, 6);
}
