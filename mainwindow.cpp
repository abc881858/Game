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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->D->setCurrentIndex(0);
    ui->S->setCurrentIndex(0);

    initDockSystem();
    initCentralView();
    initScene();
    initControllers();
    initLogDock();
    initStatusDock();
    initPieceLists();
    refreshStatusUI();
}

MainWindow::~MainWindow()
{
    delete ui;
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
    m_segmentWidget = new SegmentWidget(this);
    m_segmentWidget->setBottomText(QStringList() << "陆上移动" << "陆战" << "调动" << "准备" << "补给");
    m_segmentWidget->setCurrentSegment(0);

    ui->toolBar->addWidget(m_segmentWidget);
    ui->toolBar->addAction(ui->action_End);
    ui->toolBar->addAction(ui->action_DTZ);
    ui->toolBar->addAction(ui->action_STZ);

    m_gameController = new GameController(scene, this);

    connect(ui->action_Event1, &QAction::triggered, this, &MainWindow::actionEvent1);
    connect(ui->action_DTZ, &QAction::triggered, this, &MainWindow::actionRollDiceD);
    connect(ui->action_STZ, &QAction::triggered, this, &MainWindow::actionRollDiceS);
    connect(ui->action_End, &QAction::triggered, m_gameController, &GameController::goNextSegment);
    connect(ui->action_D, &QAction::triggered, m_gameController, &GameController::setFirstPlayerD);
    connect(ui->action_S, &QAction::triggered, m_gameController, &GameController::setFirstPlayerS);

    connect(m_graphicsView, &GraphicsView::dropPieceToScene, m_gameController, &GameController::dropPieceToScene);

    connect(m_gameController, &GameController::setCurrentSegment, m_segmentWidget, &SegmentWidget::setCurrentSegment);

    connect(m_gameController, &GameController::logLine, this, &MainWindow::appendLog);
    connect(m_gameController, &GameController::stateChanged, this, &MainWindow::refreshStatusUI);
    connect(m_gameController, &GameController::setCurrentSegment, this, &MainWindow::setCurrentSegment);
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
}

void MainWindow::initStatusDock()
{
    auto *statusDock = new ads::CDockWidget("状态");
    auto *w = new QWidget;
    statusDock->setWidget(w);

    auto *grid = new QGridLayout(w);
    grid->setContentsMargins(8, 8, 8, 8);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(6);

    m_turnLabel = new QLabel;
    m_npLabelD  = new QLabel;
    m_oilLabelD = new QLabel;
    m_apLabelD  = new QLabel;
    m_rpLabelD  = new QLabel;
    m_npLabelS  = new QLabel;
    m_apLabelS  = new QLabel;
    m_rpLabelS  = new QLabel;

    auto *lbTurn = new QLabel("回合");
    lbTurn->setAlignment(Qt::AlignCenter);
    m_turnLabel->setAlignment(Qt::AlignCenter);

    grid->addWidget(lbTurn, 0, 0, 1, 1);
    grid->addWidget(m_turnLabel, 1, 0, 2, 1);

    grid->addWidget(new QLabel("国力"), 0, 2);
    grid->addWidget(new QLabel("石油"), 0, 3);
    grid->addWidget(new QLabel("行动点"), 0, 4);
    grid->addWidget(new QLabel("准备点"), 0, 5);

    grid->addWidget(new QLabel("德国"), 1, 1);
    grid->addWidget(m_npLabelD, 1, 2);
    grid->addWidget(m_oilLabelD, 1, 3);
    grid->addWidget(m_apLabelD, 1, 4);
    grid->addWidget(m_rpLabelD, 1, 5);

    grid->addWidget(new QLabel("苏联"), 2, 1);
    grid->addWidget(m_npLabelS, 2, 2);
    grid->addWidget(m_apLabelS, 2, 4);
    grid->addWidget(m_rpLabelS, 2, 5);

    m_dockManager->addDockWidget(ads::TopDockWidgetArea, statusDock);

    QAction *statusAction = statusDock->toggleViewAction();
    statusAction->setIcon(QIcon(":/res/status.png"));
    statusAction->setText("状态");
    ui->toolBar->addAction(statusAction);
}

