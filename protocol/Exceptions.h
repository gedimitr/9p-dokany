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

#include <exception>

class ParsingException : public std::exception
{
public:
    const char *what() const noexcept override
    {
        return "Parsing Exception";
    }
};

class BufferOverrun : public ParsingException
{
public:
    const char *what() const noexcept override
    {
        return "Buffer Overrun";
    }
};

class UnknownMessageTag : public ParsingException
{
public:
    const char *what() const noexcept override
    {
        return "Unknown Message Tag";
    }
};

class ClientException : public std::exception
{
public:
    const char *what() const noexcept = 0;
};

class WinsockInitializationFailed : public ClientException
{
public:
    const char *what() const noexcept override
    {
        return "Winsock Initialization Failed";
    }
};

class ClientInitializationError : public ClientException
{
public:
    const char *what() const noexcept override
    {
        return "Client Initialization Error";
    }
};

class ConnectionFailed : public ClientException
{
public:
    const char *what() const noexcept override
    {
        return "Connection could not be established";
    }
};

class VersionHandshakeError : public ClientException
{
public:
    const char *what() const noexcept override
    {
        return "Version Handshake failed";
    }
};

class ConnectionClosed : public ClientException
{
public:
    const char* what() const noexcept override
    {
        return "Connection Closed";
    }
};

class RecvFailed : public ClientException
{
public:
    const char* what() const noexcept override
    {
        return "Receive Failed";
    }
};

class SendFailed : public ClientException
{
public:
    const char* what() const noexcept override
    {
        return "Send Failed";
    }
};

class UnexpectedMessageReceived : public ClientException
{
public:
    const char* what() const noexcept override
    {
        return "Unexpected Message Received";
    }
};

class ErrorMessageReceived : public ClientException
{
public:
    const char *what() const noexcept override
    {
        return "Error Message Received";
    }
};

class ServerRequestedAuthentication : public ClientException
{
public:
    const char* what() const noexcept override
    {
        return "Server Requested Authentication";
    }
};
