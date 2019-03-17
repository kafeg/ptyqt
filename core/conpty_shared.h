#ifndef CONPTY_SHARED_H
#define CONPTY_SHARED_H

#include "iptyprocess.h"
#include <windows.h>
#include <process.h>

//Taken from the RS5 Windows SDK, but redefined here in case we're targeting <= 17733
//Just for compile, ConPty doesn't work with Windows SDK < 17733
#ifndef PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE
#define PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE \
  ProcThreadAttributeValue(22, FALSE, TRUE, FALSE)

typedef VOID* HPCON;

#define TOO_OLD_WINSDK
#endif

template <typename T>
std::vector<T> vectorFromString(const std::basic_string<T> &str)
{
    return std::vector<T>(str.begin(), str.end());
}

#endif // CONPTY_SHARED_H
