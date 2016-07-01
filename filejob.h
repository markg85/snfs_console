#ifndef FILEJOB_H
#define FILEJOB_H

#include <QObject>
#include <QMutex>
#include <QBuffer>

class QFile;

class FileJob : public QObject
{
    Q_OBJECT
public:
    explicit FileJob(QObject *parent = 0);
    QBuffer *buffer();

signals:
    void doneStreaming();
    void moreDataAvailable();

public slots:
    void init(QString file);
    void slotRequestMoreData();

private:
    QFile *m_file;
    QBuffer m_buffer;
    QByteArray m_bufferBackend;
    QMutex m_mutex;
};

#endif // FILEJOB_H
