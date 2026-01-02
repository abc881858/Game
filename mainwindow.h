#pragma once

#include <QMainWindow>
#include <QGraphicsScene>
#include <QLabel>
#include "DockManager.h"
#include "piecelistwidget.h"
#include "util.h"
#include "placementmanager.h"
#include "gamecontroller.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class PieceItem;
class QTextEdit;
class GraphicsFrame;
class GraphicsView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void addPieceD(const QString &name, const QString &pixResPath, int count);
    void addPieceS(const QString &name, const QString &pixResPath, int count);
    void addTurn(int delta);
    void addNationalPower(Side side, int delta);
    void addOil(Side side, int delta);
    void addReadyPoints(Side side, int delta);

public slots:
    void addActionPoints(Side side, int delta);  // 加/扣行动点
    void appendLog(const QString &line, const QColor &color, bool newLine);

private slots:
    void on_action_DTZ_triggered();
    void on_action_STZ_triggered();

private:
    void initActions();
    void initDockSystem();
    void initCentralView();
    void initScene();
    void initControllers();
    void initLogDock();
    void initStatusDock();
    void initRegionItems();
    void initPieceLists();
    void initGameBoardPieces();
    void initEventActions();
    void refreshStatusUI();

    // 小工具：创建 D/S 列表
    PieceListWidget* createPieceList(QWidget* host);

private:
    Ui::MainWindow *ui = nullptr;

    // Docking
    ads::CDockManager* m_dockManager = nullptr;
    ads::CDockWidget*  m_centralDock = nullptr;

    // Graphics
    QGraphicsScene *scene = nullptr;
    GraphicsFrame *m_graphicsFrame = nullptr;
    GraphicsView *m_graphicsView = nullptr;

    // Lists
    PieceListWidget *pieceListWidget_D = nullptr;
    PieceListWidget *pieceListWidget_S = nullptr;

    // Controller
    PlacementManager* m_placementManager = nullptr;
    GameController* m_gameController = nullptr;

    // Log
    QTextEdit *logTextEdit = nullptr;

    // ===== 状态数值 =====
    int m_turn = 1;            // 回合 1-8

    int m_npD = 0;             // 德国国力
    int m_oilD = 0;            // 德国石油
    int m_apD  = 0;            // 德国行动点
    int m_rpD  = 0;            // 德国准备点

    int m_npS = 0;             // 苏联国力
    int m_oilS = 0;            // 苏联石油
    int m_apS  = 0;            // 苏联行动点
    int m_rpS  = 0;            // 苏联准备点

    // ===== 状态显示控件 =====
    QLabel* m_turnLabel = nullptr;

    QLabel* m_npLabelD = nullptr;
    QLabel* m_oilLabelD = nullptr;
    QLabel* m_apLabelD = nullptr;
    QLabel* m_rpLabelD = nullptr;

    QLabel* m_npLabelS = nullptr;
    QLabel* m_oilLabelS = nullptr;
    QLabel* m_apLabelS = nullptr;
    QLabel* m_rpLabelS = nullptr;
};
