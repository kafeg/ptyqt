#include "conptyprocess.h"
#include <QFileInfo>
#include <QMutex>
#include <sstream>
#include <QCryptographicHash>
#include <QUuid>
#include <QSocketNotifier>
#include<QThread>
#include <QSysInfo>

template <typename T>
std::vector<T> vectorFromString(const std::basic_string<T> &str)
{
    return std::vector<T>(str.begin(), str.end());
}

ConPtyProcess::ConPtyProcess()
    : IWindowsPtyProcess()
    , m_ptyHandler(INVALID_HANDLE_VALUE)
    , m_hShell(INVALID_HANDLE_VALUE)
{

}

ConPtyProcess::~ConPtyProcess()
{
    kill();
}

bool ConPtyProcess::startProcess(const QString &shellPath, QStringList environment, qint16 cols, qint16 rows)
{
    //shared between all objects in all threads
    static QMutex pipeNameCounterMutex;
    static qint64 pipeNameCounter = 0;

    if (!isAvailable())
    {
        m_lastError = m_winContext.lastError();
        return false;
    }

    //already running
    if (m_ptyHandler != INVALID_HANDLE_VALUE)
        return false;

    QFileInfo fi(shellPath);
    if (fi.isRelative() || !QFile::exists(shellPath))
    {
        //todo add auto-find executable in PATH env var
        m_lastError = QString("WinPty Error: shell file path must be absolute");
        return false;
    }

    m_shellPath = shellPath;
    m_size = QPair<qint16, qint16>(cols, rows);

    auto createNamedPipe = [] (bool writeChannel,
            HANDLE  *hNamedPipe,
            QString &resultName,
            const QString &pipeName) -> bool
    {
        *hNamedPipe = INVALID_HANDLE_VALUE;
        resultName = "\\\\.\\pipe\\" + pipeName;

        const DWORD winOpenMode =  PIPE_ACCESS_INBOUND | PIPE_ACCESS_OUTBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE/*  | FILE_FLAG_OVERLAPPED */;

        SECURITY_ATTRIBUTES sa = {};
        sa.nLength = sizeof(sa);

        *hNamedPipe = CreateNamedPipeW(
                    resultName.toStdWString().c_str(),
                    /*dwOpenMode=*/winOpenMode,
                    /*dwPipeMode=*/PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                    /*nMaxInstances=*/1,
                    /*nOutBufferSize=*/0,
                    /*nInBufferSize=*/0,
                    /*nDefaultTimeOut=*/30000,
                    &sa);

        return *hNamedPipe != INVALID_HANDLE_VALUE;
    };

    //shared between all objects in all threads
    qint64 pipeCounterVale = 0;
    {
        QMutexLocker locker(&pipeNameCounterMutex);
        pipeCounterVale = pipeNameCounter;
        pipeNameCounter++;
    }
    //

    QByteArray randPart = QCryptographicHash::hash(QByteArray::number(qrand()) + QByteArray::number(pipeCounterVale) + QUuid::createUuid().toByteArray(), QCryptographicHash::Sha256).left(16);
    bool createPipeRes = createNamedPipe(true, &m_inPipeShellSide, m_conInName, QString("conpty-conin-%1-%2").arg(pipeCounterVale).arg(QString::fromLatin1(randPart.toHex())));
    if (!createPipeRes)
    {
        m_lastError = QString("ConPty Error: Unable to create IN pipe -> %1").arg(GetLastError());
        return false;
    }

    randPart = QCryptographicHash::hash(QByteArray::number(qrand()) + QByteArray::number(pipeCounterVale) + QUuid::createUuid().toByteArray(), QCryptographicHash::Sha256).left(16);
    createPipeRes = createNamedPipe(false, &m_outPipeShellSide, m_conOutName, QString("conpty-conout-%1-%2").arg(pipeCounterVale).arg(QString::fromLatin1(randPart.toHex())));
    if (!createPipeRes)
    {
        m_lastError = QString("ConPty Error: Unable to create OUT pipe -> %1").arg(GetLastError());
        return false;
    }

    HRESULT result = m_winContext.createPseudoConsole({cols, rows}, m_inPipeShellSide, m_outPipeShellSide, 0, &m_ptyHandler);

    //CloseHandle(m_inPipeShellSide);
    //CloseHandle(m_outPipeShellSide);

    if (result != S_OK)
    {
        //error can be E_HANDLE
        m_lastError = QString("ConPty Error: Unable to launch ConPty -> %1").arg(QString::number(result, 16));
        return false;
    }

    //console created

    //this code runned in separate thread, because current thread will stops on API call 'ConnectNamedPipe'
    QThread *thread = QThread::create([this]()
    {
        QThread::msleep(10);
        m_inSocket.connectToServer(m_conInName, QIODevice::WriteOnly);
        m_inSocket.waitForConnected();
        m_outSocket.connectToServer(m_conOutName, QIODevice::ReadOnly);
        m_outSocket.waitForConnected();

        if (m_inSocket.state() != QLocalSocket::ConnectedState || m_outSocket.state() != QLocalSocket::ConnectedState)
        {
            m_lastError = QString("ConPty Error: Unable to connect local sockets -> %1 / %2").arg(m_inSocket.errorString()).arg(m_outSocket.errorString());
        }

        //qDebug() << "socketDescriptors" << m_inSocket.fullServerName() << m_inSocket.socketDescriptor() << m_outSocket.socketDescriptor() << m_inSocket.errorString() << m_outSocket.errorString();
        QThread::currentThread()->deleteLater();
    });
    //connect to Pty
    thread->start();

    //env
    std::wstringstream envBlock;
    foreach (QString line, environment)
    {
        envBlock << line.toStdWString() << L'\0';
    }
    envBlock << L'\0';
    std::wstring env = envBlock.str();
    auto envV = vectorFromString(env);
    LPWSTR envArg = envV.empty() ? nullptr : envV.data();
    //LPWSTR cmdArg = vectorFromString(m_shellPath.toStdWString()).data();
    LPWSTR cmdArg = (LPWSTR)m_shellPath.utf16();
    //LPWSTR cwdArg = vectorFromString(QFileInfo(m_shellPath).absolutePath().toStdWString()).data();
    //qDebug() << QString::fromStdWString(std::wstring(cmdArg)) << envArg;

    bool connected = ConnectNamedPipe(m_inPipeShellSide, nullptr) && ConnectNamedPipe(m_outPipeShellSide, nullptr);

    if (!connected)
    {
        m_lastError = QString("ConPty Error: Unable to connect named pipes");
        return false;
    }

    //attach the pseudoconsole to the client application we're creating
    STARTUPINFOEXW pStartupInfo{};
    pStartupInfo.StartupInfo.cb = sizeof(STARTUPINFOEXW);

    //get the size of the thread attribute list.
    size_t attrListSize{};
    InitializeProcThreadAttributeList(NULL, 1, 0, &attrListSize);

    //allocate a thread attribute list of the correct size
    pStartupInfo.lpAttributeList =
            reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(malloc(attrListSize));

    bool fSuccess = InitializeProcThreadAttributeList(pStartupInfo.lpAttributeList, 1, 0, &attrListSize);
    if (!fSuccess)
    {
        m_lastError = QString("ConPty Error: InitializeProcThreadAttributeList failed");
        return false;
    }
    fSuccess = UpdateProcThreadAttribute(pStartupInfo.lpAttributeList,
                                         0,
                                         PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
                                         m_ptyHandler,
                                         sizeof(HPCON),
                                         NULL,
                                         NULL);

    if (!fSuccess)
    {
        m_lastError = QString("ConPty Error: UpdateProcThreadAttribute failed");
        return false;
    }

    //create process
    PROCESS_INFORMATION piClient{};
    fSuccess = !!CreateProcessW(
                nullptr,
                cmdArg,
                nullptr,                      // lpProcessAttributes
                nullptr,                      // lpThreadAttributes
                false,                        // bInheritHandles VERY IMPORTANT that this is false
                EXTENDED_STARTUPINFO_PRESENT | CREATE_UNICODE_ENVIRONMENT, // dwCreationFlags
                envArg,                       // lpEnvironment
                nullptr,                      // lpCurrentDirectory
                &pStartupInfo.StartupInfo,    // lpStartupInfo
                &piClient                     // lpProcessInformation
                );

    if (!fSuccess)
    {
        m_lastError = QString("ConPty Error: Cannot create process -> %1").arg(QString::number(GetLastError(), 10));
        return false;
    }

    // Update handle
    m_hShell = piClient.hProcess;
    m_pid = piClient.dwProcessId;

    return true;
}

