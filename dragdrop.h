#pragma once

#include <QString>
#include <QStringList>
#include <QMimeData>

namespace DragDrop {

inline constexpr const char* MimePiece      = "application/x-piece";
inline constexpr const char* MimeEventPiece = "application/x-event-piece";

inline QString packEventPiece(const QString& eventId, const QString& pixPath) {
    return eventId + "|" + pixPath;
}

inline bool unpackEventPiece(const QString& payload, QString& eventId, QString& pixPath) {
    const auto parts = payload.split('|');
    if (parts.size() != 2) return false;
    eventId = parts[0];
    pixPath = parts[1];
    return true;
}

}
