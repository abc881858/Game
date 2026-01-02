#pragma once

#include <QMainWindow>
#include <QGraphicsScene>
#include "DockManager.h"
#include "piecelistwidget.h"
#include <QHash>
#include <QLabel>
#include "util.h"
#include "slotmanager.h"
#include "gamecontroller.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CitySlotItem;
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
    void setupStatusDock();
    void setupReadyList();
    void addPieceD(const QString &name, const QString &pixResPath, int count);
    void addPieceS(const QString &name, const QString &pixResPath, int count);
    void addTurn(int delta);
    void addNationalPower(Side side, int delta);
    void addOil(Side side, int delta);
    void addReadyPoints(Side side, int delta);
    void refreshStatusUI();

private:
    Ui::MainWindow *ui;
    ads::CDockManager* m_DockManager;
    QGraphicsScene *scene;
    PieceListWidget *pieceListWidget_D;
    PieceListWidget *pieceListWidget_S;
    QHash<int, CitySlotItem*> m_slots;
    PieceItem* spawnPieceToCity(int slotId, const QString& pixResPath, qreal z = 20);
    QTextEdit *logTextEdit;

    // ===== 状态数值 =====
    int m_turn = 1;            // 回合 1-8

    int m_npD = 0;             // 德国国力
    int m_oilD = 0;            // 德国石油
    int m_apD  = 0;            // 德国行动点（你已有）
    int m_rpD  = 0;            // 德国准备点

    int m_npS = 0;             // 苏联国力
    int m_oilS = 0;            // 苏联石油
    int m_apS  = 0;            // 苏联行动点（你已有）
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


    GraphicsFrame *m_graphicsFrame;
    GraphicsView *m_graphicsView;

    SlotManager* m_slotMgr = nullptr;
    GameController* m_ctrl = nullptr;

public slots:
    void addActionPoints(Side side, int delta);  // 加/扣行动点
    void appendLog(const QString &line, const QColor &color, bool newLine);

private slots:
    void on_action_DTZ_triggered();
    void on_action_STZ_triggered();
};
