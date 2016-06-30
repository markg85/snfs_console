#ifndef FILEJOB_H
#define FILEJOB_H

#include <QObject>

class QFile;

class FileJob : public QObject
{
    Q_OBJECT
public:
    explicit FileJob(QObject *parent = 0);
    QByteArray *buffer();

signals:
    void doneStreaming();
    void moreDataAvailable();

public slots:
    void init(QString file);
    void slotRequestMoreData();

private:
    QFile *m_file;
    QByteArray m_buffer;
    qint64 m_sizeRead;
};

#endif // FILEJOB_H
