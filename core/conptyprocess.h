#ifndef CONPTYPROCESS_H
#define CONPTYPROCESS_H

#include "iptyprocess.h"
#include <QLibrary>
#include <consoleapi.h>
#include <process.h>
#include <stdio.h>

//Taken from the RS5 Windows SDK, but redefined here in case we're targeting <= 17134
//Just for compile, ConPty doesn't work with Windows SDK < 17134 or 18346
#ifndef PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE
#define PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE \
  ProcThreadAttributeValue(22, FALSE, TRUE, FALSE)

typedef VOID* HPCON;

#define TOO_OLD_WINSDK
#endif


//ConPTY available only on Windows 10 releazed after 1903 (19H1) Windows release
class WindowsContext
{
public:
    // Creates a "Pseudo Console" (ConPTY).
    typedef HRESULT (*CreatePseudoConsolePtr)(
            COORD size,         // ConPty Dimensions
            HANDLE hInput,      // ConPty Input
            HANDLE hOutput,	 // ConPty Output
            DWORD dwFlags,      // ConPty Flags
            HPCON* phPC);     // ConPty Reference

    // Resizes the given ConPTY to the specified size, in characters.
    typedef HRESULT (*ResizePseudoConsolePtr)(HPCON hPC, COORD size);

    // Closes the ConPTY and all associated handles. Client applications attached
    // to the ConPTY will also terminated.
    typedef VOID (*ClosePseudoConsolePtr)(HPCON hPC);

    WindowsContext()
        : createPseudoConsole(nullptr)
        , resizePseudoConsole(nullptr)
        , closePseudoConsole(nullptr)
    {

    }

    bool init()
    {
        //already initialized
        if (createPseudoConsole)
            return true;

        //try to load symbols from library
        //if it fails -> we can't use ConPty API
        HANDLE kernel32Handle = LoadLibraryExW(L"kernel32.dll", 0, 0);

        if (kernel32Handle != nullptr)
        {
            createPseudoConsole = (CreatePseudoConsolePtr)GetProcAddress((HMODULE)kernel32Handle, "CreatePseudoConsole");
            resizePseudoConsole = (ResizePseudoConsolePtr)GetProcAddress((HMODULE)kernel32Handle, "ResizePseudoConsole");
            closePseudoConsole = (ClosePseudoConsolePtr)GetProcAddress((HMODULE)kernel32Handle, "ClosePseudoConsole");
            if (!createPseudoConsole || !resizePseudoConsole || !closePseudoConsole)
            {
                m_lastError = QString("WindowsContext/ConPty error: %1").arg("Invalid on load API functions");
                return false;
            }
        }
        else
        {
            m_lastError = QString("WindowsContext/ConPty error: %1").arg("Unable to load kernel32");
            return false;
        }

        return true;
    }

    QString lastError()
    {
        return m_lastError;
    }

public:
    //vars
    CreatePseudoConsolePtr createPseudoConsole;
    ResizePseudoConsolePtr resizePseudoConsole;
    ClosePseudoConsolePtr closePseudoConsole;

private:
    QString m_lastError;
};

class ConPtyProcess : public IWindowsPtyProcess
{
public:
    ConPtyProcess();
    ~ConPtyProcess();

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

#endif // CONPTYPROCESS_H
