#pragma once
#include <QList>
#include <QHash>
#include <QString>
#include "util.h"

class PieceItem;

struct StrikeGroupEntry {
    QString pixPath;
    bool eliteBound = false;
};

struct BattleContext
{
    bool active = false;
    int battleRegionId = -1;

    Side attacker = Side::Unknown;
    Side defender = Side::Unknown;

    QList<PieceItem*> attackers;   // 参战兵团（棋子指针）
    QList<PieceItem*> defenders;

    QList<StrikeGroupEntry> strikeA; // attacker strike groups
    QList<StrikeGroupEntry> strikeD; // defender strike groups
};
