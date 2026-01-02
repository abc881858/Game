#pragma once

#include <QString>

enum class Terrain { Plain, Swamp, Mountain };
enum class Side { D, S, L, Unknown };

struct RegionState {
    int id;
    Terrain terrain = Terrain::Plain;
    Side owner = Side::Unknown;              // “占领方”
    bool hasFortress = false;                // 要塞
    int enemyTroopCount[3];                  // 你也可以用列表存 Piece
    int enemyFortificationLevel = 0;         // 防御工事等级（0=无）
};

enum class PieceKind { Corps, Fortress, Fortification, Other };
enum class CorpsSize { Small, Large };

struct PieceState {
    PieceKind kind;
    Side side;
    CorpsSize corpsSize;   // 仅 kind==Corps 时有效
    bool movedThisSeg = false;
    bool canFightThisSeg = true; // 进入复杂地形后置 false
};

bool parseCorpsFromPixPath(const QString& pixPath, Side& side, int& level);
QString corpsPixPath(Side side, int level);
