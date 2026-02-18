#pragma once

#include <QListWidget>
#include "util.h"

class GameController;

class PieceListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit PieceListWidget(QWidget* parent=nullptr);
    void setSide(Side s) { m_side = s; }
    void setGameController(GameController* c) { m_controller = c; }
protected:
    void startDrag(Qt::DropActions supportedActions) override;
private:
    Side m_side;
    GameController* m_controller = nullptr;
};
