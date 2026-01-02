#pragma once

#include <QListWidget>
#include "util.h"

class GameController;

class PieceListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit PieceListWidget(QWidget* parent=nullptr);

    void setItemCount(QListWidgetItem* it, int c);
    int  itemCount(const QListWidgetItem* it) const;

    // ✅ 新增：让列表知道自己属于哪方 + 询问 controller 是否允许拖
    void setSide(Side s) { m_side = s; }
    void setGameController(GameController* c) { m_controller = c; }

protected:
    void startDrag(Qt::DropActions supportedActions) override;

private:
    static constexpr int RolePixPath = Qt::UserRole;
    static constexpr int RoleCount   = Qt::UserRole + 1;

    Side m_side = Side::Unknown;
    GameController* m_controller = nullptr;
};
