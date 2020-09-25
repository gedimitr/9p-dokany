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

#include <string>

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
	
	void writeData(const char* data, uint16_t len);
	void writeString(const std::string& str);

	bool hasRoomFor(size_t num_bytes) const;

	struct Data;
    Data getData() const;

private:
	void resetCursor();

	uint8_t* m_buffer;
	uint8_t *m_buffer_end;

	uint8_t* m_cursor;
};


struct TxMessage::Data {
  Data(const uint8_t* buffer, int size);

  const uint8_t* buffer;
  int size;
};