#ifndef SERVER_H
#define SERVER_H

#include <QObject>

class QTcpServer;
class QTcpSocket;
class QFile;

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    void sendFileToClient(QUrl file);

signals:

public slots:
    void slotNewConnection();

private:
    QTcpServer *m_tcpServer;
    QTcpSocket *m_clientConnection; // Just a 1-1 connection for the moment..
    QFile *m_file;
    qint64 m_remainingBytes;
};

#endif // SERVER_H
