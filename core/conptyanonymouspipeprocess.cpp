#include "conptyanonymouspipeprocess.h"
#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <sstream>
#include <QTimer>

#define READ_INTERVAL_MSEC 500

HRESULT CreatePseudoConsoleAndPipes(HPCON* phPC, HANDLE* phPipeIn, HANDLE* phPipeOut, qint16 cols, qint16 rows)
{
    HRESULT hr{ E_UNEXPECTED };
    HANDLE hPipePTYIn{ INVALID_HANDLE_VALUE };
    HANDLE hPipePTYOut{ INVALID_HANDLE_VALUE };

    // Create the pipes to which the ConPTY will connect
    if (CreatePipe(&hPipePTYIn, phPipeOut, NULL, 0) &&
            CreatePipe(phPipeIn, &hPipePTYOut, NULL, 0))
    {
        // Create the Pseudo Console of the required size, attached to the PTY-end of the pipes
        hr = CreatePseudoConsole({cols, rows}, hPipePTYIn, hPipePTYOut, 0, phPC);

        // Note: We can close the handles to the PTY-end of the pipes here
        // because the handles are dup'ed into the ConHost and will be released
        // when the ConPTY is destroyed.
        if (INVALID_HANDLE_VALUE != hPipePTYOut) CloseHandle(hPipePTYOut);
        if (INVALID_HANDLE_VALUE != hPipePTYIn) CloseHandle(hPipePTYIn);
    }

    return hr;
}

