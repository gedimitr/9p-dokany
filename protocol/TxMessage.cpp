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
#include "TxMessage.h"

#include <cassert>
#include <cstring>
#include <limits>
#include <type_traits>

namespace {

template <typename T>
inline char extractLowEndianByte(T value, int pos)
{
    static_assert(std::is_integral_v<T>, "Integer type required as first argument");
    assert(pos < sizeof(T));

    // When signed integers are right shifted, behaviour is implementation dependent. However, since we are not going
    // to use any of the bits that will be shifted in, it doesn't matter if arithmetic / logical shifting is done.

    int shift_bits = 8 * pos;
    return (value >> shift_bits) & 0xff;
}

template <typename T>
inline void writeLowEndianInteger(T value, char *buffer)
{
    static_assert(std::is_integral_v<T>, "Integer type required as first argument");

    for (int i = 0; i < sizeof(T); i++) {
        char byte = extractLowEndianByte(value, i);
        *buffer++ = byte;
    }
}

} // anonymous namespace

TxMessage::TxMessage(int capacity) : m_buffer(new char[capacity]), m_buffer_end(m_buffer + capacity)
{
    resetCursor();
}

TxMessage::~TxMessage()
{
    delete[] m_buffer;
}

void TxMessage::initialize(MsgType type)
{
    resetCursor();
    writeInteger(type);
}

void TxMessage::writeOctet(uint8_t value)
{
    *m_cursor++ = value;
}

template <typename T>
void TxMessage::writeInteger(T value)
{
    static_assert(std::is_integral_v<T>, "Integer type required as argument");

    writeLowEndianInteger(value, m_cursor);
    m_cursor += sizeof(T);
}

void TxMessage::writeRawData(const std::string_view &data)
{
    size_t data_len = data.length();

    memcpy(m_cursor, data.data(), data_len);
    m_cursor += data_len;
}

void TxMessage::writeString(const std::string_view &str)
{
    size_t str_size = str.size();
    if (!hasRoomFor(str_size) || str_size > std::numeric_limits<uint16_t>::max()) {
        return;
    }

    uint16_t str_len = static_cast<uint16_t>(str_size);
    writeInteger(str_len);
    writeRawData(str);
}

bool TxMessage::hasRoomFor(size_t num_bytes) const
{
    return m_buffer + num_bytes <= m_buffer_end;
}

std::string_view TxMessage::getData() const
{
    MsgLength size = static_cast<MsgLength>(m_cursor - m_buffer);
    writeLowEndianInteger<MsgLength>(size, m_buffer);

    return std::string_view(m_buffer, size);
}

void TxMessage::resetCursor()
{
    m_cursor = m_buffer + sizeof(MsgLength);
}

// Explicit template instantiation of writeInteger method for different integer types
template void TxMessage::writeInteger<uint8_t>(uint8_t);
template void TxMessage::writeInteger<uint16_t>(uint16_t);
template void TxMessage::writeInteger<uint32_t>(uint32_t);
template void TxMessage::writeInteger<uint64_t>(uint64_t);
