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

class SocketSetOptionFailed : public ClientException
{
public:
    SocketSetOptionFailed(int error) : m_error(error)
    {}

    const char *what() const noexcept override
    {
        return "Socket Set Option Failed";
    }

    int getError() const noexcept
    {
        return m_error;
    }

private:
    int m_error;
};

class SocketCreationFailed : public ClientException
{
public:
    SocketCreationFailed(int err) : m_error(err)
    {}

    const char *what() const noexcept override
    {
        return "Socket creation failed";
    }

    int getError() const noexcept
    {
        return m_error;
    }

private:
    int m_error;
};

class SocketConnectionFailed : public ClientException
{
public:
    SocketConnectionFailed(int error) : m_error(error)
    {}

    const char* what() const noexcept override
    {
        return "Socket Connection Failed";
    }

    int getError() const noexcept
    {
        return m_error;
    }

private:
    int m_error;
};
