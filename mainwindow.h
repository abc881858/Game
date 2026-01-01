#pragma once

#include <QMainWindow>
#include <QGraphicsScene>
#include "DockManager.h"
#include "piecelistwidget.h"
#include <QHash>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CitySlotItem;
class PieceItem;
class QTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setupReadyList();
    void addPieceD(const QString &name, const QString &pixResPath);
    void addPieceS(const QString &name, const QString &pixResPath);
    void rollDiceAnimated(const QString &who);
    void onDiceAnimTick();
private slots:
    void on_action_DTZ_triggered();
    void on_action_STZ_triggered();
private:
    Ui::MainWindow *ui;
    ads::CDockManager* m_DockManager;
    QGraphicsScene *scene;
    PieceListWidget *pieceListWidget_D;
    PieceListWidget *pieceListWidget_S;
    QHash<int, CitySlotItem*> m_slots;
    PieceItem* spawnPieceToCity(int slotId, const QString& pixResPath, qreal z = 20);
    QTextEdit *logTextEdit;
};
