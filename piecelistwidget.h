#pragma once

#include <QListWidget>

class PieceListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit PieceListWidget(QWidget* parent=nullptr);

protected:
    void startDrag(Qt::DropActions supportedActions) override;
};