void MainWindow::initPieceLists()
{
    auto createPieceList = [&](QWidget* host) {
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
    };

    pieceListWidget_D_DMQ = createPieceList(ui->D_DMQ);
    pieceListWidget_D_XZQ = createPieceList(ui->D_XZQ);
    pieceListWidget_D_CX = createPieceList(ui->D_CX);
    pieceListWidget_S_DMQ = createPieceList(ui->S_DMQ);
    pieceListWidget_S_XZQ = createPieceList(ui->S_XZQ);
    pieceListWidget_S_CX = createPieceList(ui->S_CX);
    pieceListWidget_D_DMQ->setGameController(m_gameController);
    pieceListWidget_D_XZQ->setGameController(m_gameController);
    pieceListWidget_D_CX->setGameController(m_gameController);
    pieceListWidget_S_DMQ->setGameController(m_gameController);
    pieceListWidget_S_XZQ->setGameController(m_gameController);
    pieceListWidget_S_CX->setGameController(m_gameController);

    auto addPiece = [](PieceListWidget* list, const QString& name, const QString& pixResPath, int count) {
        if (!list) return;

        QListWidgetItem* it = new QListWidgetItem;
        it->setSizeHint(QSize(220, 80));
        it->setData(Qt::UserRole, pixResPath);
        it->setData(Qt::UserRole + 1, count);

        list->addItem(it);

        PieceEntryWidget* w = new PieceEntryWidget(QIcon(pixResPath), name, list);
        w->setCount(count);
        list->setItemWidget(it, w);
    };

    using Def = std::tuple<const char*, const char*, int>;

    const Def D_DMQ[] = {
        {"德国大行动签",":/D/D_XDQ6.png", 1},
        {"德国小行动签",":/D/D_XDQ2.png", 1},
        {"Bf-109",":/D/D_B002.png", 5},
        {"Bf-110 破坏者",":/D/D_C102.png", 1},
        {"PzIII 三号",":/D/D_I3.png", 1},
        {"精锐航空兵",":/D/D_JRHKB.png", 1},
        {"精锐装甲兵",":/D/D_JRZJB.png", 4},
    };

    const Def S_DMQ[] = {
        {"苏联大行动签",":/S/S_XDQ6.png", 1},
        {"苏联小行动签",":/S/S_XDQ2.png", 1},
        {"雅克-1",":/S/S_B002.png", 1},
        {"米格-3",":/S/S_B002~.png", 4},
        {"伊-16",":/S/S_C101.png", 7},
        {"斯勃-2",":/S/S_D1.png", 7},
        {"伊尔-2",":/S/S_D2T.png", 1},
        {"德勃-3",":/S/S_D1201.png", 1},
        {"伊尔-4",":/S/S_D1302.png", 1},
        {"佩-2",":/S/S_D2302.png", 1},
        {"T-34/76",":/S/S_H4.png", 1},
    };

    const Def D_XZQ[] = {
        {"Ju-87 斯图卡",":/D/D_D57.png", 2},
        {"He-111P",":/D/D_D1002.png", 1},
        {"Do-17Z",":/D/D_D2102.png", 1},
        {"Ju-88A",":/D/D_D3402.png", 2},
        {"装甲步兵41",":/D/D_J25.png", 1},
        {"精锐航空兵",":/D/D_JRHKB.png", 2},
        {"精锐掷弹兵",":/D/D_JRZDB.png", 1},
    };

    const Def D_CX[] = {
        {"1级兵团",":/D/D_1JBT.png", 28},
        {"Bf-109",":/D/D_B002.png", 7},
        {"Fw-190A 百舌鸟",":/D/D_B103.png", 8},
        {"Bf-110 破坏者",":/D/D_C102.png", 4},
        {"Hs-129B-2",":/D/D_D4T.png", 1},
        {"Ju-87 斯图卡",":/D/D_D57.png", 4},
        {"He-111P",":/D/D_D1002.png", 4},
        {"He-111H6",":/D/D_D1502.png", 2},
        {"Do-17Z",":/D/D_D2102.png", 1},
        {"Do-217",":/D/D_D2304.png", 1 },
        {"Ju-88A",":/D/D_D3402.png", 3},
        {"PzVI 虎I",":/D/D_G6.png", 1},
        {"PzIIIJ 三号J",":/D/D_H3.png", 4},
        {"PzIVG 四号G",":/D/D_H4.png", 3},
        {"StuGIIIF 三突F",":/D/D_HI3T.png", 1},
        {"StuGIIIG 三突G",":/D/D_HI4T.png", 2},
        {"PzIII 三号",":/D/D_I3.png", 6},
        {"PzIV 四号",":/D/D_I4.png", 3},
        {"PzIIIN 三号N",":/D/D_I5.png", 2},
        {"Pz38(t)",":/D/D_J3.png", 2},
        {"装甲步兵41",":/D/D_J25.png", 6},
        {"精锐航空兵",":/D/D_JRHKB.png", 0},
        {"精锐掷弹兵",":/D/D_JRZDB.png", 0},
        {"精锐装甲兵",":/D/D_JRZJB.png", 0},
    };

    const Def S_CX[] = {
        {"1级兵团",":/S/S_1JBT.png", 27},
        {"雅克-1",":/S/S_B002.png", 7},
        {"米格-3",":/S/S_B002~.png", 5},
        {"雅克-9",":/S/S_B003.png", 7},
        {"伊-16",":/S/S_C101.png", 11},
        {"斯勃-2",":/S/S_D1.png", 11},
        {"伊尔-2",":/S/S_D2T.png", 2},
        {"伊尔-2M",":/S/S_D3T.png", 2},
        {"德勃-3",":/S/S_D1201.png", 4},
        {"伊尔-4",":/S/S_D1302.png", 4},
        {"佩-2",":/S/S_D2302.png", 2},
        {"KV-1",":/S/S_H3.png", 2},
        {"T-34/76",":/S/S_H4.png", 7},
        {"KV-2",":/S/S_I6.png", 1},
        {"BT-7",":/S/S_J3.png", 11},
        {"T-70",":/S/S_J3~.png", 4},
        {"精锐航空兵",":/S/S_JRHKB.png", 0},
        {"精锐装甲兵",":/S/S_JRZJB.png", 0},
    };

    auto fillList = [&](PieceListWidget* list, Side side, const Def* arr, int n) {
        if (!list || n==0) return;
        list->setSide(side);
        for (int i = 0; i < n; ++i) {
            const auto& [name, path, cnt] = arr[i];
            addPiece(list, QString::fromUtf8(name), QString::fromUtf8(path), cnt);
        }
    };

    fillList(pieceListWidget_D_DMQ, Side::D, D_DMQ, int(std::size(D_DMQ)));
    fillList(pieceListWidget_S_DMQ, Side::S, S_DMQ, int(std::size(S_DMQ)));
    fillList(pieceListWidget_D_XZQ, Side::D, D_XZQ, int(std::size(D_XZQ)));
    fillList(pieceListWidget_D_CX, Side::D, D_CX, int(std::size(D_CX)));
    fillList(pieceListWidget_S_CX, Side::S, S_CX, int(std::size(S_CX)));
}

