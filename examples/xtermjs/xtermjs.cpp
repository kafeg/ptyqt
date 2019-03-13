#include <QCoreApplication>
#include <QWebSocketServer>
#include <QWebSocket>
#include "ptyqt.h"
#include <QTimer>
#include <QProcessEnvironment>

#define PORT 4242

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QWebSocketServer wsServer("TestServer", QWebSocketServer::NonSecureMode);
    if (!wsServer.listen(QHostAddress::Any, PORT))
        return 1;

    QMap<QWebSocket *, IPtyProcess *> sessions;

    //create new session on new connection
    QObject::connect(&wsServer, &QWebSocketServer::newConnection, [&wsServer, &sessions]()
    {
        QWebSocket *wSocket = wsServer.nextPendingConnection();
        IPtyProcess *pty = PtyQt::createPtyProcess(IPtyProcess::WinPty);

        qDebug() << "New connection" << wSocket->peerAddress() << wSocket->peerPort();

        QString shellPath = "c:\\Windows\\system32\\cmd.exe";
#ifdef Q_OS_UNIX
        shellPath = "/bin/bash";
#endif

        pty->startProcess(shellPath, QProcessEnvironment::systemEnvironment().toStringList(), 80, 24);

        QObject::connect(pty->notifier(), &QIODevice::readyRead, [wSocket, pty]()
        {
            wSocket->sendTextMessage(pty->readAll());
        });

        QObject::connect(wSocket, &QWebSocket::textMessageReceived, [wSocket, pty](const QString &message)
        {
            pty->write(message.toUtf8());
        });

        sessions.insert(wSocket, pty);
    });

    //QTimer::singleShot(5000, [](){ qApp->quit(); });

    bool res = app.exec();

    QMapIterator<QWebSocket *, IPtyProcess *> it(sessions);
    while (it.hasNext())
    {
        it.next();

        it.key()->deleteLater();
        delete it.value();
    }
    sessions.clear();

    return res;
}
