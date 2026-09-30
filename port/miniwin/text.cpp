/*
 * Fonts and text: the game's TrueType font (CornerStone, CORNER.TTF),
 * rendered with stb_truetype, unhinted and without antialiasing (a 256-colour
 * display's text had none). Fonts come from files AddFontResource adds and
 * from C:\WINDOWS\FONTS; a face that isn't there (the system font) is drawn
 * in the first one found.
 *
 * Layout follows GDI: a TrueType font's cell is its Windows ascent and
 * descent (from the OS/2 table), a negative height asks for the em, bold is
 * made by drawing twice a pixel apart, and characters are Windows-1252.
 */

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "stb_truetype.h"

#include "miniwin/internal.h"

namespace miniwin {

void plot(DC *dc, int x, int y, uint8_t value, int rop2);
bool isDrawable(DC *dc);

struct FontFace
{
    std::string name; /* the family */
    std::string path;
    std::vector<unsigned char> data;
    stbtt_fontinfo info;
    int unitsPerEm;
    int winAscent, winDescent;
    int lineGap;
};

static std::vector<std::unique_ptr<FontFace>> faces;

static int readU16(const unsigned char *p)
{
    return p[0] << 8 | p[1];
}

/* The family name (name ID 1) from the 'name' table, Windows or Mac. */
static std::string familyName(const stbtt_fontinfo &info)
{
    int length = 0;
    const char *name = stbtt_GetFontNameString(&info, &length, STBTT_PLATFORM_ID_MICROSOFT,
                                               STBTT_MS_EID_UNICODE_BMP, STBTT_MS_LANG_ENGLISH, 1);
    std::string result;
    if (name) {
        for (int i = 0; i + 1 < length; i += 2)
            result += (char)name[i + 1]; /* UTF-16BE, ASCII here */
        return result;
    }
    name = stbtt_GetFontNameString(&info, &length, STBTT_PLATFORM_ID_MAC, 0, 0, 1);
    if (name)
        result.assign(name, (size_t)length);
    return result;
}

static FontFace *loadFace(const std::string &host)
{
    for (auto &face : faces)
        if (face->path == host)
            return face.get();
    FILE *file = ::fopen(host.c_str(), "rb");
    if (!file)
        return 0;
    auto face = std::make_unique<FontFace>();
    fseek(file, 0, SEEK_END);
    face->data.resize((size_t)ftell(file));
    fseek(file, 0, SEEK_SET);
    size_t read = fread(face->data.data(), 1, face->data.size(), file);
    fclose(file);
    if (read != face->data.size()
        || !stbtt_InitFont(&face->info, face->data.data(),
                           stbtt_GetFontOffsetForIndex(face->data.data(), 0)))
        return 0;
    face->path = host;
    face->name = familyName(face->info);
    const unsigned char *head = face->data.data() + stbtt__find_table(face->data.data(), face->info.fontstart, "head");
    face->unitsPerEm = readU16(head + 18);
    int ascent, descent, gap;
    stbtt_GetFontVMetrics(&face->info, &ascent, &descent, &gap);
    face->winAscent = ascent;
    face->winDescent = -descent;
    face->lineGap = gap;
    unsigned os2 = stbtt__find_table(face->data.data(), face->info.fontstart, "OS/2");
    if (os2) {
        face->winAscent = readU16(face->data.data() + os2 + 74);
        face->winDescent = readU16(face->data.data() + os2 + 76);
    }
    trace("font \"%s\" from %s", face->name.c_str(), host.c_str());
    faces.push_back(std::move(face));
    return faces.back().get();
}

bool addFontFile(const char *path)
{
    std::string host;
    return hostPath(path, host) && loadFace(host);
}

void removeFontFile(const char *)
{
    /* Faces stay loaded: DCs may still use them. */
}

void loadSystemFonts()
{
    std::string host;
    std::error_code error;

    if (!hostPath("C:\\WINDOWS\\FONTS", host))
        return;
    for (const auto &entry : std::filesystem::directory_iterator(host, error)) {
        std::string extension = entry.path().extension().string();
        if (!stricmp(extension.c_str(), ".ttf"))
            loadFace(entry.path().string());
    }
}

FontFace *findFace(const char *name)
{
    for (auto &face : faces)
        if (!stricmp(face->name.c_str(), name))
            return face.get();
    return faces.empty() ? 0 : faces[0].get();
}

/* A .FOT file names its TrueType file (as Windows' do, in their own way). */
BOOL CreateScalableFontResource(DWORD, LPCSTR resource, LPCSTR fontFile, LPCSTR path)
{
    std::string host;
    std::string ttf = fontFile;

    if (path && *path)
        ttf = std::string(path) + "\\" + fontFile;
    if (!hostPath(resource, host))
        return FALSE;
    FILE *file = ::fopen(host.c_str(), "w");
    if (!file)
        return FALSE;
    char full[MAX_PATH];
    std::string resolved;
    if (ttf.size() < 2 || ttf[1] != ':') {
        GetCurrentDirectory(sizeof full, full);
        ttf = std::string(full) + "\\" + ttf;
    }
    fprintf(file, "miniwin font resource\n%s\n", ttf.c_str());
    fclose(file);
    return TRUE;
}

int AddFontResource(LPCSTR file)
{
    std::string host;

    if (!hostPath(file, host))
        return 0;
    FILE *f = ::fopen(host.c_str(), "rb");
    if (!f)
        return 0;
    char line[MAX_PATH + 32];
    bool fot = fgets(line, sizeof line, f) && !strcmp(line, "miniwin font resource\n");
    std::string target = file;
    if (fot && fgets(line, sizeof line, f)) {
        line[strcspn(line, "\r\n")] = 0;
        target = line;
    }
    fclose(f);
    if (!addFontFile(target.c_str()))
        return 0;
    return 1;
}

BOOL RemoveFontResource(LPCSTR file)
{
    removeFontFile(file);
    return TRUE;
}

/* A font as drawn in a DC: its face and pixel scale. */
struct Metrics
{
    FontFace *face;
    float scale;
    int ascent, descent, height, internalLeading, externalLeading;
    bool bold, italic, underline;
};

static int yScale(DC *dc, int value)
{
    LONG a = 0, b = value, zero = 0, z = 0;
    toDevice(dc, a, b);
    toDevice(dc, zero, z);
    int scaled = abs((int)(b - z));
    return scaled ? scaled : (value ? 1 : 0);
}

static bool metricsOf(DC *dc, Metrics &m)
{
    Font *font = dc->font;
    const LOGFONT &lf = font->logical;

    if (!font->face)
        font->face = findFace(lf.lfFaceName);
    m.face = font->face;
    if (!m.face)
        return false;
    int height = lf.lfHeight ? yScale(dc, abs((int)lf.lfHeight)) : 16;
    int cell = m.face->winAscent + m.face->winDescent;
    if (lf.lfHeight < 0)
        m.scale = (float)height / m.face->unitsPerEm;
    else
        m.scale = (float)height / cell;
    m.ascent = (int)lroundf(m.face->winAscent * m.scale);
    m.descent = (int)lroundf(m.face->winDescent * m.scale);
    m.height = m.ascent + m.descent;
    m.internalLeading = m.height - (int)lroundf(m.face->unitsPerEm * m.scale);
    if (m.internalLeading < 0)
        m.internalLeading = 0;
    m.externalLeading = (int)lroundf(m.face->lineGap * m.scale);
    m.bold = lf.lfWeight >= 600;
    m.italic = lf.lfItalic != 0;
    m.underline = lf.lfUnderline != 0;
    if (lf.lfEscapement)
        unsupported("rotated text");
    return true;
}

/* Windows-1252 to Unicode. */
static int codepoint(unsigned char c)
{
    static const unsigned short high[32] = {
        0x20ac, 0x81, 0x201a, 0x192, 0x201e, 0x2026, 0x2020, 0x2021, 0x2c6, 0x2030, 0x160,
        0x2039, 0x152, 0x8d, 0x17d, 0x8f, 0x90, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022,
        0x2013, 0x2014, 0x2dc, 0x2122, 0x161, 0x203a, 0x153, 0x9d, 0x17e, 0x178,
    };
    return c >= 0x80 && c < 0xa0 ? high[c - 0x80] : c;
}

static int advanceOf(const Metrics &m, unsigned char c)
{
    int advance, bearing;
    stbtt_GetCodepointHMetrics(&m.face->info, codepoint(c), &advance, &bearing);
    return (int)lroundf(advance * m.scale) + (m.bold ? 1 : 0);
}

static int widthOf(const Metrics &m, const char *text, int count)
{
    int width = 0;
    for (int i = 0; i < count; i++)
        width += advanceOf(m, (unsigned char)text[i]);
    return width;
}

BOOL GetTextMetrics(HDC handle, LPTEXTMETRIC tm)
{
    DC *dc = dcOf(handle);
    Metrics m;

    memset(tm, 0, sizeof *tm);
    if (!dc || !metricsOf(dc, m)) {
        tm->tmHeight = 16;
        tm->tmAscent = 13;
        tm->tmDescent = 3;
        tm->tmAveCharWidth = 7;
        tm->tmMaxCharWidth = 14;
        tm->tmWeight = FW_BOLD;
        return dc != 0;
    }
    tm->tmHeight = m.height;
    tm->tmAscent = m.ascent;
    tm->tmDescent = m.descent;
    tm->tmInternalLeading = m.internalLeading;
    tm->tmExternalLeading = m.externalLeading;
    tm->tmAveCharWidth = advanceOf(m, 'x');
    int widest = 0;
    for (int c = 32; c < 256; c++)
        if (advanceOf(m, (unsigned char)c) > widest)
            widest = advanceOf(m, (unsigned char)c);
    tm->tmMaxCharWidth = widest;
    tm->tmWeight = dc->font->logical.lfWeight ? dc->font->logical.lfWeight : FW_NORMAL;
    tm->tmOverhang = m.bold ? 1 : 0;
    tm->tmDigitizedAspectX = tm->tmDigitizedAspectY = 96;
    tm->tmFirstChar = 32;
    tm->tmLastChar = 255;
    tm->tmDefaultChar = 31;
    tm->tmBreakChar = 32;
    tm->tmItalic = m.italic;
    tm->tmUnderlined = m.underline;
    tm->tmPitchAndFamily = 0x06; /* variable pitch, TrueType */
    tm->tmCharSet = ANSI_CHARSET;
    return TRUE;
}

BOOL GetTextExtentPoint(HDC handle, LPCSTR text, int count, LPSIZE size)
{
    DC *dc = dcOf(handle);
    Metrics m;

    size->cx = size->cy = 0;
    if (!dc || !metricsOf(dc, m))
        return FALSE;
    size->cx = widthOf(m, text, count);
    size->cy = m.height;
    return TRUE;
}

/* Draws a line of text with its cell's top left at device (x, y). */
static void drawLine(DC *dc, const Metrics &m, int x, int y, const char *text, int count,
                     const std::vector<RECT> &clips)
{
    uint8_t color = pixelFor(dc, dc->textColor);
    int width = widthOf(m, text, count);

    if (dc->bkMode == OPAQUE) {
        uint8_t back = pixelFor(dc, dc->bkColor);
        for (RECT clip : clips) {
            RECT cell = {x, y, x + width, y + m.height};
            if (intersect(cell, clip))
                for (LONG py = cell.top; py < cell.bottom; py++)
                    for (LONG px = cell.left; px < cell.right; px++)
                        plot(dc, px + dc->origin.x, py + dc->origin.y, back, R2_COPYPEN);
        }
    }
    int baseline = y + m.ascent;
    int penX = x;
    for (int i = 0; i < count; i++) {
        int c = codepoint((unsigned char)text[i]);
        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&m.face->info, c, m.scale, m.scale, &x0, &y0, &x1, &y1);
        int w = x1 - x0, h = y1 - y0;
        if (w > 0 && h > 0) {
            std::vector<unsigned char> coverage((size_t)(w * h));
            stbtt_MakeCodepointBitmap(&m.face->info, coverage.data(), w, h, w, m.scale, m.scale, c);
            for (int passes = m.bold ? 2 : 1, pass = 0; pass < passes; pass++)
                for (int gy = 0; gy < h; gy++) {
                    int py = baseline + y0 + gy;
                    int shear = m.italic ? (m.ascent - (py - y)) / 3 : 0;
                    for (int gx = 0; gx < w; gx++) {
                        if (coverage[(size_t)(gy * w + gx)] < 128)
                            continue;
                        int px = penX + x0 + gx + pass + shear;
                        for (const RECT &clip : clips)
                            if (px >= clip.left && px < clip.right && py >= clip.top && py < clip.bottom) {
                                plot(dc, px + dc->origin.x, py + dc->origin.y, color, R2_COPYPEN);
                                break;
                            }
                    }
                }
        }
        penX += advanceOf(m, (unsigned char)text[i]);
    }
    if (m.underline) {
        int py = baseline + 1 + m.descent / 3;
        for (int px = x; px < x + width; px++)
            for (const RECT &clip : clips)
                if (px >= clip.left && px < clip.right && py >= clip.top && py < clip.bottom) {
                    plot(dc, px + dc->origin.x, py + dc->origin.y, color, R2_COPYPEN);
                    break;
                }
    }
    if (dc->surface->device)
        screenChanged();
}

