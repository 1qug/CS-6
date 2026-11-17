#pragma once

#include <QString>

// Product metadata. Keep every user-visible product name here so that the
// window title, About dialog, settings and logs cannot drift apart.
namespace AppInfo {

inline QString shortName()
{
    return QStringLiteral("CS-6");
}

inline QString productName()
{
    return QStringLiteral("CS-6 射频信号发生器");
}

inline QString productDescription()
{
    return QStringLiteral("CS-6 便携式射频信号发生器上位机");
}

inline QString organization()
{
    return QStringLiteral("CS");
}

inline QString version()
{
    return QStringLiteral("1.0.0");
}

} // namespace AppInfo
