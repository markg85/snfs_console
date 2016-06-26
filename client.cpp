#include "client.h"

#include <QTcpSocket>
#include <QDebug>

Client::Client(QString server, quint16 port, QString fileFromServer, QObject *parent)
    : QObject(parent)
    , m_clientConnection(new QTcpSocket(this))
    , m_fileFromServer(fileFromServer)
    , m_filename(m_fileFromServer.fileName())
    , m_timer(this)
    , m_bytesPerSecond(0)
    , m_bytesTransferred(0)
    , m_file(nullptr)
{
    connect(m_clientConnection, &QTcpSocket::connected, this, &Client::slotConnected);
    connect(m_clientConnection, &QTcpSocket::disconnected, this, &Client::slotDisconnected);
    connect(m_clientConnection, &QTcpSocket::readyRead, this, &Client::slotReadyRead);
    m_clientConnection->connectToHost(server, port);

    // Set the timer to 1 second intervals (1000 ms).
    m_timer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout, this, &Client::slotTimerTimeout);
}

void Client::slotConnected()
{
    qDebug() << "Client connected to server!";
    m_bytesPerSecond = 0;
    m_bytesTransferred = 0;

    qDebug() << m_fileFromServer;
    qDebug() << "Client connected. Write: " << m_clientConnection->write(m_fileFromServer.toString().toLocal8Bit());
    //m_clientConnection->write(m_fileFromServer.toString().toLocal8Bit());

    // Open the file in write mode.
    m_file = new QFile(m_filename, this);
    m_file->open(QIODevice::WriteOnly);

    // Start the timer (it will show the download progress every second).
    m_timer.start();
}

void Client::slotDisconnected()
{
    m_file->close();
    m_timer.stop();
    qDebug() << QString("Done downloading '%1' from the server. Transferred: %2 MiB").arg(m_filename).arg(static_cast<double>(m_bytesTransferred) / 1024 / 1024);
    qDebug() << "Client disconnected from server!";
}

void Client::slotReadyRead()
{
    //qDebug() << QString("%1 bytes available.").arg(m_clientConnection->bytesAvailable());
    const int bytesAvailable = m_clientConnection->bytesAvailable();
    m_bytesPerSecond += bytesAvailable;
    m_bytesTransferred += bytesAvailable;
    m_file->write(m_clientConnection->readAll());
}

void Client::slotTimerTimeout()
{
    // Reset the bytes per second. It will be filled by the readyRead handling.
    qDebug() << QString("%1 MiB/s (%2)").arg(static_cast<double>(m_bytesPerSecond) / 1024 / 1024).arg(m_bytesPerSecond);
    m_bytesPerSecond = 0;
}
