#pragma once
#include <QDialog>
#include <QListWidget>
#include "battlecontext.h"
#include "util.h"

class BattleFieldDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BattleFieldDialog(QWidget* parent=nullptr);

    void setContent(const QStringList& atkUnits,
                    const QStringList& defUnits,
                    const QList<StrikeGroupEntry>& atkStr,
                    const QList<StrikeGroupEntry>& defStr);
private:
    QListWidget* aU{};
    QListWidget* dU{};
    QListWidget* aS{};
    QListWidget* dS{};
};