void MainWindow::actionEvent1()
{
    QSet<int> allowed = { 2,3,4,5,6,7,8,9,10,11,12,13,16,17,18,19,20,22,23,24,25,26,27,28,31,32,33,34 };
    m_gameController->setEventAllowedRegions(allowed);

    EventDialog* dlg = new EventDialog(this);

    connect(dlg, &QDialog::finished, this, [this, dlg](int){
        m_gameController->clearEventAllowedRegions();
        dlg->deleteLater();
    });

    dlg->setEventId("SLYBD");//事件-苏联预备队
    dlg->addEventPiece("4级兵团", ":/S/S_4JBT.png", 3);
    dlg->show();
}

void MainWindow::actionRollDiceD()
{
    int num = QRandomGenerator::global()->bounded(1, 7);
    appendLog(QString("德国掷骰子：%1\n").arg(num), Qt::black, true);
}

void MainWindow::actionRollDiceS()
{
    int num = QRandomGenerator::global()->bounded(1, 7);
    appendLog(QString("苏联掷骰子：%1\n").arg(num), QColor(183,100,50), true);
}

void MainWindow::setCurrentSegment(int currentSegment)
{
    ui->action_End->setDisabled(currentSegment == 0);
}

void MainWindow::refreshStatusUI()
{
    const auto& st = m_gameController->gameState();

    if (m_turnLabel) m_turnLabel->setText(QString::number(st.turn));
    if (m_npLabelD)  m_npLabelD->setText(QString::number(st.npD));
    if (m_oilLabelD) m_oilLabelD->setText(QString::number(st.oilD));
    if (m_apLabelD)  m_apLabelD->setText(QString::number(st.apD));
    if (m_rpLabelD)  m_rpLabelD->setText(QString::number(st.rpD));
    if (m_npLabelS)  m_npLabelS->setText(QString::number(st.npS));
    if (m_apLabelS)  m_apLabelS->setText(QString::number(st.apS));
    if (m_rpLabelS)  m_rpLabelS->setText(QString::number(st.rpS));
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
