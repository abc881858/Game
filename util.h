#pragma once

#include <QString>
#include <QStringList>
#include <QMimeData>
#include <QGraphicsItem>

inline constexpr int RegionType = QGraphicsItem::UserType + 100;
inline constexpr int PieceType = QGraphicsItem::UserType + 200;

inline constexpr const char* MimeListPiece   = "application/x-list-piece";
inline constexpr const char* MimeListToken   = "application/x-list-token";
inline constexpr const char* MimeDialogPiece = "application/x-event-piece";

enum class Side { D, S };

//陆地格枚举值
enum class Terrain {     // 地形
    Plain,               // 平原
    Swamp,               // 沼泽
    Mountain             // 山脉
};

//棋子枚举值
enum class Nation {      // 国家
    Germany,             // 德国
    Italy,               // 意大利
    Romania,             // 罗马尼亚
    Hungary,             // 匈牙利
    Soviet,              // 苏联
    SovietGuards         // 苏联近卫
};

enum class PieceKind {   // 棋子种类
    Corps,               // 兵团
    Fortress,            // 要塞
    StrikeGroup,         // 打击群
    Elite,               // 精锐
    Fortification,       // 防御工事
    TerrainObstacle,     // 地形障碍
};

enum class TurnPhase {   // 回合步骤
    EventPhase,          // 事件步骤
    ActionPhase,         // 行动步骤
    BombardmenPhaset,    // 轰炸步骤
    ReinforcementPhase,  // 增援步骤
    ProductionPhase,     // 生产步骤
    NationalPowerPhase   // 国力步骤
};

enum class ActionStep
{
    LandMovement,        // 陆上移动
    LandCombat,          // 陆战
    Redeployment,        // 调动
    Preparation,         // 准备
    Supply               // 补给
};

bool parseCorpsFromPixPath(const QString& pixPath, Side& side, int& level);
bool parseFortressFromPixPath(const QString& pixPath, Side& side, int& level);
bool parseFortificationFromPixPath(const QString& pixPath, Side& side, int& level);
QString corpsPixPath(Side side, int level);
