#pragma once

#include <QString>

// enum class Terrain { Plain, Swamp, Mountain };
enum class Side { D, S, L, Unknown };
enum class PieceKind { Corps, Fortress, Fortification, Other };//兵团 要塞 防御工事

bool parseCorpsFromPixPath(const QString& pixPath, Side& side, int& level);
bool parseFortressFromPixPath(const QString& pixPath, Side& side, int& level);
bool parseFortificationFromPixPath(const QString& pixPath, Side& side, int& level);
QString corpsPixPath(Side side, int level);
