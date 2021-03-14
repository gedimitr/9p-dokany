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
#pragma once

#include <memory>
#include <string>
#include <variant>

#include "dokan/dokan.h"

struct Configuration
{
    std::wstring mount_point;
    std::wstring unc_provider_name;

    std::wstring server_host;
    std::wstring server_port;
    std::wstring user_name;

    bool use_removable_drive = false;
    bool use_for_current_session = false;
    bool use_network_drive = false;
    unsigned short thread_count = 1;
    bool debug = false;
    int timeout_ms = 3000;
    bool allow_network_unmount = false;
};

struct ConfigurationError
{
    ConfigurationError(const std::wstring &message);

    std::wstring message;
};

using CommandLineScanResult = std::variant<Configuration, ConfigurationError>;

CommandLineScanResult getConfigurationFromCommandLine(unsigned long argc, wchar_t **argv);

void deleteDokanOptions(DOKAN_OPTIONS *dokan_options);

using DokanOptionsUniquePtr = std::unique_ptr<DOKAN_OPTIONS, decltype(&deleteDokanOptions)>;
DokanOptionsUniquePtr getDokanOptions(const Configuration &configuration);
