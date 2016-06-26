#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QFile>

class QTcpSocket;

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QString server, quint16 port, QString fileFromServer, QObject *parent = 0);

signals:

public slots:
    void slotConnected();
    void slotDisconnected();
    void slotReadyRead();
    void slotTimerTimeout();

private:
    QTcpSocket *m_clientConnection;
    QUrl m_fileFromServer;
    QString m_filename;
    QTimer m_timer;
    int m_bytesPerSecond;
    quint64 m_bytesTransferred;
    QFile *m_file;
};

#endif // CLIENT_H
