#ifndef MPCOMMON_H
#define MPCOMMON_H

#include <stdint.h>

typedef struct
{
    //character id
    uint16_t id;
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
}gui_font_glyph_t;

typedef struct
{
    //utf16 id of the first character
    uint16_t first;
    //utf16 id of the following character
    uint16_t second;
    //distance in pixels between beginning of first character and beginning of second character
    int16_t amount;
} gui_font_kerning_t;

typedef struct
{
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

}gui_font_info_t;

typedef struct
{
    //structure holding detailed information about font
    gui_font_info_t info;
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
} gui_font_img_t;

//this data is to be overwrittne by font's glyphs. It contains x and y coordinates of the following glyphs
typedef struct {
    uint32_t x;
    uint32_t y;
} pixmap_coords_t;

#endif // MPCOMMON_H
