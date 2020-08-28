#ifndef MPCOMMON_H
#define MPCOMMON_H

#include <QByteArray>
#include <QFile>
#include <stdint.h>

typedef uint32_t ucode32;

class FontGlyph
{
public:
    //character id
    ucode32 id;
    //offset in glyph data field
    uint32_t glyph_offset;
    //width of the character image in the texture
    uint16_t width;
    //height of the character image in the texture
    uint16_t height;
    //how much the current position should be offset when copying the image from the texture to the screen
    int16_t xoffset;
    //how much the current position should be offset when copying the image from the texture to the screen
    int16_t yofset;
    //how much the current position should be advanced after drawing the character
    uint16_t xadvance;

    void load( QFile& file) {
        file.read( reinterpret_cast<char*>(&id), sizeof(ucode32));
        file.read( reinterpret_cast<char*>(&glyph_offset), sizeof(uint32_t));
        file.read( reinterpret_cast<char*>(&width), sizeof(uint16_t));
        file.read( reinterpret_cast<char*>(&height), sizeof(uint16_t));
        file.read( reinterpret_cast<char*>(&xoffset), sizeof(uint16_t));
        file.read( reinterpret_cast<char*>(&yofset), sizeof(uint16_t));
        file.read( reinterpret_cast<char*>(&xadvance), sizeof(uint16_t));
    }

    void save( QByteArray& out ) {
        out.append( reinterpret_cast<char*>(&id), sizeof(ucode32));
        out.append( reinterpret_cast<char*>(&glyph_offset), sizeof(uint32_t));
        out.append( reinterpret_cast<char*>(&width), sizeof(uint16_t));
        out.append( reinterpret_cast<char*>(&height), sizeof(uint16_t));
        out.append( reinterpret_cast<char*>(&xoffset), sizeof(uint16_t));
        out.append( reinterpret_cast<char*>(&yofset), sizeof(uint16_t));
        out.append( reinterpret_cast<char*>(&xadvance), sizeof(uint16_t));
    }

    static uint32_t getSize() {
        return sizeof (ucode32) + sizeof(uint32_t) + 5*sizeof (int16_t);
    }
};

class FontKerning
{
public:
    //utf16 id of the first character
    ucode32 first;
    //utf16 id of the following character
    ucode32 second;
    //distance in pixels between beginning of first character and beginning of second character
    int16_t amount;

    void load( QFile& file) {
        file.read( reinterpret_cast<char*>(&first), sizeof(ucode32));
        file.read( reinterpret_cast<char*>(&second), sizeof(ucode32));
        file.read( reinterpret_cast<char*>(&amount), sizeof(uint16_t));
    }

    void save( QByteArray& out ) {
        out.append( reinterpret_cast<char*>(&first), sizeof(ucode32));
        out.append( reinterpret_cast<char*>(&second), sizeof(ucode32));
        out.append( reinterpret_cast<char*>(&amount), sizeof(int16_t));
    }

    static uint32_t getSize() {
        return 2 * sizeof(ucode32) + sizeof (uint16_t);
    }
};

class FontInfo
{
public:
    //name of the true type font. max 63 characters
    char face[64];
    //size of the true type font
    uint16_t size;
    //flag that informs if font is bold
    uint16_t bold;
    //flag that informs if font is italic
    uint16_t italic;
    //flag that informs if smoothing was turned on. 1 - smoothing was turned on.
    uint16_t smooth;
    //TODO additional space between characters????
    uint16_t char_spacing;
    //TODO additional space between lines
    uint16_t line_spacing;
    //distance in pixels between each line of text
    uint16_t line_height;
    //number of pixels from the absolute top of the line to the base of the characters
    uint16_t base;
    //width of the texture, normally used to scale the x pos of the character image
    uint16_t scale_w;
    //height of the texture, normally used to scale the y pos of the character image
    uint16_t scale_h;

