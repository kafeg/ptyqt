#ifndef CONPTYANONYMOUSPIPEPROCESS_H
#define CONPTYANONYMOUSPIPEPROCESS_H

#include "conpty_shared.h"
#include <QMutex>
#include <QTimer>
#include <QThread>

class PtyBuffer : public QIODevice
{
    friend class ConPtyAnonymousPipeProcess;
    Q_OBJECT
public:

    PtyBuffer() {  }
    ~PtyBuffer() { }

    //just empty realization, we need only 'readyRead' signal of this class
    qint64 readData(char *data, qint64 maxlen) { return 0; }
    qint64 writeData(const char *data, qint64 len) { return 0; }

    bool   isSequential() { return true; }
    qint64 bytesAvailable() { return m_readBuffer.size(); }
    qint64 size() { return m_readBuffer.size(); }

    void emitReadyRead()
    {
        //for emit signal from PtyBuffer own thread
        QTimer::singleShot(1, this, [this]()
        {
             emit readyRead();
        });
    }

private:
    QByteArray m_readBuffer;
};

class ConPtyAnonymousPipeProcess : public IPtyProcess
{
public:
    ConPtyAnonymousPipeProcess();
    ~ConPtyAnonymousPipeProcess();

    bool startProcess(const QString &shellPath, QStringList environment, qint16 cols, qint16 rows);
    bool resize(qint16 cols, qint16 rows);
    bool kill();
    PtyType type();
#ifdef PTYQT_DEBUG
    QString dumpDebugInfo();
#endif
    virtual QIODevice *notifier();
    virtual QByteArray readAll();
    virtual qint64 write(const QByteArray &byteArray);
    bool isAvailable();

private:
    HRESULT createPseudoConsoleAndPipes(HPCON* phPC, HANDLE* phPipeIn, HANDLE* phPipeOut, qint16 cols, qint16 rows);
    HRESULT initializeStartupInfoAttachedToPseudoConsole(STARTUPINFOEX* pStartupInfo, HPCON hPC);

private:
    WindowsContext m_winContext;
    HPCON m_ptyHandler;
    HANDLE m_hPipeIn, m_hPipeOut;

    QThread *m_readThread;
    QMutex m_bufferMutex;
    PtyBuffer m_buffer;

};

#endif // CONPTYANONYMOUSPIPEPROCESS_H
