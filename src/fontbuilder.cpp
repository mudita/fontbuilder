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

#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QImage>
#include <QSettings>
#include <QMetaProperty>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "fontbuilder.h"
#include "ui_fontbuilder.h"
#include "fontconfig.h"
#include "fontrenderer.h"
#include "layoutconfig.h"
#include "layoutdata.h"
#include "layouterfactory.h"
#include "outputconfig.h"
#include "exporterfactory.h"
#include "imagewriterfactory.h"
#include "fontloader.h"


FontBuilder::FontBuilder(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FontBuilder),
    m_image_writer(nullptr)
{
    ui->setupUi(this);



    m_font_config = new FontConfig(this);
    bool font_config_block = m_font_config->blockSignals(true);
    connect(m_font_config,SIGNAL(nameChanged()),this,SLOT(onFontNameChanged()));
    connect(m_font_config,SIGNAL(sizeChanged()),this,SLOT(onFontNameChanged()));

    m_font_renderer = new FontRenderer(this,m_font_config);

    connect(m_font_renderer,SIGNAL(imagesChanged()),this,SLOT(onRenderedChanged()));

    m_layout_config = new LayoutConfig(this);
    m_layout_data = new LayoutData(this);

    connect(m_layout_data,SIGNAL(layoutChanged()),this,SLOT(onLayoutChanged()));

    m_layouter = nullptr;
    m_layouter_factory = new LayouterFactory(this);

    bool b = ui->comboBoxLayouter->blockSignals(true);
    ui->comboBoxLayouter->clear();
    ui->comboBoxLayouter->addItems(m_layouter_factory->names());

    m_output_config = new OutputConfig(this);

    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    readConfig(settings,"fontconfig",m_font_config);
    m_font_config->normalize();
    readConfig(settings,"layoutconfig",m_layout_config);
    readConfig(settings,"outputconfig",m_output_config);
    ui->checkBoxDrawGrid->setChecked(settings.value("draw_grid").toBool());
    ui->widgetFontPreview->setDrawGrid(ui->checkBoxDrawGrid->isChecked());
    connect(ui->checkBoxDrawGrid,SIGNAL(toggled(bool)),this,SLOT(on_checkBoxDrawGrid_toggled(bool)));

    ui->frameCharacters->setConfig(m_font_config);
    ui->frameFontOptions->setConfig(m_font_config);
    if (!m_layout_config->layouter().isEmpty()) {
        for (int i=0;i<ui->comboBoxLayouter->count();i++)
            if (ui->comboBoxLayouter->itemText(i)==m_layout_config->layouter())
                ui->comboBoxLayouter->setCurrentIndex(i);
    }
    ui->frameLayoutConfig->setConfig(m_layout_config);


    m_exporter_factory = new ExporterFactory(this);
    ui->frameOutput->setExporters(m_exporter_factory->names());

    m_image_writer_factory = new ImageWriterFactory(this);
    ui->frameOutput->setImageWriters(m_image_writer_factory->names());

    ui->comboBoxLayouter->blockSignals(b);
    this->on_comboBoxLayouter_currentIndexChanged(
            ui->comboBoxLayouter->currentText());

    ui->frameOutput->setConfig(m_output_config);
    ui->frameFontSelect->setConfig(m_font_config);

    ui->fontTestFrame->setLayoutData(m_layout_data);
    ui->fontTestFrame->setRendererData(&m_font_renderer->data());
    ui->fontTestFrame->setFontConfig(m_font_config);

    ui->widgetFontPreview->setLayoutData(m_layout_data);
    ui->widgetFontPreview->setRendererData(&m_font_renderer->data());
    ui->widgetFontPreview->setLayoutConfig(m_layout_config);

    m_font_config->blockSignals(font_config_block);
    m_font_config->emmitChange();

    connect(m_font_config,SIGNAL(spacingChanged()),this,SLOT(onSpacingChanged()));
    ui->fontTestFrame->refresh();

    m_font_loader = new FontLoader(this);
}

FontBuilder::~FontBuilder()
{
    delete ui;
}


void FontBuilder::closeEvent(QCloseEvent *event)
 {
     QSettings settings;
     settings.setValue("geometry", saveGeometry());
     saveConfig(settings,"fontconfig",m_font_config);
     saveConfig(settings,"layoutconfig",m_layout_config);
     saveConfig(settings,"outputconfig",m_output_config);
     settings.setValue("draw_grid",ui->checkBoxDrawGrid->isChecked());
     QMainWindow::closeEvent(event);
 }

