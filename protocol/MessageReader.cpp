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
#include "MessageReader.h"

#include "DataTypes.h"
#include "Exceptions.h"
#include "MessageTypes.h"

namespace {

static void checkForBufferOverrun(const std::string_view &buf, size_t bytes)
{
    if (buf.size() < bytes) {
        throw BufferOverrun();
    }
}

template <typename T>
inline T parseInteger(std::string_view &buffer)
{
    static_assert(std::is_integral_v<T>, "Function parses only integers");

    checkForBufferOverrun(buffer, sizeof(T));

    T value = 0;
    for (int i = 0; i < sizeof(T); i++) {
        char byte = buffer[i];
        value = (value << 8) | byte;
    }

    buffer.remove_prefix(sizeof(T));

    return value;
}

std::string_view extractDataView(size_t count, std::string_view &buffer)
{
    std::string_view extracted_data(buffer.data(), count);
    buffer.remove_prefix(count);

    return extracted_data;
}

std::string_view parseString(std::string_view &buffer)
{
    uint16_t string_size = parseInteger<uint16_t>(buffer);

    checkForBufferOverrun(buffer, string_size);
    return extractDataView(string_size, buffer);
}

Qid parseQid(std::string_view &buffer)
{
    uint8_t type = parseInteger<uint8_t>(buffer);
    uint32_t vers = parseInteger<uint32_t>(buffer);
    uint64_t path = parseInteger<uint64_t>(buffer);

    return Qid(type, vers, path);
}

ParsedRVersion parseRVersion(std::string_view &buffer)
{
    uint32_t msize = parseInteger<uint32_t>(buffer);
    std::string_view version = parseString(buffer);

    return ParsedRVersion(msize, version);
}

ParsedRAuth parseRAuth(std::string_view &buffer)
{
    Qid aqid = parseQid(buffer);

    return ParsedRAuth(aqid);
}

ParsedRAttach parseRAttach(std::string_view &buffer)
{
    Qid qid = parseQid(buffer);

    return ParsedRAttach(qid);
}

ParsedRError parseRError(std::string_view &buffer)
{
    std::string_view ename = parseString(buffer);

    return ParsedRError(ename);
}

std::vector<Qid> parseQids(uint16_t nwqid, std::string_view &buffer)
{
    std::vector<Qid> wqids;

    for (int i = 0; i < nwqid; i++) {
        wqids.emplace_back(parseQid(buffer));
    }

    return wqids;
}

ParsedRWalk parseRWalk(std::string_view &buffer)
{
    uint16_t nwqid = parseInteger<uint16_t>(buffer);
    std::vector<Qid> wqids = parseQids(nwqid, buffer);

    return ParsedRWalk(wqids);
}

ParsedROpen parseROpen(std::string_view &buffer)
{
    Qid qid = parseQid(buffer);
    uint32_t iounit = parseInteger<uint32_t>(buffer);

    return ParsedROpen(qid, iounit);
}

ParsedRCreate parseRCreate(std::string_view &buffer)
{
    Qid qid = parseQid(buffer);
    uint32_t iounit = parseInteger<uint32_t>(buffer);

    return ParsedRCreate(qid, iounit);
}

ParsedRRead parseRRead(std::string_view &buffer)
{
    uint32_t count = parseInteger<uint32_t>(buffer);
    std::string_view data = extractDataView(count, buffer);

    return ParsedRRead(data);
}

ParsedRWrite parseRWrite(std::string_view &buffer)
{
    uint32_t count = parseInteger<uint32_t>(buffer);

    return ParsedRWrite(count);
}

ParsedRStat parseRStat(std::string_view &buffer)
{
    RStat stat;

    uint16_t size = parseInteger<uint16_t>(buffer);
    std::string_view stat_buffer = extractDataView(size, buffer);

    stat.type = parseInteger<uint16_t>(stat_buffer);
    stat.dev = parseInteger<uint32_t>(stat_buffer);
    stat.qid = parseQid(stat_buffer);
    stat.mode = parseInteger<uint32_t>(stat_buffer);
    stat.atime = parseInteger<uint32_t>(stat_buffer);
    stat.mtime = parseInteger<uint32_t>(stat_buffer);
    stat.length = parseInteger<uint64_t>(stat_buffer);
    stat.name = parseString(stat_buffer);
    stat.uid = parseString(stat_buffer);
    stat.gid = parseString(stat_buffer);
    stat.muid = parseString(stat_buffer);

    return ParsedRStat(stat);
}

} // namespace

ParsedRMessagePayload parseMessagePayload(MsgType type, std::string_view buffer)
{
    switch (type) {
    case msg_type::RVersion:
        return parseRVersion(buffer);
    case msg_type::RAuth:
        return parseRAuth(buffer);
    case msg_type::RError:
        return parseRError(buffer);
    case msg_type::RFlush:
        return ParsedRFlush();
    case msg_type::RAttach:
        return parseRAttach(buffer);
    case msg_type::RWalk:
        return parseRWalk(buffer);
    case msg_type::ROpen:
        return parseROpen(buffer);
    case msg_type::RCreate:
        return parseRCreate(buffer);
    case msg_type::RRead:
        return parseRRead(buffer);
    case msg_type::RWrite:
        return parseRWrite(buffer);
    case msg_type::RClunk:
        return ParsedRClunk();
    case msg_type::RRemove:
        return ParsedRRemove();
    case msg_type::RStat:
        return parseRStat(buffer);
    case msg_type::RWStat:
        return ParsedRWstat();
    default:
        throw UnknownMessageTag();
    }
}

ParsedRMessage parseMessage(std::string_view buffer)
{
    MsgLength msg_length = parseInteger<MsgLength>(buffer);

    std::string_view msg_data = extractDataView(msg_length, buffer);
    MsgType type = parseInteger<MsgType>(msg_data);
    Tag tag = parseInteger<Tag>(msg_data);
    ParsedRMessagePayload payload = parseMessagePayload(type, msg_data);

    return ParsedRMessage(tag, payload);
}
