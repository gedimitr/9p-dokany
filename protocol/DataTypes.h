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
#include <string>

typedef uint32_t MsgLength;

typedef uint8_t MsgType;
typedef uint16_t Tag;
typedef uint32_t Fid;

struct Qid
{
    Qid(uint8_t type, uint32_t vers, uint64_t path) : type(type), vers(vers), path(path)
    {}

    uint8_t type;
    uint32_t vers;
    uint64_t path;
};

template <typename StringType>
struct StatTemplate
{
    uint16_t type = 0;
    uint32_t dev = 0;
    Qid qid{0, 0, 0};
    uint32_t mode = 0;
    uint32_t atime = 0;
    uint32_t mtime = 0;
    uint64_t length = 0;
    StringType name;
    StringType uid;
    StringType gid;
    StringType muid;
};

typedef StatTemplate<std::string> TStat;
typedef StatTemplate<std::string> RStat;