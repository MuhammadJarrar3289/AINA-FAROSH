#include "FontInspector.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H
#include <QDebug>

bool FontInspector::canEmbedFont(const QString &fontPath, QString &reason) {
    FT_Library library;
    FT_Face face;
    FT_Error err = FT_Init_FreeType(&library);
    if (err) {
        reason = "Failed to init FreeType";
        return false;
    }

    QByteArray pathUtf8 = fontPath.toUtf8();
    err = FT_New_Face(library, pathUtf8.constData(), 0, &face);
    if (err) {
        reason = "Failed to open font with FreeType";
        FT_Done_FreeType(library);
        return false;
    }

    // Try to get OS/2 table
    TT_OS2 *os2 = (TT_OS2 *)FT_Get_Sfnt_Table(face, ft_sfnt_os2);
    if (!os2) {
        // No OS/2 table — assume embedding is allowed (safe fallback)
        reason = "No OS/2 table found; assuming embedding allowed";
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return true;
    }

    uint16_t fsType = os2->fsType;
    // fsType bit 1 (0x0002) == restricted license embedding (embedding disabled)
    if (fsType & 0x0002) {
        reason = "fsType indicates Restricted License embedding — embedding not allowed";
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return false;
    }

    // Otherwise embedding is allowed (preview & print or editable are fine)
    reason = "Embedding allowed per OS/2 fsType";
    FT_Done_Face(face);
    FT_Done_FreeType(library);
    return true;
}
