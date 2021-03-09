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

#include "ConstantValues.h"
#include "FidTracker.h"
#include "TxMessage.h"
#include "TxMessageBuilder.h"
#include "MessageReader.h"

#include "utils/TextConversion.h"
#include "spdlog/spdlog.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Ntdll.lib")

namespace {

constexpr int MAX_MSG_SIZE = 16 * 1024;
constexpr std::string_view PROTOCOL_VERSION = "9P2000";

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

template <class T>
class IntegerIssuer
{
public:
    T issue();

private:
    T m_value = 0;
};

template <class T>
T IntegerIssuer<T>::issue()
{
    static_assert(std::is_integral_v<T>, "Integer Issuer can issue only integers");

    return ++m_value;
}

using TagIssuer = IntegerIssuer<Tag>;
using FidIssuer = IntegerIssuer<Fid>;

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

std::wstring printAddr(const SOCKADDR_STORAGE &sock_addr_storage)
{
    const SOCKADDR &sock_addr = reinterpret_cast<const SOCKADDR &>(sock_addr_storage);
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

void validateRecvResult(int res)
{
    if (res == 0) {
        spdlog::error("Server closed connection");
        throw ConnectionClosed();
    } else if (res == SOCKET_ERROR) {
        spdlog::error(L"Reading from socket failed. Error status: {}", WSAGetLastError());
        throw RecvFailed();
    }
}

MsgLength peekForMessageLength(SOCKET socket)
{
    char read_buf[4];

    int res = recv(socket, read_buf, 4, MSG_PEEK);
    validateRecvResult(res);

    assert(res > 0);
    return parseMessageLength(read_buf);
} // namespace

} // namespace

class Client::Impl
{
public:
    Impl(const ClientConfiguration &config);
    ~Impl();

    void connectToServer();
    void doVersionHandshake();
    void doAuthentication();
    void doAttachment();
    void sendAttachMessage(Fid fid);

    void sendAuthMessage();

    void sendMessageInTxBuffer();
    std::string readIncomingMessage();
    ParsedRMessage readParseIncomingMessage();
    std::string readData(MsgLength message_length);

    ClientConfiguration m_config;

    WinsockInitializer m_winsock_initializer;
    SOCKET m_socket = INVALID_SOCKET;
    TxMessage m_tx_message;
    TxMessageBuilder m_tx_msg_builder;
    uint32_t m_max_message_size;

    TagIssuer m_tag_issuer;
    FidIssuer m_fid_issuer;

