#include "filejob.h"

#include <QFile>
#include <QDebug>

#define BUFFER_SIZE 1024 * 1024 * 256 // 256 MiB

FileJob::FileJob(QObject *parent)
    : QObject(parent)
    , m_file(nullptr)
    , m_buffer()
    , m_bufferBackend()
    , m_mutex()
{
}

QBuffer *FileJob::buffer()
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
    // This can be done after the data fetching, but then you might end up with:
    // - Sending a signal indicating more data is available
    // - Sending another signal right away that you're done streaming.
    // Now the doneStreaming is only being send when all data has been read from the file and has been send to the client.
    // thus you have a more logic flow of signals.
    if (m_file->atEnd())
    {
        // The whole file has been read.
        emit doneStreaming();
        return;
    }

    QMutexLocker lock(&m_mutex);

    qDebug() << "file pos:" << m_file->pos() << "New file pos:" << (m_file->pos() - (m_buffer.size() - m_buffer.pos())) << "reading:" << std::min(static_cast<qint64>(BUFFER_SIZE), m_file->size() - m_file->pos());
    qDebug () << " -- Hole:" << m_buffer.pos() << m_buffer.size();


    m_file->seek(m_file->pos() - (m_buffer.size() - m_buffer.pos()));
    m_bufferBackend = m_file->read(std::min(static_cast<qint64>(BUFFER_SIZE), m_file->size() - m_file->pos()));
    m_buffer.close();
    m_buffer.setBuffer(&m_bufferBackend);

    emit moreDataAvailable();
}
