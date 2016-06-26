#ifndef SERVER_H
#define SERVER_H

#include <QObject>

class QTcpServer;
class QTcpSocket;

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    void startStresstest();
    void sendFileToClient(QUrl file);

signals:

public slots:
    void slotNewConnection();

private:
    QTcpServer *m_tcpServer;
    QTcpSocket *m_clientConnection; // Just a 1-1 connection for the moment..
};

#endif // SERVER_H
