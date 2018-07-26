#ifndef MPWRITER_H
#define MPWRITER_H

#include "../abstractimagewriter.h"

class MPImageWriter : public AbstractImageWriter
{
Q_OBJECT
public:
    MPImageWriter(QString ext,QObject *parent = 0);

    virtual bool Export(QFile& file);
    virtual QImage* reload(QFile& file);
private:
signals:

public slots:

};

#endif // MPWRITER_H