    void save( QByteArray& out ) {
        out.append( reinterpret_cast<char*>(face), sizeof(face));
        out.append( reinterpret_cast<char*>(&size), sizeof(uint16_t));
        out.append( reinterpret_cast<char*>(&bold), sizeof(uint16_t));
        out.append( reinterpret_cast<char*>(&italic), sizeof(uint16_t));
        out.append( reinterpret_cast<char*>(&smooth), sizeof(uint16_t));
        out.append( reinterpret_cast<char*>(&char_spacing), sizeof(uint16_t));
        out.append( reinterpret_cast<char*>(&line_spacing), sizeof(uint16_t));
        out.append( reinterpret_cast<char*>(&line_height), sizeof(uint16_t));
        out.append( reinterpret_cast<char*>(&base), sizeof(uint16_t));
        out.append( reinterpret_cast<char*>(&scale_w), sizeof(uint16_t));
        out.append( reinterpret_cast<char*>(&scale_h), sizeof(uint16_t));
    }

    void load( QFile& file) {
        file.read( reinterpret_cast<char*>(face), sizeof(face));
        file.read( reinterpret_cast<char*>(&size), sizeof(uint16_t));
        file.read( reinterpret_cast<char*>(&bold), sizeof(uint16_t));
        file.read( reinterpret_cast<char*>(&italic), sizeof(uint16_t));
        file.read( reinterpret_cast<char*>(&smooth), sizeof(uint16_t));
        file.read( reinterpret_cast<char*>(&char_spacing), sizeof(uint16_t));
        file.read( reinterpret_cast<char*>(&line_spacing), sizeof(uint16_t));
        file.read( reinterpret_cast<char*>(&line_height), sizeof(uint16_t));
        file.read( reinterpret_cast<char*>(&base), sizeof(uint16_t));
        file.read( reinterpret_cast<char*>(&scale_w), sizeof(uint16_t));
        file.read( reinterpret_cast<char*>(&scale_h), sizeof(uint16_t));
    }

    static uint32_t getSize() {
        return sizeof(face) + 10*sizeof (uint16_t);
    }
};

class Font
{
public:
    //structure holding detailed information about font
    FontInfo info;
    //number of glyphs in the font
    uint32_t glyph_count;
    //offset to the beginning of the glyph data
    uint32_t glyph_data_offset;
    //number of kerning pairs
    uint32_t kern_count;
    //array of glyphs structures
    uint32_t kern_data_offset;
    //offset to the beginning of the image data
    uint32_t image_data_offset;

    void save( QByteArray& out ) {
        info.save(out);
        out.append( reinterpret_cast<char*>(&glyph_count), sizeof(uint32_t));
        out.append( reinterpret_cast<char*>(&glyph_data_offset), sizeof(uint32_t));
        out.append( reinterpret_cast<char*>(&kern_count), sizeof(uint32_t));
        out.append( reinterpret_cast<char*>(&kern_data_offset), sizeof(uint32_t));
        out.append( reinterpret_cast<char*>(&image_data_offset), sizeof(uint32_t));
    }

    void load( QFile& file) {
        info.load( file );
        file.read( reinterpret_cast<char*>(&glyph_count), sizeof(uint32_t));
        file.read( reinterpret_cast<char*>(&glyph_data_offset), sizeof(uint32_t));
        file.read( reinterpret_cast<char*>(&kern_count), sizeof(uint32_t));
        file.read( reinterpret_cast<char*>(&kern_data_offset), sizeof(uint32_t));
        file.read( reinterpret_cast<char*>(&image_data_offset), sizeof(uint32_t));
    }

    static uint32_t getSize() {
        return FontInfo::getSize() + 5*sizeof (uint32_t);
    }
};

//this data is to be overwrittne by font's glyphs. It contains x and y coordinates of the following glyphs
typedef struct {
    uint32_t x;
    uint32_t y;
} pixmap_coords_t;

#endif // MPCOMMON_H