/* Splits text into lines, word-wrapped to `width` when asked. */
static std::vector<std::string> layout(const Metrics &m, const std::string &text, UINT format,
                                       int width)
{
    std::vector<std::string> lines;
    std::vector<std::string> paragraphs;

    if (format & DT_SINGLELINE)
        paragraphs.push_back(text);
    else {
        std::string current;
        for (size_t i = 0; i < text.size(); i++) {
            if (text[i] == '\r' && i + 1 < text.size() && text[i + 1] == '\n')
                continue;
            if (text[i] == '\n' || text[i] == '\r') {
                paragraphs.push_back(current);
                current.clear();
            } else
                current += text[i];
        }
        paragraphs.push_back(current);
    }
    for (const std::string &paragraph : paragraphs) {
        if (!(format & DT_WORDBREAK) || (format & DT_SINGLELINE)) {
            lines.push_back(paragraph);
            continue;
        }
        size_t start = 0;
        while (start <= paragraph.size()) {
            size_t end = start, lastBreak = std::string::npos;
            while (end < paragraph.size()) {
                if (paragraph[end] == ' ')
                    lastBreak = end;
                if (widthOf(m, paragraph.data() + start, (int)(end - start + 1)) > width
                    && end > start)
                    break;
                end++;
            }
            if (end >= paragraph.size()) {
                lines.push_back(paragraph.substr(start));
                break;
            }
            size_t cut = lastBreak != std::string::npos && lastBreak > start ? lastBreak : end;
            std::string line = paragraph.substr(start, cut - start);
            while (!line.empty() && line.back() == ' ')
                line.pop_back();
            lines.push_back(line);
            start = cut;
            while (start < paragraph.size() && paragraph[start] == ' ')
                start++;
            if (start >= paragraph.size())
                break;
        }
    }
    return lines;
}

