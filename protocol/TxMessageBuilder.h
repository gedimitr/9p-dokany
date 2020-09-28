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

#include <cstdint>
#include <string>
#include <vector>

#include "DataTypes.h"

class TxMessage;

class TxMessageBuilder
{
public:
	TxMessageBuilder(TxMessage* tx_message);

	void buildTVersion(uint32_t msize, const std::string_view &version);
	void buildTAuth(Tag tag, Fid afid, const std::string_view& uname, const std::string_view& aname);
	void buildTFlush(Tag tag, Tag oldtag);
	void buildTAttach(Tag tag, Fid fid, Fid afid, const std::string_view& uname, const std::string_view& aname);
	void buildTWalk(Tag tag, Fid fid, Fid newfid, const std::vector<std::string_view>& wnames);
	void buildTOpen(Tag tag, Fid fid, uint8_t mode);
	void buildTCreate(Tag tag, Fid fid, const std::string_view& name, uint32_t perm, uint8_t mode);
	void buildTRead(Tag tag, Fid fid, uint64_t offset, uint32_t count);
	void buildTWrite(Tag tag, Fid fid, uint64_t offset, const std::string_view& data);
	void buildTClunk(Tag tag, Fid fid);
	void buildTRemove(Tag tag, Fid fid);
	void buildTStat(Tag tag, Fid fid);
	void buildTWstat(Tag tag, Fid fid, const TStat& stat);

private:
	void writeQid(const Qid& qid);
	void writeStat(const TStat& stat);

	TxMessage* m_tx_message;
};

