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
#include <Windows.h>
#include <WinSock2.h>
#include <Ws2ipdef.h>
#include <MSWSock.h>
#include <winternl.h>
#include <ip2string.h>

#include "spdlog/spdlog.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Ntdll.lib")

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
        spdlog::error(L"Winsock initialization failed");
        throw WinsockInitializationFailed();
    }
}

WinsockInitializer::~WinsockInitializer()
{
    spdlog::trace(L"Shutting down Winsock");
    WSACleanup();
}

SOCKET createSocket()
{
    SOCKET s = socket(AF_INET6, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        spdlog::error(L"Socket could not be created. Error status: {}", WSAGetLastError());
        throw ClientInitializationError();
    }

    return s;
}

void unsetIPv6OnlySocketOption(SOCKET s)
{
    int ipv6_only = 0;
    int res = setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&ipv6_only, sizeof(ipv6_only));
    if (res == SOCKET_ERROR) {
        spdlog::error(L"Disable 'IPv6 Only' socket option failed. Error status: {}", WSAGetLastError());
        throw ClientInitializationError();
    }

    spdlog::trace(L"Socket option 'IPv6 Only' successfully disabled");
}

void updateConnectContextSocketOption(SOCKET s)
{
    int res = setsockopt(s, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
    if (res == SOCKET_ERROR) {
        spdlog::warn(L"Update connect context failed. Error stats: {}", WSAGetLastError());
        throw ConnectionFailed();
    }

    spdlog::trace(L"Connect context successfully updated");
}

std::wstring printSockAddrIn(const SOCKADDR_IN &sockaddr_in)
{
    const IN_ADDR *in_addr = &sockaddr_in.sin_addr;
    USHORT port = sockaddr_in.sin_port;

    wchar_t buffer[INET_ADDRSTRLEN] = {0};
    ULONG addr_str_len = 0;

    NTSTATUS res = RtlIpv4AddressToStringExW(in_addr, port, buffer, &addr_str_len);

    if (res == 0) {
        return std::wstring(buffer);
    } else {
        spdlog::warn("Conversion of IPv4 address/port to string failed");
        return std::wstring(L"<conversion failed>");
    }
}

std::wstring printSockAddrIn6(const SOCKADDR_IN6 &sockaddr_in)
{
    const IN6_ADDR *in_addr = &sockaddr_in.sin6_addr;
    ULONG scope_id = sockaddr_in.sin6_scope_id;
    USHORT port = sockaddr_in.sin6_port;

    wchar_t buffer[INET6_ADDRSTRLEN] = {0};
    ULONG addr_str_len = 0;

    NTSTATUS res = RtlIpv6AddressToStringExW(in_addr, scope_id, port, buffer, &addr_str_len);

    if (res == 0) {
        return std::wstring(buffer);
    } else {
        spdlog::warn("Conversion of IPv6 address/port to string failed");
        return std::wstring(L"<conversion failed>");
    }
}

std::wstring printAddr(const SOCKADDR &sock_addr)
{
    ADDRESS_FAMILY sa_family = sock_addr.sa_family;
    if (sa_family == AF_INET) {
        const SOCKADDR_IN &sockaddr_in = reinterpret_cast<const SOCKADDR_IN &>(sock_addr);
        return printSockAddrIn(sockaddr_in);
    } else if (sa_family == AF_INET6) {
        const SOCKADDR_IN6 &sockaddr_in6 = reinterpret_cast<const SOCKADDR_IN6 &>(sock_addr);
        return printSockAddrIn6(sockaddr_in6);
    } else {
        spdlog::warn("Cannot print address, unsupported address family ({})", sa_family);
        return L"<Unsupported address family>";
    }
}

} // namespace

class Client::Impl
{
public:
    Impl(const std::wstring &host, const std::wstring &service);
    ~Impl();

    void connectToServer();

    std::wstring m_host;
    std::wstring m_service;

    SOCKET m_socket = INVALID_SOCKET;

    WinsockInitializer m_winsock_initializer;
};

Client::Impl::Impl(const std::wstring &host, const std::wstring &service)
    : m_host(host), m_service(service), m_socket(createSocket())
{
    connectToServer();
}

Client::Impl::~Impl()
{
    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
    }
}

void Client::Impl::connectToServer()
{
    unsetIPv6OnlySocketOption(m_socket);

    SOCKADDR_STORAGE local_addr = {0};
    DWORD local_addr_len = sizeof(local_addr);

    SOCKADDR_STORAGE remote_addr = {0};
    DWORD remote_addr_len = sizeof(remote_addr);

    bool res = WSAConnectByNameW(m_socket, m_host.data(), m_service.data(), &local_addr_len, (SOCKADDR *)&local_addr,
                                 &remote_addr_len, (SOCKADDR *)&remote_addr, NULL, NULL);

    if (!res) {
        spdlog::warn(L"Connection could not be established. Error status: {}", WSAGetLastError());
        throw ConnectionFailed();
    }

    spdlog::debug(L"Successfully connected to port {}", printAddr((const SOCKADDR &)remote_addr));

    updateConnectContextSocketOption(m_socket);
}

Client::Client(const std::wstring &host, const std::wstring &service) : m_i(new Impl(host, service))
{}

// The empty desctructor is needed because the header file does not provide enough information for one to delete the
// Impl object. We need to define the destructor in the source file, even though there is nothing specific to be
// recorded in it.
Client::~Client()
{}