/* Without DT_NOPREFIX, & marks the next character (&& is an &). */
static std::string withoutPrefixes(const std::string &text)
{
    std::string result;
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '&' && i + 1 < text.size())
            i++;
        result += text[i];
    }
    return result;
}

int DrawText(HDC handle, LPCSTR text, int count, LPRECT rect, UINT format)
{
    DC *dc = dcOf(handle);
    Metrics m;

    if (!dc || !metricsOf(dc, m))
        return 0;
    /* Windows 95's DrawText is 16-bit underneath: a count of 0xffff is -1,
       the text up to its NUL (the game passes that). */
    if ((count & 0xffff) == 0xffff)
        count = -1;
    std::string content(text, count < 0 ? strlen(text) : strnlen(text, (size_t)count));
    if (!(format & DT_NOPREFIX))
        content = withoutPrefixes(content);
    RECT device = toDeviceRect(dc, rect->left, rect->top, rect->right, rect->bottom);
    int lineHeight = m.height + (format & DT_EXTERNALLEADING ? m.externalLeading : 0);
    std::vector<std::string> lines = layout(m, content, format, device.right - device.left);
    int height = lineHeight * (int)lines.size();

    if (format & DT_CALCRECT) {
        int widest = 0;
        for (const std::string &line : lines)
            widest = std::max(widest, widthOf(m, line.data(), (int)line.size()));
        LONG right = device.left + ((format & DT_WORDBREAK) ? device.right - device.left : widest);
        LONG bottom = device.top + height;
        LONG left = device.left, top = device.top;
        toLogical(dc, left, top);
        toLogical(dc, right, bottom);
        rect->right = right;
        rect->bottom = bottom;
        LONG zero = 0, h = height, z = 0, zz = 0;
        toLogical(dc, zero, h);
        toLogical(dc, z, zz);
        return (int)(h - zz);
    }

    std::vector<RECT> clips = clipRects(dc);
    if (!(format & DT_NOCLIP)) {
        std::vector<RECT> within;
        for (RECT clip : clips)
            if (intersect(clip, device))
                within.push_back(clip);
        clips = within;
    }
    int y = device.top;
    if (format & DT_SINGLELINE) {
        if (format & DT_VCENTER)
            y = device.top + (device.bottom - device.top - m.height) / 2;
        else if (format & DT_BOTTOM)
            y = device.bottom - m.height;
    }
    if (isDrawable(dc))
        for (const std::string &line : lines) {
            int width = widthOf(m, line.data(), (int)line.size());
            int x = device.left;
            if (format & DT_CENTER)
                x = device.left + (device.right - device.left - width) / 2;
            else if (format & DT_RIGHT)
                x = device.right - width;
            drawLine(dc, m, x, y, line.data(), (int)line.size(), clips);
            y += lineHeight;
        }
    LONG zero = 0, h = height, z = 0, zz = 0;
    toLogical(dc, zero, h);
    toLogical(dc, z, zz);
    return (int)(h - zz);
}

} /* namespace miniwin */
