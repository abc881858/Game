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
#include <QLabel>
#include <QTimer>
#include "view.h"
#include "eventdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QVBoxLayout *hostLayout = new QVBoxLayout(ui->dockWidget);
    hostLayout->setContentsMargins(0,0,0,0);
    hostLayout->setSpacing(0);

    m_DockManager = new ads::CDockManager(ui->dockWidget);
    hostLayout->addWidget(m_DockManager);

    auto *centralDock = new ads::CDockWidget("Center");
    View *view = new View;
    centralDock->setWidget(view);
    m_DockManager->setCentralWidget(centralDock);

    // auto *topDock = new ads::CDockWidget("Label 1");
    // QLabel *l = new QLabel("Lorem ipsum...");
    // l->setWordWrap(true);
    // topDock->setWidget(l);
    // ui->menuView->addAction(topDock->toggleViewAction());
    // m_DockManager->addDockWidget(ads::TopDockWidgetArea, topDock);

    QActionGroup *actionGroup = new QActionGroup(this);
    actionGroup->addAction(ui->action1);
    actionGroup->addAction(ui->action2);
    actionGroup->addAction(ui->action3);
    actionGroup->addAction(ui->action4);
    actionGroup->addAction(ui->action5);
    actionGroup->addAction(ui->action6);

    connect(ui->action1, &QAction::triggered, this, [=](){
        // 允许投放的苏联占领格 slotId
        QSet<int> allowed = { 2,3,4,5,6,7,8,9,10,11,12,13,16,17,18,19,20,22,23,24,25,26,27,28,31,32,33,34 };

        // 开启事件投放限制
        auto* gv = static_cast<GraphicsView*>(view->view()); // 你 view() 返回的是 QGraphicsView*
        // 更稳妥：你可以在 View 提供 GraphicsView* getter；这里先按你当前结构强转
        gv->setEventDropSlots(allowed);

        // 弹对话框
        auto* dlg = new EventDialog(this);
        dlg->setEventId("SOV_RESERVE_3x4");          // 任意字符串即可区分事件
        dlg->addEventPiece("4级兵团", ":/S/S_4JBT.png", 3);

        // drop 成功 -> 对话框删一个
        connect(gv, &GraphicsView::eventPiecePlaced, dlg, &EventDialog::onEventPiecePlaced);

        // 对话框关闭后，关闭事件投放限制（避免影响其他拖拽）
        connect(dlg, &QDialog::finished, this, [=](int){
            gv->clearEventDropSlots();
            dlg->deleteLater();
        });

        dlg->show();
    });


    ui->D->setCurrentIndex(0);
    ui->S->setCurrentIndex(0);

    QGraphicsPixmapItem *back_item = new QGraphicsPixmapItem(QPixmap::fromImage(QImage(":/res/back.png")));
    back_item->setPos(0, 0);
    back_item->setZValue(0);

    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 3164, 4032);
    scene->addItem(back_item);

    view->view()->setScene(scene);

    setupReadyList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addPieceD(const QString& name, const QString& pixResPath)
{
    QListWidgetItem *it = new QListWidgetItem(QIcon(pixResPath), name);
    it->setData(Qt::UserRole, pixResPath);
    pieceListWidget_D->addItem(it);
}

void MainWindow::addPieceS(const QString& name, const QString& pixResPath)
{
    QListWidgetItem *it = new QListWidgetItem(QIcon(pixResPath), name);
    it->setData(Qt::UserRole, pixResPath);
    pieceListWidget_S->addItem(it);
}

