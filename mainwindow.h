#pragma once

#include <QMainWindow>
#include <QGraphicsScene>
#include <QLabel>
#include "DockManager.h"
#include "piecelistwidget.h"
#include "util.h"
#include "gamecontroller.h"
#include "segmentwidget.h"

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

private:
    Ui::MainWindow *ui = nullptr;

    void initDockSystem();
    void initCentralView();
    void initScene();
    void initControllers();
    void initLogDock();
    void initStatusDock();
    void initPieceLists();

    ads::CDockManager* m_dockManager = nullptr;
    ads::CDockWidget*  m_centralDock = nullptr;

    QGraphicsScene *scene = nullptr;
    GraphicsFrame *m_graphicsFrame = nullptr;
    GraphicsView *m_graphicsView = nullptr;

    PieceListWidget *pieceListWidget_D_DMQ = nullptr;
    PieceListWidget *pieceListWidget_D_XZQ = nullptr;
    PieceListWidget *pieceListWidget_D_CX = nullptr;
    PieceListWidget *pieceListWidget_S_DMQ = nullptr;
    PieceListWidget *pieceListWidget_S_XZQ = nullptr;
    PieceListWidget *pieceListWidget_S_CX = nullptr;

    SegmentWidget *m_segmentWidget = nullptr;

    GameController* m_gameController = nullptr;

    QTextEdit *logTextEdit = nullptr;

    QLabel* m_turnLabel = nullptr;
    QLabel* m_npLabelD = nullptr;
    QLabel* m_oilLabelD = nullptr;
    QLabel* m_apLabelD = nullptr;
    QLabel* m_rpLabelD = nullptr;
    QLabel* m_npLabelS = nullptr;
    QLabel* m_apLabelS = nullptr;
    QLabel* m_rpLabelS = nullptr;

public slots:
    void actionEvent1();
    void actionRollDiceD();
    void actionRollDiceS();
    void setCurrentSegment(int currentSegment);
    void refreshStatusUI();
    void appendLog(const QString &line, const QColor &color, bool newLine);
};
