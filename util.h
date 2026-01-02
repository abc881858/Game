#pragma once

#include <QString>

enum class Side { D, S, L, Unknown }; // ✅建议把 Side 挪到 util 或单独 header
enum class UnitKind { Corps, Other }; // Corps=兵团（1~4级），Other=飞机坦克等

bool parseCorpsFromPixPath(const QString& pixPath, Side& side, int& level);
QString corpsPixPath(Side side, int level);
