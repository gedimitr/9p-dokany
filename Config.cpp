/*
 * This file is part of 9p-dokany <https://github.com/gedimitr/9p-dokany>.
 *
 * 9p-dokany is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * 9p-dokany is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 9p-dokany. If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2020-2021 Gerasimos Dimitriadis
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "Config.h"

#include <cassert>
#include <cwchar>
#include <exception>

#include "spdlog/spdlog.h"

namespace {

class CommandLineConfigException : public std::exception
{
public:
    CommandLineConfigException(const std::wstring &slogan) : slogan(slogan)
    {}

    std::wstring slogan;
};

const wchar_t *USE_REMOVABLE_DRIVE_OPTION = L"/M";
const wchar_t *USE_NETWORK_DRIVE_OPTION = L"/N";
const wchar_t *USE_FOR_CURRENT_SESSION_OPTION = L"/C";
const wchar_t *DEBUG_OPTION = L"/DEBUG";
const wchar_t *ALLOW_NETWORK_UNMOUNT_OPTION = L"/X";
const wchar_t *MOUNT_POINT_OPTION = L"/M";
const wchar_t *UNC_NAME_OPTION = L"/UNC";
const wchar_t *SERVER_ADDR_OPTION = L"/S";
const wchar_t *SERVER_PORT_OPTION = L"/P";
const wchar_t *USER_NAME_OPTION = L"/U";

bool doesOptionTakeArgument(const std::wstring &option_str)
{
    return option_str == MOUNT_POINT_OPTION || option_str == UNC_NAME_OPTION || option_str == SERVER_ADDR_OPTION ||
           option_str == SERVER_PORT_OPTION || option_str == USER_NAME_OPTION;
    ;
}

std::wstring buildSloganOptionNeedsArgument(const std::wstring &opt_str)
{
    std::wstring slogan = L"Option (";
    slogan += opt_str;
    slogan += L") needs an argument";

    return slogan;
}

std::wstring buildSloganUnknownOption(const std::wstring &opt_str)
{
    std::wstring slogan = L"Option (";
    slogan += opt_str;
    slogan += L") is unknown";

    return slogan;
}

void evalCommandLineOption(unsigned long argc, wchar_t **argv, unsigned long *current_index,
                           Configuration *configuration)
{
    std::wstring opt_str = argv[*current_index];

    if (opt_str == USE_REMOVABLE_DRIVE_OPTION) {
        configuration->use_removable_drive = true;
    } else if (opt_str == USE_NETWORK_DRIVE_OPTION) {
        configuration->use_network_drive = true;
    } else if (opt_str == USE_FOR_CURRENT_SESSION_OPTION) {
        configuration->use_for_current_session = true;
    } else if (opt_str == DEBUG_OPTION) {
        configuration->debug = true;
    } else if (opt_str == ALLOW_NETWORK_UNMOUNT_OPTION) {
        configuration->allow_network_unmount = true;
    } else if (doesOptionTakeArgument(opt_str)) {
        unsigned long arg_index = ++(*current_index);
        if (arg_index >= argc) {
            std::wstring slogan = buildSloganOptionNeedsArgument(opt_str);
            throw CommandLineConfigException(slogan);
        }

        std::wstring arg_str = argv[arg_index];
        if (opt_str == MOUNT_POINT_OPTION) {
            configuration->mount_point = arg_str;
        } else if (opt_str == UNC_NAME_OPTION) {
            configuration->unc_provider_name = arg_str;
        } else if (opt_str == SERVER_ADDR_OPTION) {
            configuration->server_host = arg_str;
        } else if (opt_str == SERVER_PORT_OPTION) {
            configuration->server_port = arg_str;
        } else if (opt_str == USER_NAME_OPTION) {
            configuration->user_name = arg_str;
        } else {
            assert(false);
        }
    } else {
        spdlog::debug(L"Was asked to evaluate unknown command line option: {}", opt_str);
        std::wstring slogan = buildSloganUnknownOption(opt_str);
        throw CommandLineConfigException(slogan);
    }
}

void validateConfiguration(const Configuration &configuration)
{
    if (configuration.server_host.empty()) {
        throw CommandLineConfigException(L"Server host is mandatory.");
    }

    if (configuration.server_port.empty()) {
        throw CommandLineConfigException(L"Server port is mandatory");
    }
}

wchar_t *dupWString(const std::wstring &str)
{
    auto str_size = str.size();
    if (str_size) {
        wchar_t *new_str = new wchar_t[str.size() + 1];
        wcscpy_s(new_str, str.size(), str.c_str());

        return new_str;
    } else {
        return nullptr;
    }
}

unsigned long buildDokanOptionsFlags(const Configuration& configuration)
{
    unsigned long flags = DOKAN_OPTION_WRITE_PROTECT;

    if (configuration.debug) {
        flags |= DOKAN_OPTION_DEBUG | DOKAN_OPTION_STDERR;
    }

    if (configuration.use_network_drive) {
        flags |= DOKAN_OPTION_NETWORK;
    } else if (configuration.use_removable_drive) {
        flags |= DOKAN_OPTION_REMOVABLE;
    } else {
        flags |= DOKAN_OPTION_MOUNT_MANAGER;
    }

    if (configuration.use_for_current_session) {
        flags |= DOKAN_OPTION_CURRENT_SESSION;
    }

    return flags;
}

} // anonymous namespace

ConfigurationError::ConfigurationError(const std::wstring &message) : message(message)
{}

Configuration scanCommandLine(unsigned long argc, wchar_t **argv)
{
    Configuration configuration;

    for (unsigned long i = 1; i < argc; i++) {
        evalCommandLineOption(argc, argv, &i, &configuration);
    }

    return configuration;
}

CommandLineScanResult getConfigurationFromCommandLine(unsigned long argc, wchar_t **argv)
{
    try {
        Configuration configuration = scanCommandLine(argc, argv);
        validateConfiguration(configuration);

        return configuration;
    }
    catch (CommandLineConfigException &e) {
        spdlog::debug(L"Command line configuration parsing exception: {}", e.slogan);
        return ConfigurationError(e.slogan);
    }
}

void deleteDokanOptions(DOKAN_OPTIONS *dokan_options)
{
    delete[] dokan_options->MountPoint;
    delete[] dokan_options->UNCName;

    delete dokan_options;
}

DokanOptionsUniquePtr getDokanOptions(const Configuration &configuration)
{
    DOKAN_OPTIONS *dokan_options = new DOKAN_OPTIONS;
    memset(dokan_options, 0, sizeof(DOKAN_OPTIONS));

    dokan_options->Version = DOKAN_VERSION;
    dokan_options->ThreadCount = configuration.thread_count;
    dokan_options->Options = buildDokanOptionsFlags(configuration);
    dokan_options->MountPoint = dupWString(configuration.mount_point);
    dokan_options->UNCName = dupWString(configuration.unc_provider_name);
    dokan_options->Timeout = configuration.timeout_ms;

    return DokanOptionsUniquePtr(dokan_options, &deleteDokanOptions);
}
