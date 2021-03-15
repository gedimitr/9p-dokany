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

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Exceptions.h"
#include "DataTypes.h"

struct ClientConfiguration
{
    ClientConfiguration(const std::wstring &host, const std::wstring &service) : host(host), service(service)
    {}

    std::wstring host;
    std::wstring service;
    std::wstring uname = L"nobody";
    std::wstring aname;
};

class Client
{
public:
    Client(const ClientConfiguration &config);
    ~Client();

    std::vector<RStat> getDirectoryContents(const std::wstring &wpath);

    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;

private:
    class Impl;
    std::unique_ptr<Impl> m_i;
};
