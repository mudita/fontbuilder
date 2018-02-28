#ifndef CARRAYEXPORTER_H
#define CARRAYEXPORTER_H

#include "../abstractexporter.h"

class CArrayExporter : public AbstractExporter
{
    Q_OBJECT
public:
    explicit CArrayExporter(QObject *parent = 0);

    virtual bool Export(QByteArray& out);
signals:

public slots:

};

#endif // BCARRAYEXPORTER_H
