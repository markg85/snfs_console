#include "server.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QUrl>
#include <QFile>

Server::Server(QObject *parent)
    : QObject(parent)
    , m_tcpServer(new QTcpServer(this))
{
    // Connect to signals from the server.
    connect(m_tcpServer, &QTcpServer::newConnection, this, &Server::slotNewConnection);

    // Now start listening for connections.
    m_tcpServer->listen(QHostAddress::Any, 5001);

    if (m_tcpServer->isListening())
    {
        qDebug() << QString("The server is listening at %1 on port: %2").arg(m_tcpServer->serverAddress().toString()).arg(m_tcpServer->serverPort());
    }
    else
    {
        qDebug() << "The TCP server encountered the following error:" << m_tcpServer->errorString();
    }
}

void Server::startStresstest()
{
    QByteArray buffer = QByteArray().fill('X', 4096);
    qDebug() << "Buffer size: " << buffer.size();

    // 4096 * 262144 = 1,073,741,824 == ~1GB!
    int numOfBuffersToSend = 262144;

    for (int i = 0; i < numOfBuffersToSend; i++)
    {
        m_clientConnection->write(buffer);
    }

    // Just disconnect the client. The client interprets this as "done sending file"
    m_clientConnection->disconnectFromHost();
}

void Server::sendFileToClient(QUrl file)
{
    qDebug() << QString("Going to send the file: %1 to the client").arg(file.toString());
    QString fileAsString = file.toString();

    if (QFile::exists(fileAsString) == false)
    {
        qDebug() << QString("Could not send file: %1, it doesn't exist.").arg(fileAsString);
        m_clientConnection->disconnectFromHost();
        return;
    }

    QFile fileRequest(fileAsString);
    fileRequest.open(QIODevice::ReadOnly);
    const int chunkSize = 4096;
    int remainingSize = fileRequest.size();

    QByteArray buffer;
    buffer.resize(chunkSize);

    while (remainingSize > 0)
    {
        if (remainingSize < chunkSize)
        {
            buffer.resize(remainingSize);
        }

        remainingSize -= fileRequest.read(buffer.data(), buffer.size());
        m_clientConnection->write(buffer);
    }

    qDebug() << "Done sending file. Closing connection.";
    m_clientConnection->disconnectFromHost();
}

void Server::slotNewConnection()
{
    QTcpSocket *clientConnection = m_tcpServer->nextPendingConnection();

    if (clientConnection == nullptr)
    {
        qDebug() << "Err, we received a new client connection, but it's already a nullptr.. WTF..";
    }
    else
    {
        m_clientConnection = clientConnection;
        qDebug() << QString("New client connection from %1 on port: %2").arg(m_clientConnection->peerAddress().toString()).arg(m_clientConnection->peerPort());

        //m_clientConnection->write("Hello dear snfs begin.. Lets make a great Net work File System! :)");
        //startStresstest();

        connect(m_clientConnection, &QTcpSocket::disconnected, [&]()
        {
            qDebug() << "Client disconnected. Resetting client pointer to null in the server";
            m_clientConnection->deleteLater();
        });

        connect(m_clientConnection, &QTcpSocket::readyRead, [&]()
        {
            QString fileRequest = QString::fromLocal8Bit(m_clientConnection->readAll());

            qDebug() << QString("Received request to send the following file to the client: %1").arg(fileRequest);

            sendFileToClient(fileRequest);
        });
    }
}
