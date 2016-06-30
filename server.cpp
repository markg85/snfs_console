#include "server.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QUrl>
#include <QFile>
#include <QtGlobal>

#define CHUNKSIZE 4096 // 4 KiB
#define LOW_BYTES_THRESHOLD 1024 * 64 // 64 KiB
#define HIGH_BYTES_THRESHOLD 1024 * 512 // 512 KiB

Server::Server(QObject *parent)
    : QObject(parent)
    , m_tcpServer(new QTcpServer(this))
    , m_file(nullptr)
    , m_remainingBytes(0)
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

    m_file = new QFile(fileAsString, this);
    m_file->open(QIODevice::ReadOnly);
    m_remainingBytes = m_file->size();

    QByteArray buffer(std::min(static_cast<qint64>(CHUNKSIZE), m_remainingBytes), Qt::Uninitialized);

    m_remainingBytes -= m_file->read(buffer.data(), buffer.size());
    m_clientConnection->write(buffer);
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

        connect(m_clientConnection, &QTcpSocket::bytesWritten, [&]()
        {
            if (m_clientConnection->bytesToWrite() <= LOW_BYTES_THRESHOLD)
            {
                while (m_remainingBytes > 0 && m_clientConnection->bytesToWrite() <= HIGH_BYTES_THRESHOLD)
                {
                    QByteArray buffer(std::min(static_cast<qint64>(CHUNKSIZE), m_remainingBytes), Qt::Uninitialized);

                    m_remainingBytes -= m_file->read(buffer.data(), buffer.size());
                    m_clientConnection->write(buffer);
                }
            }

            if (m_remainingBytes <= 0)
            {
                    qDebug() << "Done sending file. Closing connection.";
                    m_clientConnection->disconnectFromHost();
            }
        });
    }
}
