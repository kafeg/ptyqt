#ifndef IPTYPROCESS_H
#define IPTYPROCESS_H

#include <QString>
#include <QDebug>

#ifdef Q_OS_WIN
#include <QLocalSocket>
#endif

#define CONPTY_MINIMAL_WINDOWS_VERSION 18309

class IPtyProcess
{
public:
    enum PtyType
    {
        UnixPty = 0,
        WinPty = 1,
        ConPty = 2,
        AutoPty = 3
    };

    IPtyProcess()
        : m_pid(0)
    {  }

    virtual bool startProcess(const QString &shellPath, QStringList environment, qint16 cols, qint16 rows) = 0;
    virtual bool resize(qint16 cols, qint16 rows) = 0;
    virtual bool kill() = 0;
    virtual PtyType type() = 0;
#ifdef PTYQT_DEBUG
    virtual QString dumpDebugInfo() = 0;
#endif
    virtual QIODevice *notifier() = 0;
    virtual QByteArray readAll() = 0;
    virtual qint64 write(const QByteArray &byteArray) = 0;
    virtual bool isAvailable() = 0;
    qint64 pid() { return m_pid; }
    QPair<qint16, qint16> size() { return m_size; }
    const QString lastError() { return m_lastError; }

protected:
    QString m_shellPath;
    QString m_lastError;
    qint64 m_pid;
    QPair<qint16, qint16> m_size; //cols / rows
};

#ifdef Q_OS_WIN
class IWindowsPtyProcess : public IPtyProcess
{
protected:
    IWindowsPtyProcess()
        : IPtyProcess()
    { }

    QIODevice *notifier()
    {
        return &m_outSocket;
    }

    QByteArray readAll()
    {
        return m_outSocket.readAll();
    }

    qint64 write(const QByteArray &byteArray)
    {
        return m_inSocket.write(byteArray);
    }
protected:
    QString m_conInName;
    QString m_conOutName;
    QLocalSocket m_inSocket;
    QLocalSocket m_outSocket;
};
#endif

#endif // IPTYPROCESS_H
