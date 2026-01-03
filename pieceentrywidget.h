#pragma once

#include <QWidget>
#include <QLabel>

class PieceEntryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PieceEntryWidget(const QIcon& icon, const QString& name, QWidget* parent=nullptr);
    void setCount(int c);
    int count() const { return m_count; }
    void setDisabledVisual(bool dis);
private:
    QLabel* m_iconLabel{};
    QLabel* m_nameLabel{};
    QLabel* m_countLabel{};
    int m_count = 0;
};
