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

#include <tuple>

#include "UserRights.h"
#include "Win32Exception.h"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

enum Action
{
    NONE,
    LIST,
    GRANT,
    REVOKE
};

const std::string version = "1.0";

void CheckOption(const po::variables_map& vm, const std::string& option_name)
{
    if (!vm.count(option_name))
    {
        throw std::logic_error("Option \"" + option_name + "\" is required.");
    }
}

std::tuple<Action, std::wstring, std::wstring> GetOptions(int argc, _TCHAR* argv[])
{
    po::options_description desc(
        "Copyright Cloudbase Solutions Srl (c) 2015\nVersion: " +
        version + " - http://www.cloudbase.it\n\nAllowed options");
    desc.add_options()
        ("help,h", "display usage info")
        ("list,l", po::wvalue<std::wstring>(), "list existing account rights and privileges")
        ("grant,g", po::wvalue<std::wstring>(), "grant account right or privilege")
        ("revoke,r", po::wvalue<std::wstring>(), "revoke account right or privilege")
        ("value,v", po::wvalue<std::wstring>(), "account right or privilege");

    po::variables_map vm;
    po::basic_parsed_options<wchar_t> opt = po::parse_command_line(argc, argv, desc);
    po::store(opt, vm);
    po::notify(vm);

    if (vm.count("list") + vm.count("grant") + vm.count("revoke") + vm.count("help") > 1)
    {
        throw std::logic_error("Only one action can be specified: help, list, grant, revoke");
    }

    std::vector<std::wstring> v = po::collect_unrecognized(opt.options, po::include_positional);
    if (v.size() > 0)
    {
        throw std::logic_error("Unrecognized options. Use -h and --help for usage information.");
    }

    if (vm.count("list"))
    {
        return std::make_tuple(Action::LIST,
                               vm["list"].as<std::wstring>(),
                               std::wstring());
    }
    else if (vm.count("grant"))
    {
        CheckOption(vm, "value");
        return std::make_tuple(Action::GRANT,
                               vm["grant"].as<std::wstring>(),
                               vm["value"].as<std::wstring>());
    }
    else if (vm.count("revoke"))
    {
        CheckOption(vm, "value");
        return std::make_tuple(Action::REVOKE,
            vm["revoke"].as<std::wstring>(),
            vm["value"].as<std::wstring>());
    }
    else
    {
        // Display usage
        std::cout << desc << std::endl;
        return std::make_tuple(Action::NONE, std::wstring(), std::wstring());
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
    std::locale::global(std::locale(""));

    PSID sid = NULL;

    try
    {
        Action action;
        std::wstring name;
        std::wstring privilege;

        std::tie(action, name, privilege) = GetOptions(argc, argv);

        if (action == Action::NONE)
        {
            return 0;
        }

        DWORD sidSize = 1024;
        PSID sid = malloc(sidSize);
        if (!sid)
        {
            throw std::exception("Malloc failed");
        }

        ZeroMemory(sid, sidSize);

        GetSid(sid, &sidSize, name.c_str());

        switch (action)
        {
        case  Action::LIST:
            {
                std::vector<std::wstring>& v = GetPrivileges(sid);
                for (std::vector<std::wstring>::iterator it = v.begin(); it != v.end(); ++it)
                {
                    std::wcout << *it << std::endl;
                }
            }
            break;
        case Action::GRANT:
            GrantPrivilege(sid, privilege.c_str());
            std::wcout << L"User right / privilege granted: " << privilege << std::endl;
            break;
        case Action::REVOKE:
            try
            {
                RevokePrivilege(sid, privilege.c_str());
                std::wcout << L"User right / privilege revoked: " << privilege << std::endl;
            }
            catch (ErrorFileNotFoundException&)
            {
                std::wcout << L"The user does not hold the requested user right / privilege: " << privilege << std::endl;
            }
            break;
        }

        free(sid);
        sid = NULL;
    }
    catch (const Win32Exception& ex)
    {
        if (sid)
        {
            free(sid);
            sid = NULL;
        }

        std::wcerr << ex.GetMessage() << std::endl;
        return 1;
    }
    catch (const std::exception& ex)
    {
        if (sid)
        {
            free(sid);
            sid = NULL;
        }

        std::wcerr << ex.what() << std::endl;
        return 1;
    }

	return 0;
}

