// Copyright © Jacob Snyder, Billy O'Neal III
// This is under the 2 clause BSD license.
// See the included LICENSE.TXT file for more details.
// From: https://github.com/BillyONeal/Instalog/blob/master/LogCommon/Win32Exception.hpp

#pragma once
#include <string>
#include <exception>
#include <windows.h>

class Win32Exception : public std::exception
{
    DWORD errorCode_;

public:
    Win32Exception(DWORD errorCode) : errorCode_(errorCode) {};

    /// Gets from last error as an exception_ptr.
    static std::exception_ptr FromLastError();

    /// Gets the supplied error as an exception_ptr.
    static std::exception_ptr FromWinError(DWORD errorCode);

    /// Initializes a Win32Exception from the from the given NT error.
    static std::exception_ptr FromNtError(NTSTATUS errorCode);

    static void __declspec(noreturn) Throw(DWORD lastError);

    static void __declspec(noreturn) ThrowFromLastError()
    {
        Throw(::GetLastError());
    }
    ;

    DWORD GetErrorCode() const
    {
        return errorCode_;
    }

    std::wstring GetMessage() const;

    virtual const char* what() const
    {
        return "Win32 error. Use GetMessage to retrieve details";
    }
    ;
};

struct ErrorSuccessException : public Win32Exception
{
    ErrorSuccessException() : Win32Exception(ERROR_SUCCESS)
    {
    }
};

struct ErrorFileNotFoundException : public Win32Exception
{
    ErrorFileNotFoundException() : Win32Exception(ERROR_FILE_NOT_FOUND)
    {
    }
};

struct ErrorAccessDeniedException : public Win32Exception
{
    ErrorAccessDeniedException() : Win32Exception(ERROR_ACCESS_DENIED)
    {
    }
};

struct ErrorAlreadyExistsException : public Win32Exception
{
    ErrorAlreadyExistsException() : Win32Exception(ERROR_ALREADY_EXISTS)
    {
    }
};

struct ErrorPathNotFoundException : public Win32Exception
{
    ErrorPathNotFoundException() : Win32Exception(ERROR_PATH_NOT_FOUND)
    {
    }
};

struct ErrorInvalidParameterException : public Win32Exception
{
    ErrorInvalidParameterException() : Win32Exception(ERROR_INVALID_PARAMETER)
    {
    }
};

struct ErrorModuleNotFoundException : public Win32Exception
{
    ErrorModuleNotFoundException() : Win32Exception(ERROR_MOD_NOT_FOUND)
    {
    }
};

struct ErrorProcedureNotFoundException : public Win32Exception
{
    ErrorProcedureNotFoundException() : Win32Exception(ERROR_PROC_NOT_FOUND)
    {
    }
};

class HresultException : public std::exception
{
    std::string narrow;
    HRESULT hResult;

public:
    HresultException(HRESULT hRes, std::string n);
    HRESULT GetErrorCode() const;
    std::string const& GetErrorStringA() const;
    virtual char const* what();
};

std::wstring GetWin32ErrorMessage(DWORD errorCode);
