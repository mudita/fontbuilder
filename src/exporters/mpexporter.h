#ifndef MPEXPORTER_H
#define MPEXPORTER_H

#include "../abstractexporter.h"

class MPExporter : public AbstractExporter
{
    Q_OBJECT
public:
    explicit MPExporter(QObject *parent = 0);

    virtual bool Export(QByteArray& out);
signals:

public slots:

};

#endif // MPEXPORTER_H
