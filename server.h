#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include "filejob.h"

class QTcpServer;
class QTcpSocket;
class QFile;
class QBuffer;
class QIODevice;

enum DataType
{
    File,
    Benchmark,
    None
};

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    void processClientRequest(DataType dataType, QString argument);

signals:

public slots:
    void slotNewConnection();
    void slotBytesWritten(qint64 bytes);

private:
    QTcpServer *m_tcpServer;
    QTcpSocket *m_clientConnection; // Just a 1-1 connection for the moment..
    FileJob *m_sourceDevice;
};

#endif // SERVER_H
