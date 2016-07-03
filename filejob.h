#ifndef FILEJOB_H
#define FILEJOB_H

#include <QFile>

class FileJob : public QFile
{
    Q_OBJECT
public:
    explicit FileJob(QString fileUri, QObject *parent = 0);
    QByteArray read(qint64 maxlen);

signals:
    void deviceAtEnd();

public slots:

private:
};

#endif // FILEJOB_H
