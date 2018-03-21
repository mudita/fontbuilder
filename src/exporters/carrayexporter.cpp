#include "carrayexporter.h"
#include "../fontconfig.h"

CArrayExporter::CArrayExporter(QObject *parent) :
    AbstractExporter(parent)
{
    setExtension("h");
}

bool CArrayExporter::Export(QByteArray &out)
{
    QString filename = texFilename();
    int _pos = filename.indexOf('.');
    filename.truncate(_pos);
    int charsnum = 0;
    int kernnum = 0;

    const FontConfig* cfg = fontConfig();
/*
    QString tmp = QString("%1%2").arg(cfg->bold() ? "_bold" : "").arg(cfg->italic() ? "_italic" : "");
        if (!cfg->bold() && !cfg->italic())
            tmp = QString("_regular");
*/
    out.append( QString("#ifndef ") + QString("%1%2_%3_H\n").arg(cfg->family()).arg(filename).arg(cfg->size()).toUpper().toUtf8());
    out.append( QString("#define ") + QString("%1%2_%3_H\n\n").arg(cfg->family()).arg(filename).arg(cfg->size()).toUpper().toUtf8());

    out.append( QString("#include \"font.h\"\n\n").toUtf8());

/*
 * struct font_info
 * {
 *      char face[];
 *      uint16_t size;
 *      uint8_t bold;
 *      uint8_t italic;
 *      uint8_t smooth;
 *      uint8_t charSpacing;
 *      uint8_t lineSpacing;
 * }
 */

    out.append( QString("#ifndef ") + QString("FI_%1\n").arg(filename).toUpper().toUtf8());
    out.append( QString("#define ") + QString("FI_%1 \\\n").arg(filename).toUpper().toUtf8());

    out.append( QString("\tstatic const struct font_info __attribute__((section(\".ExtFlash\"))) fi_%1 = { \\\n").arg(filename).toLower()
        + QString("\t\t.face=\"%1\", \\\n").arg(cfg->family())
        + QString("\t\t.size=%1, \\\n").arg(cfg->size())
        + QString("\t\t.bold=%1, \\\n").arg(cfg->bold() ? 1 : 0)
        + QString("\t\t.italic=%1, \\\n").arg(cfg->italic() ? 1 : 0)
        + QString("\t\t.smooth=%1, \\\n").arg(cfg->antialiased() ? 1 : 0)
        + QString("\t\t.charSpacing=%1, \\\n").arg(cfg->charSpacing())
        + QString("\t\t.lineSpacing=%1 \\\n};").arg(cfg->lineSpacing())
        .toUtf8()).append('\n');

    out.append( QString("#endif\n\n").toUtf8());
/*
 * struct font_common
 * {
 *      uint16_t lineHeight;
 *      uint16_t base;
 *      uint16_t scaleW;
 *      uint16_t scaleH;
 *      uint8_t pages;
 * }
 */
    out.append( QString("#ifndef ") + QString("FC_%1\n").arg(filename).toUpper().toUtf8());
        out.append( QString("#define ") + QString("FC_%1 \\\n").arg(filename).toUpper().toUtf8());

    out.append( QString("\tstatic const struct font_common __attribute__((section(\".ExtFlash\"))) fc_%1 = { \\\n").arg(filename).toLower()
        + QString("\t\t.lineHeight=%1, \\\n").arg(metrics().height)
        + QString("\t\t.base=%1, \\\n").arg(metrics().ascender)
        + QString("\t\t.scaleW=%1, \\\n").arg(texWidth())
        + QString("\t\t.scaleH=%1, \\\n").arg(texHeight())
        + QString("\t\t.pages=1 \\\n};")
        .toUtf8()).append('\n');
    out.append( QString("#endif\n\n").toUtf8());
/*
 * struct font_page
 * {
 *      uint8_t id;
 *      char file[];
 * }
 */
    out.append( QString("#ifndef ") + QString("FP_%1\n").arg(filename).toUpper().toUtf8());
        out.append( QString("#define ") + QString("FP_%1 \\\n").arg(filename).toUpper().toUtf8());

    out.append( QString("\tstatic const struct font_page __attribute__((section(\".ExtFlash\"))) fp_%1 = { \\\n").arg(filename).toLower()
        + QString("\t\t.id=%1, \\\n").arg(0)
        + QString("\t\t.file=\"%1\" \\\n};").arg(texFilename())
        .toUtf8()).append('\n');

    out.append( QString("#endif\n\n").toUtf8());
/*
 * struct font_char
 * {
 *      uint16_t id;
 *      uint16_t x;
 *      uint16_t y;
 *      uint16_t width;
 *      uint16_t height;
 *      uint16_t xoffset;
 *      uint16_t yofset;
 *      uint16_t xadvance;
 *      uint8_t page;
 * }
 */
    out.append( QString("#ifndef ") + QString("FCH_%1\n").arg(filename).toUpper().toUtf8());
        out.append( QString("#define ") + QString("FCH_%1 \\\n").arg(filename).toUpper().toUtf8());

    out.append( QString("\tstatic const struct font_char __attribute__((section(\".ExtFlash\"))) fch_%1[] = { \\\n").arg(filename).toLower().toUtf8());
    foreach(const Symbol& c , symbols()) {
        out.append( QString("\t\t{ ")
            + QString(".id=%1, ").arg(c.id)
            + QString(".x=%1, ").arg(c.placeX)
            + QString(".y=%1, ").arg(c.placeY)
            + QString(".width=%1, ").arg(c.placeW)
            + QString(".height=%1, ").arg(c.placeH)
            + QString(".xoffset=%1, ").arg(c.offsetX)
            + QString(".yoffset=%1, ").arg(metrics().ascender - c.offsetY)
            + QString(".xadvance=%1, ").arg(c.advance)
            + QString(".page=%1 }, /*%2*/ \\").arg(0).arg( QString(c.id) )
            .toUtf8()).append('\n');
        charsnum++;
    }
    out.append( QString("\t};\n").toUtf8());

    //out.append( QString("\tconst uint16_t ").toUtf8() + QString("%1_CHARSNUM").arg(filename).toUpper().toUtf8() + QString(" = sizeof(fch_%1)/sizeof(fch_%1[0]);\n").arg(filename).toUtf8());

    out.append( QString("#endif\n\n").toUtf8());
/*
 * struct font_kerning
 * {
 *      uint16_t first;
 *      uint16_t second;
 *      int16_t amount;
 * }
 */
    out.append( QString("#ifndef ") + QString("FK_%1\n").arg(filename).toUpper().toUtf8());
    out.append( QString("#define ") + QString("FK_%1 \\\n").arg(filename).toUpper().toUtf8());

    out.append( QString("\tstatic const struct font_kerning __attribute__((section(\".ExtFlash\"))) fk_%1[] = { \\\n").arg(filename).toUtf8());
    typedef QMap<uint,int>::ConstIterator Kerning;
    foreach(const Symbol& c , symbols()) {
        for (Kerning k = c.kerning.begin(); k != c.kerning.end(); k++) {
            out.append( QString("\t\t{ ")
                + QString(".first=%1, ").arg(c.id)
                + QString(".second=%1, ").arg(k.key())
                + QString(".amount=%1 }, \\").arg(k.value())
                .toUtf8()).append('\n');
            kernnum++;
        }
    }
    out.append( QString("\t};\n").toUtf8());
    //out.append( QString("\tconst uint16_t ").toUtf8() + QString("%1_KERNNUM").arg(filename).toUpper().toUtf8() + QString(" = sizeof(fk_%1)/sizeof(fk_%1[0]);\n").arg(filename).toUtf8());
    out.append( QString("#endif\n\n").toUtf8());

    /*
     * struct font
     * {
     *      struct *font_info;
     *      struct *font_commn;
     *      struct *font_page;
     *      struct *font_char;
     *      struct *font_kerning;
     *      uint8_t *font_data;
     *      uint16_t charsnum;
     *      uint16_t kernnum;
     * }
     *
     * const struct font NAME;
     */


    out.append( QString("#ifndef ") + QString("F_%1\n").arg(filename).toUpper().toUtf8());
    out.append( QString("#define ") + QString("F_%1 \\\n").arg(filename).toUpper().toUtf8());

    out.append( QString("const struct font __attribute__((section(\".ExtFlash\"))) f_%1 = { \\\n").arg(filename).toLower().toUtf8());
    out.append( QString("\t.fi=&fi_%1, \\\n").arg(filename).toLower()
            + QString("\t.fc=&fc_%1, \\\n").arg(filename).toLower()
            + QString("\t.fp=&fp_%1, \\\n").arg(filename).toLower()
            + QString("\t.fch=fch_%1, \\\n").arg(filename).toLower()
            + QString("\t.fk=fk_%1, \\\n").arg(filename).toLower()
            + QString("\t.fd=font_data_%1, \\\n").arg(filename)
            + QString("\t.charsnum=%1, \\\n").arg(charsnum)
            + QString("\t.kernnum=%1 \\").arg(kernnum)
            .toUtf8()).append('\n');
    out.append( QString("};\n\n").toUtf8());

    out.append( QString("#endif\n\n").toUtf8());

    out.append( QString("extern const struct font f_%1;\n").arg(filename).toLower().toUtf8());
    //out.append( QString("extern const uint16_t ").toUtf8() + QString("%1_CHARSNUM;\n\n").arg(filename).toUpper().toUtf8());
    //out.append( QString("extern const uint16_t ").toUtf8() + QString("%1_KERNNUM;\n\n").arg(filename).toUpper().toUtf8());

    out.append( QString("#ifndef ") + QString("%1\n").arg(filename).toUpper().toUtf8());
    out.append( QString("#define ") + QString("%1 \\\n").arg(filename).toUpper().toUtf8());

    out.append( QString("\tFI_%1 \\\n").arg(filename).toUpper().toUtf8());
    out.append( QString("\tFC_%1 \\\n").arg(filename).toUpper().toUtf8());
    out.append( QString("\tFP_%1 \\\n").arg(filename).toUpper().toUtf8());
    out.append( QString("\tFCH_%1 \\\n").arg(filename).toUpper().toUtf8());
    out.append( QString("\tFK_%1 \\\n").arg(filename).toUpper().toUtf8());
    out.append( QString("\tF_%1 \n").arg(filename).toUpper().toUtf8());

    out.append( QString("#endif\n\n").toUtf8());
    out.append( QString("#endif\n").toUtf8());

    return true;
}

AbstractExporter* CArrayExporterFactoryFunc (QObject* parent) {
    return new CArrayExporter(parent);
}