// Initializes the specified startup info struct with the required properties and
// updates its thread attribute list with the specified ConPTY handle
HRESULT InitializeStartupInfoAttachedToPseudoConsole(STARTUPINFOEX* pStartupInfo, HPCON hPC)
{
    HRESULT hr{ E_UNEXPECTED };

    if (pStartupInfo)
    {
        size_t attrListSize{};

        pStartupInfo->StartupInfo.cb = sizeof(STARTUPINFOEX);

        // Get the size of the thread attribute list.
        InitializeProcThreadAttributeList(NULL, 1, 0, &attrListSize);

        // Allocate a thread attribute list of the correct size
        pStartupInfo->lpAttributeList =
                reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(malloc(attrListSize));

        // Initialize thread attribute list
        if (pStartupInfo->lpAttributeList
                && InitializeProcThreadAttributeList(pStartupInfo->lpAttributeList, 1, 0, &attrListSize))
        {
            // Set Pseudo Console attribute
            hr = UpdateProcThreadAttribute(
                        pStartupInfo->lpAttributeList,
                        0,
                        PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
                        hPC,
                        sizeof(HPCON),
                        NULL,
                        NULL)
                    ? S_OK
                    : HRESULT_FROM_WIN32(GetLastError());
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    return hr;
}

ConPtyAnonymousPipeProcess::ConPtyAnonymousPipeProcess()
    : IPtyProcess()
    , m_ptyHandler { INVALID_HANDLE_VALUE }
    , m_hPipeIn { INVALID_HANDLE_VALUE }
    , m_hPipeOut { INVALID_HANDLE_VALUE }
{

}

ConPtyAnonymousPipeProcess::~ConPtyAnonymousPipeProcess()
{
    kill();
}

bool ConPtyAnonymousPipeProcess::startProcess(const QString &shellPath, QStringList environment, qint16 cols, qint16 rows)
{
    if (!isAvailable())
    {
        m_lastError = "ConPty Error: ConPty unavailable on this system!";
        return false;
    }

    //already running
    if (m_ptyHandler != INVALID_HANDLE_VALUE)
        return false;

    QFileInfo fi(shellPath);
    if (fi.isRelative() || !QFile::exists(shellPath))
    {
        //todo add auto-find executable in PATH env var
        m_lastError = QString("ConPty Error: shell file path must be absolute");
        return false;
    }

    m_shellPath = shellPath;
    m_size = QPair<qint16, qint16>(cols, rows);

    //env
    std::stringstream envBlock;
    foreach (QString line, environment)
    {
        envBlock << line.toStdString() << '\0';
    }
    envBlock << '\0';
    std::string env = envBlock.str();
    auto envV = vectorFromString(env);
    LPSTR envArg = envV.empty() ? nullptr : envV.data();

    LPSTR cmdArg = new char[m_shellPath.toStdString().length() + 1];
    std::strcpy(cmdArg, m_shellPath.toStdString().c_str());
    //qDebug() << "m_shellPath" << m_shellPath << cmdArg << m_shellPath.toStdString().c_str();

    HRESULT hr{ E_UNEXPECTED };

    //  Create the Pseudo Console and pipes to it
    HANDLE hPipeIn{ INVALID_HANDLE_VALUE };
    HANDLE hPipeOut{ INVALID_HANDLE_VALUE };
    hr = CreatePseudoConsoleAndPipes(&m_ptyHandler, &hPipeIn, &hPipeOut, cols, rows);

    if (S_OK != hr)
    {
        m_lastError = QString("ConPty Error: CreatePseudoConsoleAndPipes fail");
        return false;
    }

    // Initialize the necessary startup info struct
    STARTUPINFOEX startupInfo{};
    if (S_OK != InitializeStartupInfoAttachedToPseudoConsole(&startupInfo, m_ptyHandler))
    {
        m_lastError = QString("ConPty Error: InitializeStartupInfoAttachedToPseudoConsole fail");
        return false;
    }

    // Launch ping to emit some text back via the pipe
    PROCESS_INFORMATION piClient{};
    hr = CreateProcess(
                NULL,                           // No module name - use Command Line
                cmdArg,                         // Command Line
                NULL,                           // Process handle not inheritable
                NULL,                           // Thread handle not inheritable
                FALSE,                          // Inherit handles
                EXTENDED_STARTUPINFO_PRESENT,   // Creation flags
                envArg, //NULL,                           // Use parent's environment block
                NULL,                           // Use parent's starting directory
                &startupInfo.StartupInfo,       // Pointer to STARTUPINFO
                &piClient)                      // Pointer to PROCESS_INFORMATION
            ? S_OK
            : GetLastError();

    if (S_OK != hr)
    {
        m_lastError = QString("ConPty Error: Cannot create process -> %1").arg(hr);
        return false;
    }
    m_pid = piClient.dwProcessId;

    //this code runned in separate thread
    QThread *readThread = QThread::create([this, &hPipeIn, &hPipeOut, &piClient, &startupInfo]()
    {
        HANDLE hConsole{ GetStdHandle(STD_OUTPUT_HANDLE) };

        const DWORD BUFF_SIZE{ 512 };
        char szBuffer[BUFF_SIZE]{};

        DWORD dwBytesWritten{};
        DWORD dwBytesRead{};
        BOOL fRead{ FALSE };

        QEventLoop ev;

        QTimer readTimer;
        QObject::connect(&readTimer, &QTimer::timeout, [this, &ev, &hPipeIn, &fRead, &szBuffer, &BUFF_SIZE, &dwBytesRead, &dwBytesWritten]()
        {
            // Read from the pipe
            fRead = ReadFile(hPipeIn, szBuffer, BUFF_SIZE, &dwBytesRead, NULL);

            // Write received text to the Console
            // Note: Write to the Console using WriteFile(hConsole...), not printf()/puts() to
            // prevent partially-read VT sequences from corrupting output
            //WriteFile(hConsole, szBuffer, dwBytesRead, &dwBytesWritten, NULL);
            //qDebug() << "READ" << QByteArray(szBuffer, dwBytesRead);

            m_buffer.m_readBuffer.append(szBuffer, dwBytesRead);

            //debug obly
            //if ( !fRead || dwBytesRead < 0 )
            //{
            //    kill();
            //}
        });
        readTimer.setInterval(READ_INTERVAL_MSEC);
        readTimer.start();

        //cleanup when thread or eventloop was finished
        QObject::connect(QThread::currentThread(), &QThread::finished, &ev, &QEventLoop::quit);
        m_readEv.exec();
        QThread::currentThread()->deleteLater();

        // Now safe to clean-up client app's process-info & thread
        CloseHandle(piClient.hThread);
        CloseHandle(piClient.hProcess);

        // Cleanup attribute list
        DeleteProcThreadAttributeList(startupInfo.lpAttributeList);
        free(startupInfo.lpAttributeList);
    });

    //start read thread
    readThread->start();


    return true;
}

bool ConPtyAnonymousPipeProcess::resize(qint16 cols, qint16 rows)
{
    if (m_ptyHandler == nullptr)
    {
        return false;
    }

    bool res = SUCCEEDED(ResizePseudoConsole(m_ptyHandler, {cols, rows}));

    if (res)
    {
        m_size = QPair<qint16, qint16>(cols, rows);
    }

    return res;

    return true;
}

bool ConPtyAnonymousPipeProcess::kill()
{
    bool exitCode = false;

    if ( m_ptyHandler != INVALID_HANDLE_VALUE )
    {
        m_readEv.quit();

        // Close ConPTY - this will terminate client process if running
        ClosePseudoConsole(m_ptyHandler);

        // Clean-up the pipes
        if (INVALID_HANDLE_VALUE != m_hPipeOut) CloseHandle(m_hPipeOut);
        if (INVALID_HANDLE_VALUE != m_hPipeIn) CloseHandle(m_hPipeIn);
        m_pid = 0;
        m_ptyHandler = INVALID_HANDLE_VALUE;
        m_hPipeOut = INVALID_HANDLE_VALUE;
        m_hPipeOut = INVALID_HANDLE_VALUE;

        exitCode = true;
    }

    return exitCode;
}

IPtyProcess::PtyType ConPtyAnonymousPipeProcess::type()
{
    return PtyType::ConPtyAnonPipe;
}

#ifdef PTYQT_DEBUG
QString ConPtyAnonymousPipeProcess::dumpDebugInfo()
{
    return QString("PID: %1, Type: %2, Cols: %3, Rows: %4")
            .arg(m_pid).arg(type())
            .arg(m_size.first).arg(m_size.second);
}
#endif

QIODevice *ConPtyAnonymousPipeProcess::notifier()
{
    return new QFile();
}

QByteArray ConPtyAnonymousPipeProcess::readAll()
{
    return m_buffer.m_readBuffer;
}

qint64 ConPtyAnonymousPipeProcess::write(const QByteArray &byteArray)
{
    DWORD dwBytesWritten{};
    WriteFile(m_hPipeOut, byteArray.data(), byteArray.size(), &dwBytesWritten, NULL);
    return dwBytesWritten;
}

bool ConPtyAnonymousPipeProcess::isAvailable()
{
#ifdef TOO_OLD_WINSDK
    return false; //very importnant! ConPty can be built, but it doesn't work if built with old sdk and Win10 < 1903
#endif

    qint32 buildNumber = QSysInfo::kernelVersion().split(".").last().toInt();
    if (buildNumber < CONPTY_MINIMAL_WINDOWS_VERSION)
        return false;
    //    return m_winContext.init();
    return true;
}
