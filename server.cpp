#include "server.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QUrl>
#include <QFile>
#include <QBuffer>
#include <QTimer>
#include <QThread>
#include <QtGlobal>

#include "filejob.h"

#define CHUNKSIZE 1024 * 4                  // 4    KiB
#define LOW_BYTES_THRESHOLD 1024 * 4        // 4    KiB
#define HIGH_BYTES_THRESHOLD 1024 * 256     // 256  KiB

Server::Server(QObject *parent)
    : QObject(parent)
    , m_tcpServer(new QTcpServer(this))
//    , m_dataType(DataType::None)
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

void Server::processClientRequest(DataType dataType, QString argument)
{
    if (dataType == DataType::File)
    {
        qDebug() << "Sending file to the client.";
        FileJob *file = new FileJob(argument, this);
        m_sourceDevice = file;

        connect(file, &FileJob::deviceAtEnd, this, [&]()
        {
            // We might still have bytes to write. This can easily happen because we request new bytes when the buffer goes
            // below the low threshold. If that buffer was the last part of the file then the next signal will be the doneStreaming
            // signal (this slot). Therefore we need to wait in here till all remaining bytes have been written before actually
            // closing the connection.
            while (m_clientConnection->bytesToWrite() > 0)
            {
                m_clientConnection->waitForBytesWritten();
            }

            // Now proceed with closing down.
            m_clientConnection->disconnectFromHost();
        });

        // Send the initial data. This starts the flow to write data to the socket.
        slotBytesWritten(0);
    }
    else if (dataType == DataType::Benchmark)
    {
        // TODO: To be implemented
        qDebug() << "Sending as much data as we can to the client.";
    }
    else if (dataType == DataType::None)
    {
        qDebug() << "Not sending anything to the client.";
    }
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

            processClientRequest(DataType::File, fileRequest);
        });

        connect(m_clientConnection, &QTcpSocket::bytesWritten, this, &Server::slotBytesWritten);
    }
}

void Server::slotBytesWritten(qint64)
{
    qint64 bytesToWrite = m_clientConnection->bytesToWrite();

    if (bytesToWrite <= LOW_BYTES_THRESHOLD)
    {
        while (m_sourceDevice->pos() < m_sourceDevice->size() && bytesToWrite <= HIGH_BYTES_THRESHOLD)
        {
            qint64 bytesToRead = std::min(static_cast<qint64>(CHUNKSIZE), m_sourceDevice->size() - m_sourceDevice->pos());
            m_clientConnection->write(m_sourceDevice->read(bytesToRead));
            bytesToWrite = m_clientConnection->bytesToWrite();
        }
    }
}
