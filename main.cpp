#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>

#include "client.h"
#include "server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCoreApplication::setApplicationName("snfs (Simple Network File System)");
    QCoreApplication::setApplicationVersion("0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("Simple Network File System. Meant to get the most out of local networks.");
    parser.addHelpOption();
    parser.addVersionOption();

    // The mode argument (client or server)
    QCommandLineOption modeArgument("m", "The mode in which the application can run.", "client or server");
    parser.addOption(modeArgument);

    // The host address.
    QCommandLineOption serverArgument("s", "The address of the server.", "ip or hostname");
    parser.addOption(serverArgument);

    // The host address.
    QCommandLineOption fileArgument("f", "The file you want from the server.", "file");
    parser.addOption(fileArgument);

    // The host address.
    QCommandLineOption dataTypeArgument("d", "The type of data you want to send.", "file or benchmark", "file");
    parser.addOption(dataTypeArgument);

    // Process the parser
    parser.process(a);

    // Determine which mode we're in. If no mode property is set then abort. A mode must be given!
    if (parser.isSet(modeArgument))
    {
        QString modeValue = parser.value(modeArgument).toLower().trimmed();
        if (modeValue == "client")
        {
            bool allOk = true;
            QString serverAddress = parser.value(serverArgument).toLower().trimmed();

            if (serverAddress.isEmpty())
            {
                qCritical() << "You need to provide a valid server address. Like for example 10.0.0.1.";
                allOk &= false;
            }

            QString fileFromServer = parser.value(fileArgument).trimmed();

            if (fileFromServer.isEmpty())
            {
                qCritical() << "You need to provide a file to download from the server.";
                allOk &= false;
            }

            // Only continue if there are no errors.
            if (allOk == true)
            {
                qDebug() << "Running in client mode.";
                Client *client = new Client(serverAddress, 5001, fileFromServer, &a);
                a.exec();
            }
        }
        else if (modeValue == "server")
        {
            bool allOk = true;

            QString dataType = parser.value(dataTypeArgument).trimmed();

            qDebug() << dataType;

            // TODO: fully implement this to suport file and benchmark mode (currently it's just a stub, it does nothing).
            if (dataType.isEmpty())
            {
                qCritical() << "You need to provide a file to download from the server.";
                allOk &= false;
            }

            // Only continue if there are no errors.
            if (allOk == true)
            {
                qDebug() << "Running in server mode.";
                Server *server = new Server(&a);
                a.exec();
            }
        }
    }
    else
    {
        qCritical() << "The mode argument (-m) needs to be either 'client' or 'server'!";
    }

    a.exit(0);

    return 0;
}
