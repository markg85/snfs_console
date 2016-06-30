#include "filejob.h"

#include <QFile>
#include <QDebug>


FileJob::FileJob(QObject *parent)
    : QObject(parent)
    , m_file(nullptr)
    , m_buffer(1024 * 1024 * 256, Qt::Uninitialized) // 256 MiB buffer
    , m_sizeRead(0)
{
}

QByteArray *FileJob::buffer()
{
    return &m_buffer;
}

void FileJob::init(QString file)
{
    m_file = new QFile(file, this);

    if (m_file->open(QIODevice::ReadOnly) == false)
    {
        qDebug() <<QString("Unable to open file: '%1'").arg(file);
    }
    else
    {
        qDebug() <<QString("File: '%1'' opened for reading").arg(file);
    }
}

void FileJob::slotRequestMoreData()
{
    m_buffer = m_file->read(std::min(static_cast<qint64>(m_buffer.size()), m_file->size() - m_sizeRead));
    m_sizeRead += m_buffer.size();

    emit moreDataAvailable();

    if (m_file->size() - m_sizeRead <= 0)
    {
        // The whole file has been read.
        emit doneStreaming();
    }
}
