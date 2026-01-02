#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsPixmapItem>
#include "cityslotitem.h"
#include "pieceitem.h"
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QPixmap>
#include <QActionGroup>
#include "piecelistwidget.h"
#include <QTextEdit>
#include <QTimer>
#include "graphicsframe.h"
#include "eventdialog.h"
#include <QRandomGenerator>
#include "pieceentrywidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ===== actions =====
    auto *actionGroup = new QActionGroup(this);
    actionGroup->addAction(ui->action1);
    actionGroup->addAction(ui->action2);
    actionGroup->addAction(ui->action3);
    actionGroup->addAction(ui->action4);
    actionGroup->addAction(ui->action5);
    actionGroup->addAction(ui->action6);

    ui->D->setCurrentIndex(0);
    ui->S->setCurrentIndex(0);

    // ===== Dock host layout first =====
    auto *hostLayout = new QVBoxLayout(ui->dockWidget);
    hostLayout->setContentsMargins(0,0,0,0);
    hostLayout->setSpacing(0);

    // ===== Dock manager first (setupStatusDock needs it) =====
    m_DockManager = new ads::CDockManager(ui->dockWidget);
    hostLayout->addWidget(m_DockManager);

    // ===== Central view + scene (setupReadyList needs scene) =====
    m_graphicsFrame = new GraphicsFrame;
    m_graphicsView = m_graphicsFrame->graphicsView();
    connect(m_graphicsFrame, &GraphicsFrame::pieceMovedCityToCity, this, [=](int /*from*/, int /*to*/, Side side){
        addActionPoints(side, -2);
    });

    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 3164, 4032);

    auto *back_item = new QGraphicsPixmapItem(QPixmap::fromImage(QImage(":/res/back.png")));
    back_item->setPos(0, 0);
    back_item->setZValue(0);
    scene->addItem(back_item);

    m_graphicsView->setScene(scene);

    auto *centralDock = new ads::CDockWidget("场上");
    centralDock->setWidget(m_graphicsFrame);
    m_DockManager->setCentralWidget(centralDock);

    // ===== Log dock =====
    auto *logDock = new ads::CDockWidget("日志");
    logTextEdit = new QTextEdit;
    logDock->setWidget(logTextEdit);
    m_DockManager->addDockWidget(ads::BottomDockWidgetArea, logDock);
    m_DockManager->setSplitterSizes(centralDock->dockAreaWidget(), {800, 200});

    QAction *logAction = logDock->toggleViewAction();
    logAction->setIcon(QIcon(":/res/log.png"));
    logAction->setText("日志");
    ui->menuView->addAction(logAction);
    logDock->toggleView(false);

    // ===== Now safe to call (scene + dock manager already ready) =====
    setupReadyList();
    setupStatusDock();

    // ===== action1 event-drop dialog =====
    connect(ui->action1, &QAction::triggered, this, [=](){
        QSet<int> allowed = { 2,3,4,5,6,7,8,9,10,11,12,13,16,17,18,19,20,22,23,24,25,26,27,28,31,32,33,34 };

        m_graphicsView->setEventDropSlots(allowed);

        auto* dlg = new EventDialog(this);
        dlg->setEventId("SOV_RESERVE_3x4");
        dlg->addEventPiece("4级兵团", ":/S/S_4JBT.png", 3);

        connect(m_graphicsView, &GraphicsView::eventPiecePlaced, dlg, &EventDialog::onEventPiecePlaced);

        connect(dlg, &QDialog::finished, this, [=](int){
            m_graphicsView->clearEventDropSlots();
            dlg->deleteLater();
        });

        dlg->show();
    });

    // ===== 普通落子：苏联大行动签 -> 苏联行动点+6 =====
    connect(m_graphicsView, &GraphicsView::piecePlaced, this, [=](const QString& pixResPath, int /*slotId*/){

        // 只处理行动签
        if (!pixResPath.contains("_XDQ", Qt::CaseSensitive))
            return;

        // 阵营
        Side side = Side::Unknown;
        if (pixResPath.startsWith(":/D/")) side = Side::D;
        else if (pixResPath.startsWith(":/S/")) side = Side::S;
        if (side == Side::Unknown) return;

        // 点数（2 或 6）
        int ap = 0;
        if (pixResPath.contains("XDQ2")) ap = 2;
        else if (pixResPath.contains("XDQ6")) ap = 6;
        if (ap == 0) return;

        addActionPoints(side, ap);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

PieceItem* MainWindow::spawnPieceToCity(int slotId, const QString& pixResPath, qreal z)
{
    if (!scene) return nullptr;
    CitySlotItem* slot = m_slots.value(slotId, nullptr);
    if (!slot) return nullptr;

    QPixmap pm(pixResPath);
    if (pm.isNull()) return nullptr;

    auto* p = new PieceItem(pm);
    connect(p, &PieceItem::movedCityToCity, m_graphicsView, &GraphicsView::pieceMovedCityToCity);

    p->setZValue(z);
    scene->addItem(p);
    p->placeToSlot(slot);

    Side side;
    int lvl;
    if (parseCorpsFromPixPath(pixResPath, side, lvl)) {
        p->setUnitMeta(UnitKind::Corps, side, lvl, pixResPath);
    } else {
        p->setUnitMeta(UnitKind::Other, Side::Unknown, 0, pixResPath);
    }
    return p;
}

void MainWindow::setupReadyList()
{
    const QVector<QRectF> cityRects = {
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

    for (int i = 0; i < cityRects.size(); ++i) {
        CitySlotItem* slot = new CitySlotItem(i, cityRects[i]);
        scene->addItem(slot);
        m_slots.insert(i, slot);
    }

    pieceListWidget_D = new PieceListWidget(ui->D_DMQ);
    pieceListWidget_D->setViewMode(QListView::IconMode);
    pieceListWidget_D->setIconSize(QSize(72,72));
    pieceListWidget_D->setResizeMode(QListView::Adjust);
    pieceListWidget_D->setMovement(QListView::Static);
    pieceListWidget_D->setSpacing(8);
    pieceListWidget_D->setDragEnabled(true);
    pieceListWidget_D->setSelectionMode(QAbstractItemView::SingleSelection);

    QVBoxLayout *layout_D = new QVBoxLayout(ui->D_DMQ);
    layout_D->setContentsMargins(4,4,4,4);
    layout_D->addWidget(pieceListWidget_D);

    addPieceD("德国大行动签",":/D/D_XDQ6.png", 1);
    addPieceD("德国小行动签",":/D/D_XDQ2.png", 1);
    addPieceD("1级兵团",":/D/D_1JBT.png", 0);
    addPieceD("2级兵团",":/D/D_2JBT.png", 0);
    addPieceD("3级兵团",":/D/D_3JBT.png", 0);
    addPieceD("4级兵团",":/D/D_4JBT.png", 0);
    addPieceD("1级兵团",":/L/L_1JBT.png", 0);
    addPieceD("2级兵团",":/L/L_2JBT.png", 0);
    addPieceD("3级兵团",":/L/L_3JBT.png", 0);
    addPieceD("4级兵团",":/L/L_4JBT.png", 0);
    addPieceD("1级海军",":/D/D_1JHJ.png", 0);
    addPieceD("2级海军",":/D/D_2JHJ.png", 0);
    addPieceD("1级航空",":/D/D_1JHK.png", 0);
    addPieceD("2级航空",":/D/D_2JHK.png", 0);
    addPieceD("Bf-109",":/D/D_B002.png", 5);
    addPieceD("Fw-190A 百舌鸟",":/D/D_B103.png", 0);
    addPieceD("Bf-110 破坏者",":/D/D_C102.png", 1);
    addPieceD("Hs-129B-2",":/D/D_D4T.png", 0);
    addPieceD("Ju-87 斯图卡",":/D/D_D57.png", 0);
    addPieceD("He-111P",":/D/D_D1002.png", 0);
    addPieceD("He-111H6",":/D/D_D1502.png", 0);
    addPieceD("Do-17Z",":/D/D_D2102.png", 0);
    addPieceD("Do-217",":/D/D_D2304.png", 0);
    addPieceD("Ju-88A",":/D/D_D3402.png", 0);
    addPieceD("潜艇舰队",":/D/D_DJ2.png", 0);
    addPieceD("战列巡洋舰队+++",":/D/D_E131.png", 0);
    addPieceD("战列舰队+++",":/D/D_E141.png", 0);
    addPieceD("重巡舰队+++",":/D/D_F131.png", 0);
    addPieceD("PzVI 虎I",":/D/D_G6.png", 0);
    addPieceD("PzIIIJ 三号J",":/D/D_H3.png", 0);
    addPieceD("PzIVG 四号G",":/D/D_H4.png", 0);
    addPieceD("StuGIIIF 三突F",":/D/D_HI3T.png", 0);
    addPieceD("StuGIIIG 三突G",":/D/D_HI4T.png", 0);
    addPieceD("PzIII 三号",":/D/D_I3.png", 1);
    addPieceD("PzIV 四号",":/D/D_I4.png", 0);
    addPieceD("PzIIIN 三号N",":/D/D_I5.png", 0);
    addPieceD("Pz38(t)",":/D/D_J3.png", 0);
    addPieceD("装甲步兵41",":/D/D_J25.png", 0);
    addPieceD("精锐航空兵",":/D/D_JRHKB.png", 1);
    addPieceD("精锐潜艇兵",":/D/D_JRQTB.png", 0);
    addPieceD("精锐掷弹兵",":/D/D_JRZDB.png", 0);
    addPieceD("精锐装甲兵",":/D/D_JRZJB.png", 4);

    pieceListWidget_S = new PieceListWidget(ui->S_DMQ);
    pieceListWidget_S->setViewMode(QListView::IconMode);
    pieceListWidget_S->setIconSize(QSize(72,72));
    pieceListWidget_S->setResizeMode(QListView::Adjust);
    pieceListWidget_S->setMovement(QListView::Static);
    pieceListWidget_S->setSpacing(8);
    pieceListWidget_S->setDragEnabled(true);
    pieceListWidget_S->setSelectionMode(QAbstractItemView::SingleSelection);

    QVBoxLayout *layout_S = new QVBoxLayout(ui->S_DMQ);
    layout_S->setContentsMargins(4,4,4,4);
    layout_S->addWidget(pieceListWidget_S);

    addPieceS("苏联大行动签",":/S/S_XDQ6.png", 1);
    addPieceS("苏联小行动签",":/S/S_XDQ2.png", 1);
    addPieceS("1级兵团",":/S/S_1JBT.png", 0);
    addPieceS("2级兵团",":/S/S_2JBT.png", 0);
    addPieceS("3级兵团",":/S/S_3JBT.png", 0);
    addPieceS("4级兵团",":/S/S_4JBT.png", 0);
    addPieceS("1级海军",":/S/S_1JHJ.png", 0);
    addPieceS("2级海军",":/S/S_2JHJ.png", 0);
    addPieceS("1级航空",":/S/S_1JHK.png", 0);
    addPieceS("2级航空",":/S/S_2JHK.png", 0);
    addPieceS("雅克-1",":/S/S_B002.png", 1);
    addPieceS("米格-3",":/S/S_B002~.png", 4);
    addPieceS("雅克-9",":/S/S_B003.png", 0);
    addPieceS("伊-16",":/S/S_C101.png", 7);
    addPieceS("斯勃-2",":/S/S_D1.png", 7);
    addPieceS("伊尔-2",":/S/S_D2T.png", 1);
    addPieceS("伊尔-2M",":/S/S_D3T.png", 0);
    addPieceS("德勃-3",":/S/S_D1201.png", 1);
    addPieceS("伊尔-4",":/S/S_D1302.png", 1);
    addPieceS("佩-2",":/S/S_D2302.png", 1);
    addPieceS("战列舰队++",":/S/S_E121.png", 0);
    addPieceS("战列舰队++",":/S/S_E131.png", 0);
    addPieceS("KV-1",":/S/S_H3.png", 0);
    addPieceS("T-34/76",":/S/S_H4.png", 1);
    addPieceS("KV-2",":/S/S_I6.png", 0);
    addPieceS("BT-7",":/S/S_J3.png", 0);
    addPieceS("T-70",":/S/S_J3~.png", 0);
    addPieceS("精锐航空兵",":/S/S_JRHKB.png", 0);
    addPieceS("精锐装甲兵",":/S/S_JRZJB.png", 0);

    // addPieceS("1级要塞",":/S/S_F1.png");
    // addPieceS("2级要塞",":/S/S_F2.png");
    // addPieceS("3级要塞",":/S/S_F3.png");
    // addPieceS("4级要塞",":/S/S_F4.png");

    spawnPieceToCity(1, ":/D/D_2JBT.png");//挪威北部
    spawnPieceToCity(2, ":/S/S_2JBT.png");//摩尔曼斯克
    spawnPieceToCity(6, ":/S/S_F4.png");//列宁格勒
    spawnPieceToCity(11, ":/D/D_4JBT.png");//波罗的海国家
    spawnPieceToCity(11, ":/D/D_4JBT.png");//波罗的海国家
    spawnPieceToCity(14, ":/D/D_4JBT.png");//东普鲁士
    spawnPieceToCity(16, ":/D/D_4JBT.png");//白俄罗斯
    spawnPieceToCity(16, ":/D/D_4JBT.png");//白俄罗斯
    spawnPieceToCity(16, ":/D/D_2JBT.png");//白俄罗斯
    spawnPieceToCity(21, ":/D/D_2JBT.png");//波兰南部
    spawnPieceToCity(22, ":/D/D_4JBT.png");//利沃夫
    spawnPieceToCity(22, ":/D/D_4JBT.png");//利沃夫
    spawnPieceToCity(23, ":/S/S_4JBT.png");//基辅
    spawnPieceToCity(28, ":/S/S_4JBT.png");//敖德萨
    spawnPieceToCity(30, ":/L/L_4JBT.png");//罗马尼亚
    spawnPieceToCity(30, ":/L/L_3JBT.png");//罗马尼亚
    spawnPieceToCity(30, ":/D/D_4JBT.png");//罗马尼亚
    spawnPieceToCity(31, ":/S/S_F4.png");//克里米亚
    spawnPieceToCity(34, ":/S/S_3JBT.png");//巴库
}

void MainWindow::on_action_DTZ_triggered()
{
    int num = QRandomGenerator::global()->bounded(1, 7);

    QTextCharFormat fmt;
    fmt.setForeground(QBrush(Qt::black));

    QTextCursor cursor = logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(QString("德国掷骰子：%1\n").arg(num), fmt);
    logTextEdit->setTextCursor(cursor);
}

void MainWindow::on_action_STZ_triggered()
{
    int num = QRandomGenerator::global()->bounded(1, 7);

    QTextCharFormat fmt;
    fmt.setForeground(QBrush(QColor(183,100,50)));

    QTextCursor cursor = logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(QString("苏联掷骰子：%1\n").arg(num), fmt);
    logTextEdit->setTextCursor(cursor);
}

void MainWindow::addPieceD(const QString& name, const QString& pixResPath, int count)
{
    auto* it = new QListWidgetItem;
    it->setSizeHint(QSize(220, 80));         // 控制每行高度/宽度
    it->setData(Qt::UserRole, pixResPath);
    it->setData(Qt::UserRole + 1, count);

    pieceListWidget_D->addItem(it);

    auto* w = new PieceEntryWidget(QIcon(pixResPath), name, pieceListWidget_D);
    w->setCount(count);
    pieceListWidget_D->setItemWidget(it, w);
}

void MainWindow::addPieceS(const QString& name, const QString& pixResPath, int count)
{
    auto* it = new QListWidgetItem;
    it->setSizeHint(QSize(220, 80));
    it->setData(Qt::UserRole, pixResPath);
    it->setData(Qt::UserRole + 1, count);

    pieceListWidget_S->addItem(it);

    auto* w = new PieceEntryWidget(QIcon(pixResPath), name, pieceListWidget_S);
    w->setCount(count);
    pieceListWidget_S->setItemWidget(it, w);
}

void MainWindow::setupStatusDock()
{
    auto *statusDock = new ads::CDockWidget("状态");
    auto *w = new QWidget;
    statusDock->setWidget(w);

    auto *grid = new QGridLayout(w);
    grid->setContentsMargins(8, 8, 8, 8);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(6);

    // ===== 左侧：回合（单独一列）=====
    auto *lbTurn = new QLabel("回合");
    m_turnLabel = new QLabel("1");

    lbTurn->setAlignment(Qt::AlignCenter);
    m_turnLabel->setAlignment(Qt::AlignCenter);

    // 放在第0列，跨多行（0~2行：表头+德国+苏联）
    grid->addWidget(lbTurn, 0, 0, 1, 1);
    grid->addWidget(m_turnLabel, 1, 0, 2, 1);

    // ===== 表头：指标列 =====
    grid->addWidget(new QLabel(""), 0, 1);  // 行标题占位
    grid->addWidget(new QLabel("国力"), 0, 2);
    grid->addWidget(new QLabel("石油"), 0, 3);
    grid->addWidget(new QLabel("行动点"), 0, 4);
    grid->addWidget(new QLabel("准备点"), 0, 5);

    // ===== 德国行 =====
    auto* headD = new QLabel("德国");
    m_npLabelD  = new QLabel("0");
    m_oilLabelD = new QLabel("0");
    m_apLabelD  = new QLabel("0");
    m_rpLabelD  = new QLabel("0");

    grid->addWidget(headD, 1, 1);
    grid->addWidget(m_npLabelD, 1, 2);
    grid->addWidget(m_oilLabelD, 1, 3);
    grid->addWidget(m_apLabelD, 1, 4);
    grid->addWidget(m_rpLabelD, 1, 5);

    // ===== 苏联行 =====
    auto* headS = new QLabel("苏联");
    m_npLabelS  = new QLabel("0");
    m_oilLabelS = new QLabel("0");
    m_apLabelS  = new QLabel("0");
    m_rpLabelS  = new QLabel("0");

    grid->addWidget(headS, 2, 1);
    grid->addWidget(m_npLabelS, 2, 2);
    grid->addWidget(m_oilLabelS, 2, 3);
    grid->addWidget(m_apLabelS, 2, 4);
    grid->addWidget(m_rpLabelS, 2, 5);

    m_DockManager->addDockWidget(ads::TopDockWidgetArea, statusDock);

    refreshStatusUI();

    QAction *apAction = statusDock->toggleViewAction();
    apAction->setIcon(QIcon(":/res/status.png"));
    apAction->setText("状态");
    ui->menuView->addAction(apAction);

    statusDock->toggleView(false);
}

void MainWindow::refreshStatusUI()
{
    if (m_turnLabel) m_turnLabel->setText(QString::number(m_turn));

    if (m_npLabelD)  m_npLabelD->setText(QString::number(m_npD));
    if (m_npLabelS)  m_npLabelS->setText(QString::number(m_npS));

    if (m_oilLabelD) m_oilLabelD->setText(QString::number(m_oilD));
    if (m_oilLabelS) m_oilLabelS->setText(QString::number(m_oilS));

    if (m_apLabelD)  m_apLabelD->setText(QString::number(m_apD));
    if (m_apLabelS)  m_apLabelS->setText(QString::number(m_apS));

    if (m_rpLabelD)  m_rpLabelD->setText(QString::number(m_rpD));
    if (m_rpLabelS)  m_rpLabelS->setText(QString::number(m_rpS));
}

void MainWindow::addTurn(int delta)
{
    m_turn += delta;
    refreshStatusUI();
}

void MainWindow::addNationalPower(Side side, int delta)
{
    if (side == Side::D) m_npD += delta;
    else if (side == Side::S) m_npS += delta;
    refreshStatusUI();
}

void MainWindow::addOil(Side side, int delta)
{
    if (side == Side::D) m_oilD += delta;
    else if (side == Side::S) m_oilS += delta;
    refreshStatusUI();
}

void MainWindow::addActionPoints(Side side, int delta)
{
    if (side == Side::D) m_apD += delta;
    else if (side == Side::S) m_apS += delta;

    if (m_apLabelD) m_apLabelD->setText(QString::number(m_apD));
    if (m_apLabelS) m_apLabelS->setText(QString::number(m_apS));

    // 顺便写日志（可选）
    QTextCharFormat fmt;
    fmt.setForeground(QBrush(Qt::black));
    QTextCursor cursor = logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);

    const QString who = (side == Side::D ? "德国" : "苏联");
    cursor.insertText(QString("%1行动点 %2%3，当前：D=%4 S=%5\n")
        .arg(who)
        .arg(delta >= 0 ? "+" : "")
        .arg(delta)
        .arg(m_apD)
        .arg(m_apS), fmt);

    logTextEdit->setTextCursor(cursor);

    refreshStatusUI();
}

void MainWindow::addReadyPoints(Side side, int delta)
{
    if (side == Side::D) m_rpD += delta;
    else if (side == Side::S) m_rpS += delta;
    refreshStatusUI();
}
