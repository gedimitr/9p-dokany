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
#include "Client.h"

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>

#pragma comment(lib, "Ws2_32.lib")

namespace {

class WinsockInitializer
{
public:
    WinsockInitializer();
    ~WinsockInitializer();
};

WinsockInitializer::WinsockInitializer()
{
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res) {
        throw WinsockInitializationFailed();
    }
}

WinsockInitializer::~WinsockInitializer()
{
    WSACleanup();
}

} // namespace

class Client::Impl
{
public:
    Impl(const std::wstring &host, const std::wstring &service);

    std::wstring m_host;
    std::wstring m_service;

    WinsockInitializer m_winsock_initializer;
};

Client::Impl::Impl(const std::wstring &host, const std::wstring &service) : m_host(host), m_service(service)
{}

Client::Client(const std::wstring &host, const std::wstring &service) : m_i(new Impl(host, service))
{}

// The empty desctructor is needed because the header file does not provide enough information for one to delete the
// Impl object. We need to define the destructor in the source file, even though there is nothing specific to be
// recorded in it.
Client::~Client()
{}