void FontBuilder::saveConfig(QSettings& settings,
                             const QString& name,
                             const QObject* object) {
    settings.beginGroup(name);
    const QMetaObject *metaobject = object->metaObject();
     int count = metaobject->propertyCount();
     for (int i=0; i<count; ++i) {
         QMetaProperty metaproperty = metaobject->property(i);
         const char *name = metaproperty.name();
         QVariant value = object->property(name);
         settings.setValue(name,value);
     }
    settings.endGroup();
}

void FontBuilder::readConfig(QSettings& settings,
                             const QString& name,
                             QObject* object) {
    settings.beginGroup(name);
    const QMetaObject *metaobject = object->metaObject();
     int count = metaobject->propertyCount();
     for (int i=0; i<count; ++i) {
         QMetaProperty metaproperty = metaobject->property(i);
         const char *name = metaproperty.name();
         if (settings.contains(name))
             object->setProperty(name,settings.value(name));
      }
    settings.endGroup();
}

void FontBuilder::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void FontBuilder::fontParametersChanged() {
    qDebug("font parameters changed");
}

void FontBuilder::on_comboBoxLayouter_currentIndexChanged(QString name)
{
    if (name.isEmpty()) return;
    if (m_layouter) {
        delete m_layouter;
        m_layouter = nullptr;
    }
    m_layouter = m_layouter_factory->build(name,this);
    if (m_layouter) {
        m_layouter->setConfig(m_layout_config);
        m_layouter->setData(m_layout_data);
        connect(m_font_renderer,SIGNAL(imagesChanged(QVector<LayoutChar>)),
                m_layouter,SLOT(on_ReplaceImages(QVector<LayoutChar>)));
        m_layouter->on_ReplaceImages(m_font_renderer->rendered());
        m_layout_config->setLayouter(name);
    }
}

void FontBuilder::onRenderedChanged() {
    ui->fontTestFrame->refresh();
    if (m_image_writer)
        m_image_writer->forget();
}


void FontBuilder::setLayoutImage(const QImage& image) {
    ui->widgetFontPreview->setImage(image);
    ui->label_ImageSize->setText(tr("Image size: ")+
            QString().number(m_layout_data->width()) + "x" +
            QString().number(m_layout_data->height())
            );
}

void FontBuilder::onLayoutChanged() {
    QImage image (m_layout_data->width(),m_layout_data->height(),QImage::Format_ARGB32);
    image.fill(0);
    {
        QPainter painter(&image);
        foreach (const LayoutChar& c,m_layout_data->placed()) {
            m_font_renderer->placeImage(painter,c.symbol,
                                        c.x + m_layout_config->offsetLeft(),
                                        c.y + m_layout_config->offsetTop()
                                        );
        }
    }
    qDebug() << "set layout image from rendered";
    m_layout_data->setImage(image);
    setLayoutImage(image);
    ui->fontTestFrame->refresh();
    if (m_image_writer)
        m_image_writer->forget();
}

void FontBuilder::on_checkBoxDrawGrid_toggled(bool dg) {
    ui->widgetFontPreview->setDrawGrid(dg);
}

void FontBuilder::onFontNameChanged() {
    QString name = m_font_config->family()+ "_" +
                   m_font_config->style()+ "_" +
                   QString().number(m_font_config->size());

    name = name.toLower().replace(" ","_");
    m_output_config->setImageName(name);
    m_output_config->setDescriptionName(name);
    if (m_image_writer)
        m_image_writer->forget();
}

