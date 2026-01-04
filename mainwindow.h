#pragma once

#include <QMainWindow>
#include <QGraphicsScene>
#include <QLabel>
#include "DockManager.h"
#include "piecelistwidget.h"
#include "util.h"
#include "placementmanager.h"
#include "gamecontroller.h"
#include "navprogress.h"

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

public slots:
    void appendLog(const QString &line, const QColor &color, bool newLine);
    void refreshStatusUI();

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

    NavProgress *m_navProgress;
};
