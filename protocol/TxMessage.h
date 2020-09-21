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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
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

#include <type_traits>

#include "DataTypes.h"

class TxMessage
{
public:
	TxMessage(int capacity);
	~TxMessage();

	TxMessage(const TxMessage&) = delete;
	TxMessage& operator=(const TxMessage&) = delete;

	void initialize(MsgTag msg_tag);

	void writeOctet(uint8_t value);

	template <typename T>
	void writeInteger(T value);

private:
	void resetCursor();

	uint8_t* m_buffer;
	int m_capacity;

	uint8_t* m_cursor;
};

template<typename T>
inline void TxMessage::writeInteger(T value)
{
	static_assert(std::is_integral_v<T>, "Integer type required");
	for (int i = 0; i < sizeof(T); i++) {
		uint8_t octet = (value >> (8 * i)) & 0xff;
		writeOctet(octet);
	}
}