void FontBuilder::doExport(bool x2) {
    QDir dir(m_output_config->path());
    QString texture_filename;
    setLayoutImage(m_layout_data->image());  

    if (m_output_config->writeImage()) {
        delete m_image_writer;
        m_image_writer = nullptr;

        //mudita pure must be selected in both image format and description format
        if( m_output_config->imageFormat().compare("Mudita Pure") == 0 &&
            m_output_config->descriptionFormat().compare("Mudita Pure") != 0 )
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Error ")+m_output_config->descriptionFormat()+ " selected. Must be "+m_output_config->imageFormat());
            msgBox.exec();
            return;
        }
        if( m_output_config->imageFormat().compare("Mudita Pure") != 0 &&
            m_output_config->descriptionFormat().compare("Mudita Pure") == 0 )
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Error ")+m_output_config->imageFormat()+ " selected. Must be "+m_output_config->descriptionFormat());
            msgBox.exec();
            return;
        }
        //both description and image formats are Mudita Pure
        if( m_output_config->imageFormat().compare("Mudita Pure") == 0 &&
            m_output_config->descriptionFormat().compare("Mudita Pure") == 0)
        {
            QString filename;
            QFile file;

            AbstractExporter* descExporter = m_exporter_factory->build(m_output_config->descriptionFormat(),this);
            if (!descExporter) {
                QMessageBox msgBox;
                msgBox.setText(tr("Unknown exporter :")+m_output_config->descriptionFormat());
                msgBox.exec();
                return;
            }
            descExporter->setFace(m_font_renderer->face());
            descExporter->setFontConfig(m_font_config,m_layout_config);
            descExporter->setData(m_layout_data,m_font_renderer->data());
            descExporter->setTextureFilename(texture_filename);
            descExporter->setScale(m_font_renderer->scale());

            QByteArray data;
            if (!descExporter->Write(data)) {
                 QMessageBox msgBox;
                 msgBox.setText(tr("Error on save description :\n")+descExporter->getErrorString()+"\nFile not writed.");
                 msgBox.exec();
             } else {
                filename = m_output_config->imageName();
                filename+="."+descExporter->getExtension();
                filename = dir.filePath(filename);
                 file.setFileName(filename);
                 if (file.open(QIODevice::ReadWrite)) {
                     file.write(data);
                 } else {
                     QMessageBox msgBox;
                     msgBox.setText(tr("Failed to open file:\n")+filename);
                     msgBox.exec();
                 }
             }

            AbstractImageWriter* imgExporter = m_image_writer_factory->build(m_output_config->imageFormat(),this);
            if (!imgExporter) {
                QMessageBox msgBox;
                msgBox.setText(tr("Unknown exporter :")+m_output_config->descriptionFormat());
                msgBox.exec();
                return;
            }

            imgExporter->setData(m_layout_data,m_layout_config,m_font_renderer->data());
            texture_filename = m_output_config->imageName();
            if (x2) {
                texture_filename += "_x2";
            }
            texture_filename+="."+imgExporter->extension();
             filename = dir.filePath(texture_filename);

            if (!imgExporter->Write(file)) {
                QMessageBox msgBox;
                msgBox.setText(tr("Error on save image :\n")+imgExporter->errorString()+"\nFile not writed.");
                msgBox.exec();
            }
            file.close();                        

            m_image_writer = imgExporter;
            m_image_writer->watch(filename);
            connect(m_image_writer,SIGNAL(imageChanged(QString)),this,SLOT(onExternalImageChanged(QString)));

            delete descExporter;

            return;
        }

        AbstractImageWriter* exporter = m_image_writer_factory->build(m_output_config->imageFormat(),this);
        if (!exporter) {
            QMessageBox msgBox;
            msgBox.setText(tr("Unknown exporter :")+m_output_config->descriptionFormat());
            msgBox.exec();
            return;
        }

        exporter->setData(m_layout_data,m_layout_config,m_font_renderer->data());
        texture_filename = m_output_config->imageName();
        if (x2) {
            texture_filename += "_x2";
        }
        texture_filename+="."+exporter->extension();
        QString filename = dir.filePath(texture_filename);

        QFile file(this);
        file.setFileName(filename);
        if (!file.open(QIODevice::WriteOnly)) {
            delete exporter;
            QMessageBox msgBox;
            msgBox.setText(tr("Error opening file :")+filename);
            msgBox.exec();
            return;
        }
        if (!exporter->Write(file)) {
            QMessageBox msgBox;
            msgBox.setText(tr("Error on save image :\n")+exporter->errorString()+"\nFile not writed.");
            msgBox.exec();
        }
        file.close();
        m_image_writer = exporter;
        m_image_writer->watch(filename);
        connect(m_image_writer,SIGNAL(imageChanged(QString)),this,SLOT(onExternalImageChanged(QString)));
    }
    if (m_output_config->writeDescription()) {
        AbstractExporter* exporter = m_exporter_factory->build(m_output_config->descriptionFormat(),this);
        if (!exporter) {
            QMessageBox msgBox;
            msgBox.setText(tr("Unknown exporter :")+m_output_config->descriptionFormat());
            msgBox.exec();
            return;
        }
        exporter->setFace(m_font_renderer->face());
        exporter->setFontConfig(m_font_config,m_layout_config);
        exporter->setData(m_layout_data,m_font_renderer->data());
        exporter->setTextureFilename(texture_filename);
        exporter->setScale(m_font_renderer->scale());
        QString filename = dir.filePath(m_output_config->descriptionName());
        if (x2) {
            filename += "_x2";
        }
        filename+="."+exporter->getExtension();
        QByteArray data;
        if (!exporter->Write(data)) {
             QMessageBox msgBox;
             msgBox.setText(tr("Error on save description :\n")+exporter->getErrorString()+"\nFile not writed.");
             msgBox.exec();
         } else {
             QFile file(filename);
             if (file.open(QIODevice::WriteOnly)) {
                 file.write(data);
             } else {

             }
         }
        delete exporter;
    }
}

