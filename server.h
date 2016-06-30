#ifndef SERVER_H
#define SERVER_H

#include <QObject>

class QTcpServer;
class QTcpSocket;
class QFile;
class QBuffer;

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
    void initDataFeeder(QString argument);
    void requestMoreData();

public slots:
    void slotNewConnection();
    void slotSendDataToClient();

private:
    QTcpServer *m_tcpServer;
    QTcpSocket *m_clientConnection; // Just a 1-1 connection for the moment..
    QBuffer *m_buffer;
};

#endif // SERVER_H
