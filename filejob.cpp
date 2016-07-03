#include "filejob.h"

#include <QFile>
#include <QDebug>

#define BUFFER_SIZE 1024 * 1024 * 256 // 256 MiB

FileJob::FileJob(QString fileUri, QObject *parent)
    : QFile(fileUri, parent)
{
    if (open(QIODevice::ReadOnly) == false)
    {
        qDebug() <<QString("Unable to open file: '%1'").arg(fileUri);
    }
    else
    {
        qDebug() <<QString("File: '%1'' opened for reading").arg(fileUri);
    }
}

QByteArray FileJob::read(qint64 maxlen)
{
    if (atEnd())
    {
        emit deviceAtEnd();
        return QByteArray();
    }
    else
    {
        return QIODevice::read(maxlen);
    }
}
