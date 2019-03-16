#ifndef UNIXPTYPROCESS_H
#define UNIXPTYPROCESS_H

#include "iptyprocess.h"
#include <QProcess>
#include <QSocketNotifier>

class ShellProcess : public QProcess
{
    friend class UnixPtyProcess;
    Q_OBJECT
public:
    ShellProcess( )
        : QProcess( )
        , m_handleMaster(-1)
        , m_handleSlave(-1)
    {
        setProcessChannelMode(QProcess::SeparateChannels);
    }

    void emitReadyRead()
    {
        emit readyRead();
    }

protected:
    virtual void setupChildProcess();

private:
    int m_handleMaster, m_handleSlave;
    QString m_handleSlaveName;
};

class UnixPtyProcess : public IPtyProcess
{
public:
    UnixPtyProcess();
    virtual ~UnixPtyProcess();

    virtual bool startProcess(const QString &shellPath, QStringList environment, qint16 cols, qint16 rows);
    virtual bool resize(qint16 cols, qint16 rows);
    virtual bool kill();
    virtual PtyType type();
#ifdef PTYQT_DEBUG
    virtual QString dumpDebugInfo();
#endif
    virtual QIODevice *notifier();
    virtual QByteArray readAll();
    virtual qint64 write(const QByteArray &byteArray);
    virtual bool isAvailable();

private:
    ShellProcess m_shellProcess;
    QSocketNotifier *m_readMasterNotify;
    QByteArray m_shellReadBuffer;

};

#endif // UNIXPTYPROCESS_H
