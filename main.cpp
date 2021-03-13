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
 * Copyright (C) 2020 Gerasimos Dimitriadis
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "9pfs.h"

#include "Config.h"

#include "spdlog/spdlog.h"
#include "protocol/Client.h"

int __cdecl wmain(unsigned long argc, wchar_t **argv)
{
    CommandLineScanResult scan_result = getConfigurationFromCommandLine(argc, argv);
    if (std::holds_alternative<ConfigurationError>(scan_result)) {
        const ConfigurationError &configuration_error = std::get<ConfigurationError>(scan_result);
        spdlog::error(L"Scanning of command line arguments failed: {}", configuration_error.message);

        return 1;
    }

    assert(std::holds_alternative<Configuration>(scan_result));

    const Configuration configuration = std::get<Configuration>(scan_result);
    ClientConfiguration client_configuration(configuration.server_host, configuration.server_port);
    Client client(client_configuration);

    return 0;
}