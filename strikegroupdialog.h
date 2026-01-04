#pragma once
#include <QDialog>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include "battlecontext.h"
#include "util.h"

class StrikeGroupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StrikeGroupDialog(QWidget* parent=nullptr);

    void setSides(Side attacker, Side defender);

public slots:
    void refreshStrikeGroups(const QList<StrikeGroupEntry>& atk,
                             const QList<StrikeGroupEntry>& def,
                             Side currentTurnSide,
                             bool finished);

private:
    QLabel* m_tip{};
    QListWidget* m_atk{};
    QListWidget* m_def{};
    QPushButton* m_pass{};
    Side m_atkSide = Side::Unknown;
    Side m_defSide = Side::Unknown;

signals:
    void passClicked();
};
