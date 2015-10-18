# SetUserAccountRights

A simple command line tool to list, grant and revoke
Windows user account rights and privileges.

This is particularly useful when assigning user accounts to
services, where the _SeServiceLogonRight_ right is needed.

It works on any supported version of Windows and Windows Server,
including Nano Server.

## Usage

Display usage information:

    SetUserAccountRights -h

List user rights or privileges assigned to a given local or domain account:

    SetUserAccountRights -l Administrator

Grant a given user account right or privilege to a user:

    SetUserAccountRights -g Administrator -v SeServiceLogonRight

Revoke a given user account right or privilege from a user:

    SetUserAccountRights -r Administrator -v SeServiceLogonRight

The list of valid user rights is available here:
https://msdn.microsoft.com/en-us/library/windows/desktop/bb545671(v=vs.85).aspx

The list of valid privileges is available here:
https://msdn.microsoft.com/en-us/library/windows/desktop/bb530716(v=vs.85).aspx
