#pragma once

#include <QString>

// enum class Terrain { Plain, Swamp, Mountain };
enum class Side { D, S, L, Unknown };
enum class PieceKind { Corps, Fortress, Fortification, Other };

bool parseCorpsFromPixPath(const QString& pixPath, Side& side, int& level);
QString corpsPixPath(Side side, int level);