bool ConPtyProcess::resize(qint16 cols, qint16 rows)
{
    if (m_ptyHandler == nullptr)
    {
        return false;
    }

    bool res = SUCCEEDED(m_winContext.resizePseudoConsole(m_ptyHandler, {cols, rows}));

    if (res)
    {
        m_size = QPair<qint16, qint16>(cols, rows);
    }

    return res;
}

bool ConPtyProcess::kill()
{
    bool exitCode = false;
    if (m_hShell != 0 && m_ptyHandler != 0)
    {
        m_inSocket.disconnectFromServer();
        m_outSocket.disconnectFromServer();

        m_winContext.closePseudoConsole(m_ptyHandler);
        exitCode = CloseHandle(m_hShell);

        m_hShell = INVALID_HANDLE_VALUE;
        m_ptyHandler = INVALID_HANDLE_VALUE;
        m_conInName = QString();
        m_conOutName = QString();
        m_inPipeShellSide = INVALID_HANDLE_VALUE;
        m_outPipeShellSide = INVALID_HANDLE_VALUE;
        m_pid = 0;
    }
    return exitCode;
}

IPtyProcess::PtyType ConPtyProcess::type()
{
    return PtyType::ConPty;
}

#ifdef PTYQT_DEBUG
QString ConPtyProcess::dumpDebugInfo()
{
    return QString("PID: %1, ConIn: %2, ConOut: %3, Type: %4, Cols: %5, Rows: %6, IsRunning: %7, Shell: %8")
            .arg(m_pid).arg(m_conInName).arg(m_conOutName).arg(type())
            .arg(m_size.first).arg(m_size.second).arg(m_ptyHandler != nullptr)
            .arg(m_shellPath);
}
#endif

bool ConPtyProcess::isAvailable()
{
    qint32 buildNumber = QSysInfo::kernelVersion().split(".").last().toInt();
    if (buildNumber < CONPTY_MINIMAL_WINDOWS_VERSION)
        return false;
    return m_winContext.init();
}
