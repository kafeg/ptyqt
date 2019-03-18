#ifndef CONPTYNAMEDPIPEPROCESS_H
#define CONPTYNAMEDPIPEPROCESS_H

#include "conpty_shared.h"

class ConPtyNamedPipeProcess : public IWindowsPtyProcess
{
public:
    ConPtyNamedPipeProcess();
    ~ConPtyNamedPipeProcess();

    bool startProcess(const QString &shellPath, QStringList environment, qint16 cols, qint16 rows);
    bool resize(qint16 cols, qint16 rows);
    bool kill();
    PtyType type();
#ifdef PTYQT_DEBUG
    QString dumpDebugInfo();
#endif
    bool isAvailable();

private:
    WindowsContext m_winContext;
    HPCON m_ptyHandler;
    HANDLE m_hShell;
    HANDLE m_inPipeShellSide, m_outPipeShellSide;
};

#endif // CONPTYNAMEDPIPEPROCESS_H
