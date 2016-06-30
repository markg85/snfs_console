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

#define CHUNKSIZE 4096 // 4 KiB
#define LOW_BYTES_THRESHOLD 1024 * 64 // 64 KiB
#define HIGH_BYTES_THRESHOLD 1024 * 512 // 512 KiB

Server::Server(QObject *parent)
    : QObject(parent)
    , m_tcpServer(new QTcpServer(this))
    , m_buffer(nullptr)
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
        QThread *workerThread = new QThread(this);
        FileJob *file = new FileJob();
        file->moveToThread(workerThread);

        m_buffer = new QBuffer(file->buffer());
        m_buffer->open(QIODevice::ReadOnly);
        connect(this, &Server::initDataFeeder, file, &FileJob::init, Qt::QueuedConnection);
        connect(this, &Server::requestMoreData, file, &FileJob::slotRequestMoreData, Qt::QueuedConnection);
        connect(file, &FileJob::moreDataAvailable, this, [&]()
        {
            // If we get this signal, the buffer has been re-arranged with new data and starts from the beginning.
            m_buffer->seek(0);
            slotSendDataToClient();
        });
        connect(file, &FileJob::doneStreaming, this, [&]()
        {
            // If there are bytes left to be read, read them all and send them to the client.
            m_clientConnection->write(m_buffer->readAll());

            // Now proceed with closing down.
            m_clientConnection->disconnectFromHost();
            m_buffer = nullptr;
            workerThread->deleteLater();
        });

        // Start the worker thread.
        workerThread->start();

        // initialize emit... Has to be done this way since QFile is constructed in the filejob. It cannot be initialized in the constructor because the object then lives on the wrong thread...
        emit initDataFeeder(argument);

        // this is the initial emit. It triggers the start of the transfer flow.
        emit requestMoreData();
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
    }
}

void Server::slotSendDataToClient()
{
    // This could be a null pointer in the event that data was read here and a singleshot timer was fired to read more data.
    // However, for small files you could also have received the doneStreaming() signal which internally sends all remaining data (here, in the handler)
    // and then cleans the Job object and the m_buffer member.
    if (m_buffer == nullptr)
    {
        return;
    }

    qint64 bytesToWrite = m_clientConnection->bytesToWrite();

    if (bytesToWrite <= LOW_BYTES_THRESHOLD)
    {
        while (m_buffer->pos() < m_buffer->size() && bytesToWrite <= HIGH_BYTES_THRESHOLD)
        {
            QByteArray buffer(std::min(static_cast<qint64>(CHUNKSIZE), m_buffer->size() - m_buffer->pos()), Qt::Uninitialized);
            m_buffer->read(buffer.data(), buffer.size());
            m_clientConnection->write(buffer);
            bytesToWrite = m_clientConnection->bytesToWrite();
        }
    }

    if (m_buffer->size() - m_buffer->pos() < LOW_BYTES_THRESHOLD)
    {
        // Read all remaining data and send it to the client.
        m_clientConnection->write(m_buffer->readAll());

        // Now request more data.
        emit requestMoreData();
    }
    else if (m_buffer->pos() < m_buffer->size())
    {
        // We end up here when we've written the data required to fill the threshold and still have buffer room left to send more data.
        // We simply re-evaluate this function every 50ms with a singleshot timer.
        QTimer::singleShot(50, this, &Server::slotSendDataToClient);
    }
    else
    {
        qCritical() << "This shouldn't happen...";
    }
}
