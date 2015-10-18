// Copyright © Jacob Snyder, Billy O'Neal III
// This is under the 2 clause BSD license.
// See the included LICENSE.TXT file for more details.

#include "stdafx.h"
#include "Win32Exception.h"

#include <memory>


std::exception_ptr Win32Exception::FromLastError()
{
    return FromWinError(::GetLastError());
}

std::exception_ptr Win32Exception::FromWinError(DWORD errorCode)
{
    switch (errorCode)
    {
    case ERROR_SUCCESS:
        return std::make_exception_ptr(ErrorSuccessException());
    case ERROR_FILE_NOT_FOUND:
        return std::make_exception_ptr(ErrorFileNotFoundException());
    case ERROR_PATH_NOT_FOUND:
        return std::make_exception_ptr(ErrorPathNotFoundException());
    case ERROR_ACCESS_DENIED:
        return std::make_exception_ptr(ErrorAccessDeniedException());
    case ERROR_ALREADY_EXISTS:
        return std::make_exception_ptr(ErrorAlreadyExistsException());
    case ERROR_INVALID_PARAMETER:
        return std::make_exception_ptr(ErrorInvalidParameterException());
    case ERROR_MOD_NOT_FOUND:
        return std::make_exception_ptr(ErrorModuleNotFoundException());
    case ERROR_PROC_NOT_FOUND:
        return std::make_exception_ptr(ErrorProcedureNotFoundException());
    default:
        return std::make_exception_ptr(Win32Exception(errorCode));
    }
}

void __declspec(noreturn) Win32Exception::Throw(DWORD lastError)
{
    std::rethrow_exception(FromWinError(lastError));
}

struct LocalFreeHelper
{
    void operator()(void* toFree)
    {
        ::LocalFree(reinterpret_cast<HLOCAL>(toFree));
    }
    ;
};

std::wstring Win32Exception::GetMessage() const
{
    return GetWin32ErrorMessage(this->GetErrorCode());
}

std::wstring GetWin32ErrorMessage(DWORD errorCode)
{
    std::unique_ptr<wchar_t[], LocalFreeHelper> buff;
    LPWSTR buffPtr;
    DWORD bufferLength = ::FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL,
        errorCode,
        0,
        reinterpret_cast<LPWSTR>(&buffPtr),
        0,
        NULL);
    buff.reset(buffPtr);
    return std::wstring(buff.get());
}
