#pragma once
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include "util.h"

class BattleSetupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BattleSetupDialog(QWidget* parent=nullptr);

    void setSides(Side attacker, Side defender);
    void setRegionId(int rid);

public slots:
    void refreshUnits(const QStringList& atkPix, const QStringList& defPix);

private:
    QListWidget* m_atkList{};
    QListWidget* m_defList{};
    QPushButton* m_ok{};
    Side m_atk;
    Side m_def;
    int  m_rid = -1;
};
