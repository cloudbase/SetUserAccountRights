/*
Copyright 2015 Cloudbase Solutions Srl
All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License"); you may
not use this file except in compliance with the License. You may obtain
a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
License for the specific language governing permissions and limitations
under the License.
*/

#include "stdafx.h"

#include <ntsecapi.h>

#include "UserRights.h"
#include "Win32Exception.h"

void InitLsaString(PLSA_UNICODE_STRING lsaStr, LPWSTR str)
{
    if (str == NULL) {
        lsaStr->Buffer = NULL;
        lsaStr->Length = 0;
        lsaStr->MaximumLength = 0;
        return;
    }

    DWORD l = wcslen(str);
    lsaStr->Buffer = str;
    lsaStr->Length = (USHORT)l * sizeof(WCHAR);
    lsaStr->MaximumLength = (USHORT)(l + 1) * sizeof(WCHAR);
}

void CheckRetVal(NTSTATUS res, ULONG okRetVal = ERROR_SUCCESS)
{
    ULONG err = LsaNtStatusToWinError(res);
    if (err != ERROR_SUCCESS && err != okRetVal)
    {
        Win32Exception::Throw(err);
    }
}

LSA_HANDLE OpenPolicy(DWORD accessMask)
{
    LSA_OBJECT_ATTRIBUTES objectAttributes;
    ZeroMemory(&objectAttributes, sizeof(objectAttributes));
    LSA_UNICODE_STRING lsaMachineName;
    InitLsaString(&lsaMachineName, L".");
    LSA_HANDLE hPolicy = NULL;

    CheckRetVal(LsaOpenPolicy(&lsaMachineName, &objectAttributes, accessMask, &hPolicy));

    return hPolicy;
}

void GetSid(PSID sid, LPDWORD pSidSize, LPCWSTR accountName)
{
    SID_NAME_USE use;
    WCHAR referencedDomainName[1024];
    DWORD cchReferencedDomainName = sizeof(referencedDomainName) / sizeof(WCHAR);
    if (!LookupAccountName(NULL, accountName, sid, pSidSize, referencedDomainName, &cchReferencedDomainName, &use))
    {
        Win32Exception::ThrowFromLastError();
    }
}

std::vector<std::wstring> GetPrivileges(PSID sid)
{
    LSA_HANDLE hPolicy = OpenPolicy(POLICY_LOOKUP_NAMES);
    PLSA_UNICODE_STRING userRights = NULL;

    try
    {
        ULONG rightsCount = 0;

        CheckRetVal(LsaEnumerateAccountRights(hPolicy, sid, &userRights, &rightsCount), ERROR_FILE_NOT_FOUND);

        std::vector<std::wstring> v;
        for (int i = 0; i < rightsCount; i++)
        {
            std::wstring s(userRights[i].Buffer, userRights[i].Length / sizeof(WCHAR));
            v.push_back(s);
        }

        LsaFreeMemory(userRights);
        userRights = NULL;
        LsaClose(hPolicy);
        hPolicy = NULL;

        return v;
    }
    catch (const std::exception&)
    {
        if (userRights)
        {
            LsaFreeMemory(userRights);
        }

        if (hPolicy)
        {
            LsaClose(hPolicy);
        }

        throw;
    }
}

void GrantPrivilege(PSID sid, LPCWSTR userRight)
{
    LSA_HANDLE hPolicy = OpenPolicy(POLICY_LOOKUP_NAMES | POLICY_CREATE_ACCOUNT);

    try
    {
        LSA_UNICODE_STRING lsaUserRight;
        InitLsaString(&lsaUserRight, (LPWSTR)userRight);
        CheckRetVal(LsaAddAccountRights(hPolicy, sid, &lsaUserRight, 1));

        LsaClose(hPolicy);
    }
    catch (const std::exception&)
    {
        LsaClose(hPolicy);
        throw;
    }
}

void RevokePrivilege(PSID sid, LPCWSTR userRight)
{
    LSA_HANDLE hPolicy = OpenPolicy(POLICY_LOOKUP_NAMES);

    try
    {
        LSA_UNICODE_STRING lsaUserRight;
        InitLsaString(&lsaUserRight, (LPWSTR)userRight);
        CheckRetVal(LsaRemoveAccountRights(hPolicy, sid, FALSE, &lsaUserRight, 1));
        LsaClose(hPolicy);
    }
    catch (const std::exception&)
    {
        LsaClose(hPolicy);
        throw;
    }
}
