#ifndef WINPTYPROCESS_H
#define WINPTYPROCESS_H

#include "iptyprocess.h"
#include "winpty.h"

class WinPtyProcess : public IWindowsPtyProcess
{
public:
    WinPtyProcess();
    ~WinPtyProcess();

    bool startProcess(const QString &shellPath, QStringList environment, qint16 cols, qint16 rows);
    bool resize(qint16 cols, qint16 rows);
    bool kill();
    PtyType type();
#ifdef PTYQT_DEBUG
    QString dumpDebugInfo();
#endif
    bool isAvailable();

private:
    winpty_t *m_ptyHandler;
    HANDLE m_innerHandle;
};

#endif // WINPTYPROCESS_H