    FidTracker m_fid_tracker;
};

Client::Impl::Impl(const ClientConfiguration &config)
    : m_config(config), m_socket(createSocket()), m_tx_message(16 * 1024), m_tx_msg_builder(&m_tx_message),
      m_max_message_size(MAX_MSG_SIZE)
{
    connectToServer();
    doVersionHandshake();
    doAuthentication();
    doAttachment();
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

    wchar_t *host = m_config.host.data();
    wchar_t *service = m_config.service.data();
    bool res = WSAConnectByNameW(m_socket, host, service, &local_addr_len, (SOCKADDR *)&local_addr, &remote_addr_len,
                                 (SOCKADDR *)&remote_addr, NULL, NULL);

    if (!res) {
        spdlog::warn(L"Connection could not be established. Error status: {}", WSAGetLastError());
        throw ConnectionFailed();
    }

    spdlog::debug(L"Successfully connected to port {}", printAddr(remote_addr));

    updateConnectContextSocketOption(m_socket);
}

void Client::Impl::doVersionHandshake()
{
    m_tx_msg_builder.buildTVersion(m_max_message_size, PROTOCOL_VERSION);
    sendMessageInTxBuffer();

    ParsedRMessage response = readParseIncomingMessage();

    const ParsedRMessagePayload &response_payload = response.payload;
    if (std::holds_alternative<ParsedRVersion>(response_payload)) {
        const ParsedRVersion &rversion = std::get<ParsedRVersion>(response_payload);

        spdlog::debug(L"Received RVersion with msize: {} and version: {}", rversion.msize,
                      convertUtf8ToWstring(rversion.version));

        m_max_message_size = min(m_max_message_size, rversion.msize);
    } else if (std::holds_alternative<ParsedRError>(response_payload)) {
        const ParsedRError &rerror = std::get<ParsedRError>(response_payload);
        std::wstring w_ename = convertUtf8ToWstring(rerror.ename);
        spdlog::error(L"Server responded with RError to TVersion sent, with ename: {}", w_ename);
        throw VersionHandshakeError();
    } else {
        spdlog::error(L"Unexpected message received while waiting for response to TVersion");
        throw UnexpectedMessageReceived();
    }
}

void Client::Impl::doAuthentication()
{
    sendAuthMessage();

    ParsedRMessage response = readParseIncomingMessage();

    const ParsedRMessagePayload &response_payload = response.payload;
    if (std::holds_alternative<ParsedRError>(response_payload)) {
        spdlog::info(L"Server doesn't support authentication");
    } else if (std::holds_alternative<ParsedRAuth>(response_payload)) {
        spdlog::error(L"Server sent unsupported authentication details");
        throw ServerRequestedAuthentication();
    } else {
        spdlog::error(L"Unexpected message received while waiting for response to TVersion");
        throw UnexpectedMessageReceived();
    }
}

void Client::Impl::doAttachment()
{
    Fid fid = m_fid_issuer.issue();

    sendAttachMessage(fid);

    ParsedRMessage response = readParseIncomingMessage();

    const ParsedRMessagePayload &response_payload = response.payload;
    if (std::holds_alternative<ParsedRAttach>(response_payload)) {
        spdlog::debug(L"Server responded to TAttach with RAttach");
        const ParsedRAttach &parsed_rattach = std::get<ParsedRAttach>(response_payload);
        m_fid_tracker.setRoot(fid, parsed_rattach.qid);
    } else if (std::holds_alternative<ParsedRError>(response_payload)) {
        const ParsedRError &rerror = std::get<ParsedRError>(response_payload);
        std::wstring w_ename = convertUtf8ToWstring(rerror.ename);
        spdlog::error(L"Server responded with RError to TAttach sent, with ename: {}", w_ename);
        throw UnexpectedMessageReceived();
    } else {
        spdlog::error(L"Unexpected message received while waiting for response to TAttach");
        throw UnexpectedMessageReceived();
    }
}

void Client::Impl::sendAuthMessage()
{
    Tag tag = m_tag_issuer.issue();
    Fid afid = constant::NOFID;

    std::string uname_utf8 = convertWstringToUtf8(m_config.uname);
    std::string aname_utf8 = convertWstringToUtf8(m_config.aname);

    m_tx_msg_builder.buildTAuth(tag, afid, uname_utf8, aname_utf8);
    sendMessageInTxBuffer();
}

void Client::Impl::sendAttachMessage(Fid fid)
{
    Tag tag = m_tag_issuer.issue();
    Fid afid = static_cast<Fid>(-1);

    std::string uname_utf8 = convertWstringToUtf8(m_config.uname);
    std::string aname_utf8 = convertWstringToUtf8(m_config.aname);

    m_tx_msg_builder.buildTAttach(tag, fid, afid, uname_utf8, aname_utf8);
    sendMessageInTxBuffer();
}

void Client::Impl::sendMessageInTxBuffer()
{
    std::string_view buffer = m_tx_message.getData();

    int res = send(m_socket, buffer.data(), (int)buffer.size(), 0);
    if (res == SOCKET_ERROR) {
        spdlog::error(L"Send failed. Error status: {}", WSAGetLastError());
        throw SendFailed();
    }
}

std::string Client::Impl::readIncomingMessage()
{
    MsgLength message_length = peekForMessageLength(m_socket);
    return readData(message_length);
}

ParsedRMessage Client::Impl::readParseIncomingMessage()
{
    std::string incoming_msg = readIncomingMessage();
    std::string_view incoming_msg_view(incoming_msg.data(), incoming_msg.size());
    return parseMessage(incoming_msg_view);
}

std::string Client::Impl::readData(MsgLength msg_length)
{
    std::string incoming_buf(msg_length, '\0');
    int res = recv(m_socket, incoming_buf.data(), msg_length, 0);
    validateRecvResult(res);

    return incoming_buf;
}

Client::Client(const ClientConfiguration &config) : m_i(new Impl(config))
{}

// The empty desctructor is needed because the header file does not provide enough information for one to delete the
// Impl object. We need to define the destructor in the source file, even though there is nothing specific to be
// recorded in it.
Client::~Client()
{}