void FontBuilder::on_pushButtonWriteFont_clicked()
{
    doExport(false);
    if (m_output_config->generateX2()) {
        m_font_renderer->render(2.0f);
        doExport(true);
        m_font_renderer->render(1.0f);
    }
}

void FontBuilder::onExternalImageChanged(const QString& fn) {
    if (!m_image_writer) return;
    qDebug() << "File changed : " << fn ;
    QFile f(this);
    f.setFileName(fn);
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed open : " << fn ;
        return;
    }
    QImage* image = m_image_writer->Read(f);
    if (image) {
        /*foreach (LayoutChar c, m_layout_data->placed()) {
            QImage img = image->copy(c.x+m_layout_config->offsetLeft(),c.y+m_layout_config->offsetTop(),
                                     c.w-m_layout_config->offsetLeft()-m_layout_config->offsetRight(),
                                     c.h-m_layout_config->offsetTop()-m_layout_config->offsetBottom());
            m_font_renderer->SetImage(c.symbol,img);
        }*/
        setLayoutImage(*image);
        m_layout_data->setImage(*image);
        qDebug() << "set layout image from exernal";
        ui->fontTestFrame->refresh();
        delete image;
    }
}

void FontBuilder::onSpacingChanged() {
    ui->fontTestFrame->refresh();
}

void FontBuilder::on_comboBox_currentIndexChanged(int index)
{
    static const float scales[] = { 0.5,1.0,2.0,4.0,8.0 };
    ui->widgetFontPreview->setScale(scales[index]);
}

void FontBuilder::on_action_Open_triggered()
{

    QString file = QFileDialog::getOpenFileName(this,tr("Select file"),
                                                QString(),
                                                tr("Font file(*.xml)"));
    if (!file.isEmpty()) {
        if (m_font_loader->Load(file)) {

        }
    }


}

void FontBuilder::on_pushButtonImportJson_clicked()
{
    if( m_output_config->imageFormat().compare("Mudita Pure") != 0 &&
        m_output_config->descriptionFormat().compare("Mudita Pure") != 0 ) {
        QMessageBox msgBox;
        msgBox.setText(tr("Error ")+". Must be Mudita Pure both description and image format!");
        msgBox.exec();
        return;
    }
    else if( m_output_config->imageFormat().compare("Mudita Pure") != 0 ||
             m_output_config->descriptionFormat().compare("Mudita Pure") != 0 )
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Error ")+". Must be Mudita Pure both description and image format!");
        msgBox.exec();
        return;
    }
    else {
        QString file_path = QFileDialog::getOpenFileName(this, "Open a file", QDir::homePath(), "JSON File (*.json)");
        QFile inFile(file_path);
        inFile.open(QIODevice::ReadOnly|QIODevice::Text);
        QByteArray data = inFile.readAll();
        inFile.close();

        QJsonParseError errorPtr;
        QJsonDocument doc = QJsonDocument::fromJson(data, &errorPtr);
        if(doc.isNull()) {
            qDebug() << "Failed to load file";
        }
        QJsonObject obj = doc.object();
        QVector<QPointF> listOfSettings;

        QJsonArray settingArray = obj.value("fonts").toArray();
        foreach(const QJsonValue & val, settingArray) {
            QString font_file = val.toObject().value("font-file").toString();
            QString font = val.toObject().value("font").toString();
            QString style = val.toObject().value("style").toString();
            int size = val.toObject().value("size").toInt();
            int dpi = val.toObject().value("DPI").toInt();
            int bold = val.toObject().value("bold").toInt();
            int italic = val.toObject().value("italic").toInt();
            float scale_width = val.toObject().value("scale_width").toInt();
            float scale_height = val.toObject().value("scale_height").toInt();
            int spacing_char = val.toObject().value("spacing_char").toInt();
            int spacing_line = val.toObject().value("spacing_line").toInt();
            if (m_font_config) {
                m_font_config->setFilename(font_file);
                m_font_config->setStyle(style);
                m_font_config->setSize(size);
                m_font_config->setDPI(dpi);
                m_font_config->setBold(bold);
                m_font_config->setItalic(italic);
                m_font_config->setWidth(scale_width);
                m_font_config->setHeight(scale_height);
                m_font_config->setCharSpacing(spacing_char);
                m_font_config->setLineSpacing(spacing_line);
            }
            doExport(false);
            if (m_output_config->generateX2()) {
                m_font_renderer->render(2.0f);
                doExport(true);
                m_font_renderer->render(1.0f);
            }
        }
    }
}
