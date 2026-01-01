#pragma once

#include <QListWidget>

class PieceListWidget : public QListWidget {
    Q_OBJECT
public:
    explicit PieceListWidget(QWidget* parent=nullptr);

    void setItemCount(QListWidgetItem* it, int c);
    int  itemCount(const QListWidgetItem* it) const;

protected:
    void startDrag(Qt::DropActions supportedActions) override;

private:
    static constexpr int RolePixPath = Qt::UserRole;
    static constexpr int RoleCount   = Qt::UserRole + 1;
};
