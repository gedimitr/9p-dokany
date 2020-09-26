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

#include "DataTypes.h"

#include <string_view>

class TxMessage
{
public:
	TxMessage(int capacity);
	~TxMessage();

	TxMessage(const TxMessage&) = delete;
	TxMessage& operator=(const TxMessage&) = delete;

	void initialize(MsgType msg_tag);

	void writeOctet(uint8_t value);

	template <typename T>
	void writeInteger(T value);
	
	void writeRawData(const std::string_view &data);
	void writeString(const std::string_view& str);

	bool hasRoomFor(size_t num_bytes) const;

    std::string_view getData() const;

private:
	void resetCursor();

	char* m_buffer;
	char* m_buffer_end;

	char* m_cursor;
};