PieceItem* MainWindow::spawnPieceToCity(int slotId, const QString& pixResPath, qreal z)
{
    if (!scene) return nullptr;

    CitySlotItem* slot = m_slots.value(slotId, nullptr);
    if (!slot) return nullptr;

    QPixmap pm(pixResPath);
    if (pm.isNull()) return nullptr;

    PieceItem* p = new PieceItem(pm);
    p->setZValue(z);
    scene->addItem(p);
    p->placeToSlot(slot);
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

    addPieceD("1级兵团",":/D/D_1JBT.png");
    addPieceD("2级兵团",":/D/D_2JBT.png");
    addPieceD("3级兵团",":/D/D_3JBT.png");
    addPieceD("4级兵团",":/D/D_4JBT.png");
    addPieceD("1级兵团",":/L/L_1JBT.png");
    addPieceD("2级兵团",":/L/L_2JBT.png");
    addPieceD("3级兵团",":/L/L_3JBT.png");
    addPieceD("4级兵团",":/L/L_4JBT.png");
    addPieceD("1级海军",":/D/D_1JHJ.png");
    addPieceD("2级海军",":/D/D_2JHJ.png");
    addPieceD("1级航空",":/D/D_1JHK.png");
    addPieceD("2级航空",":/D/D_2JHK.png");
    addPieceD("Bf-109",":/D/D_B002.png");
    addPieceD("Fw-190A百舌鸟",":/D/D_B103.png");
    addPieceD("Bf-110破坏者",":/D/D_C102.png");
    addPieceD("Hs-129B-2",":/D/D_D4T.png");
    addPieceD("Ju-87 斯图卡",":/D/D_D57.png");
    addPieceD("He-111P",":/D/D_D1002.png");
    addPieceD("He-111H6",":/D/D_D1502.png");
    addPieceD("Do-17Z",":/D/D_D2102.png");
    addPieceD("Do-217",":/D/D_D2304.png");
    addPieceD("Ju-88A",":/D/D_D3402.png");
    addPieceD("潜艇舰队",":/D/D_DJ2.png");
    addPieceD("战列巡洋舰队+++",":/D/D_E131.png");
    addPieceD("战列舰队+++",":/D/D_E141.png");
    addPieceD("重巡舰队+++",":/D/D_F131.png");
    addPieceD("PzVI 虎I",":/D/D_G6.png");
    addPieceD("PzIIIJ 三号J",":/D/D_H3.png");
    addPieceD("PzIVG 四号G",":/D/D_H4.png");
    addPieceD("StuGIIIF 三突F",":/D/D_HI3T.png");
    addPieceD("StuGIIIG 三突G",":/D/D_HI4T.png");
    addPieceD("PzIII 三号",":/D/D_I3.png");
    addPieceD("PzIV 四号",":/D/D_I4.png");
    addPieceD("PzIIIN 三号N",":/D/D_I5.png");
    addPieceD("Pz38(t)",":/D/D_J3.png");
    addPieceD("装甲步兵41",":/D/D_J25.png");
    addPieceD("精锐航空兵",":/D/D_JRHKB.png");
    addPieceD("精锐潜艇兵",":/D/D_JRQTB.png");
    addPieceD("精锐掷弹兵",":/D/D_JRZDB.png");
    addPieceD("精锐装甲兵",":/D/D_JRZJB.png");

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

    addPieceS("1级兵团",":/S/S_1JBT.png");
    addPieceS("2级兵团",":/S/S_2JBT.png");
    addPieceS("3级兵团",":/S/S_3JBT.png");
    addPieceS("4级兵团",":/S/S_4JBT.png");
    addPieceS("1级海军",":/S/S_1JHJ.png");
    addPieceS("2级海军",":/S/S_2JHJ.png");
    addPieceS("1级航空",":/S/S_1JHK.png");
    addPieceS("2级航空",":/S/S_2JHK.png");
    addPieceS("雅克-1",":/S/S_B002.png");
    addPieceS("米格-3",":/S/S_B002~.png");
    addPieceS("雅克-9",":/S/S_B003.png");
    addPieceS("伊-16",":/S/S_C101.png");
    addPieceS("斯勃-2",":/S/S_D1.png");
    addPieceS("伊尔-2",":/S/S_D2T.png");
    addPieceS("伊尔-2M",":/S/S_D3T.png");
    addPieceS("德勃-3",":/S/S_D1201.png");
    addPieceS("伊尔-4",":/S/S_D1302.png");
    addPieceS("佩-2",":/S/S_D2302.png");
    addPieceS("战列舰队++",":/S/S_E121.png");
    addPieceS("战列舰队++",":/S/S_E131.png");
    addPieceS("KV-1",":/S/S_H3.png");
    addPieceS("T-34/76",":/S/S_H4.png");
    addPieceS("KV-2",":/S/S_I6.png");
    addPieceS("BT-7",":/S/S_J3.png");
    addPieceS("T-70",":/S/S_J3~.png");
    addPieceD("精锐航空兵",":/S/S_JRHKB.png");
    addPieceD("精锐装甲兵",":/S/S_JRZJB.png");

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
