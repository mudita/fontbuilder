/**
 * Copyright (c) 2010-2010 Andrey AndryBlack Kunitsyn
 * email:support.andryblack@gmail.com
 *
 * Report bugs and download new versions at http://code.google.com/p/fontbuilder
 *
 * This software is distributed under the MIT License.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "carraywriter.h"

#include "builtinimagewriter.h"
#include "layoutdata.h"
#include <QImage>
#include <QPainter>
#include <QFileInfo>
#include "../layoutconfig.h"

#include <QDebug>

CArrayImageWriter::CArrayImageWriter(QString ext,QObject *parent) :
    AbstractImageWriter(parent)
{
    setExtension(ext);
    setReloadSupport(true);
}

/*
 * uint8_t font_data[height][width] =
 * {
 *      {...},
 *      ...,
 *      {...}
 * };
 */

bool CArrayImageWriter::Export(QFile& file) {
    QImage pixmap = buildImage();

    QTextStream out(&file);
    QFileInfo fno = QFileInfo(file);

    out << QString("#include \"%1.h\"\n\n").arg(fno.baseName()).toUtf8();

    out << QString("static const uint8_t font_data_%1[%2] = {\n").arg(fno.baseName()).arg(pixmap.height() * pixmap.width() / 2).toUtf8();

    for (int y=0;y<pixmap.height();y++) {
        for (int x = 0; x < pixmap.width() * 4; x+=8)
        {
            uint8_t val = ((pixmap.scanLine(y)[x+7] / 17) << 4) | (pixmap.scanLine(y)[x+3] / 17);
            out << QString("0x%1, ").arg(val, 2, 16, QLatin1Char('0')).toUtf8();
        }
        out << QString("\n");
    }

    out << QString("};\n").toUtf8();

    out << QString("\n%1").arg(fno.baseName()).toUpper().toUtf8();
    //out << QString("\nconst uint16_t ").toUtf8() << QString("%1_SIZE").arg(fno.baseName()).toUpper().toUtf8() << QString(" = sizeof(font_data_%1)/sizeof(font_data_%1[0]);\n").arg(fno.baseName()).toUtf8();

    return true;
}

QImage* CArrayImageWriter::reload(QFile& file) {
    QImage* img = 0;

    return img;
}
