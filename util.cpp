#include "util.h"
#include <QRegularExpression>
#include <QImage>
#include <QColor>
#include <QtMath>
#include <QRandomGenerator>

bool parseCorpsFromPixPath(const QString& pixPath, Side& side, int& level)
{
    QRegularExpression re(R"(^:/(D|S|L)/\1_(\d)JBT\.png$)");
    auto m = re.match(pixPath);
    if (!m.hasMatch()) return false;

    const QString s = m.captured(1);
    level = m.captured(2).toInt();

    if (s == "D") side = Side::D;
    else if (s == "S") side = Side::S;

    return level >= 1 && level <= 4;
}

bool parseFortressFromPixPath(const QString& pixPath, Side& side, int& level)
{
    QRegularExpression re(R"(^:/(D|S|L)/\1_(\d)JYS\.png$)");
    auto m = re.match(pixPath);
    if (!m.hasMatch()) return false;

    const QString s = m.captured(1);
    level = m.captured(2).toInt();

    if (s == "D") side = Side::D;
    else if (s == "S") side = Side::S;

    return level >= 1 && level <= 4;
}

bool parseFortificationFromPixPath(const QString& pixPath, Side& side, int& level)
{
    QRegularExpression re(R"(^:/(D|S|L)/\1_(\d)JFYGS\.png$)");
    auto m = re.match(pixPath);
    if (!m.hasMatch()) return false;

    const QString s = m.captured(1);
    level = m.captured(2).toInt();

    if (s == "D") side = Side::D;
    else if (s == "S") side = Side::S;

    return level >= 1 && level <= 4;
}

QString corpsPixPath(Side side, int level)
{
    const char* s = (side==Side::D ? "D" : "S");
    return QString(":/%1/%1_%2JBT.png").arg(s).arg(level);
}
