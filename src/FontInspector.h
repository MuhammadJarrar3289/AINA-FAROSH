#pragma once
#include <QString>

class FontInspector {
public:
    // Returns true if embedding is allowed (per OS/2 fsType) or if the table is missing.
    // If check fails, returns false and sets reason.
    static bool canEmbedFont(const QString &fontPath, QString &reason);
};
