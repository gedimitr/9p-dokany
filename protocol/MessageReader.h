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

#include <string_view>
#include <variant>
#include <vector>

#include "DataTypes.h"

struct ParsedRVersion
{
	ParsedRVersion(uint32_t msize, std::string_view version) :
		msize(msize), version(version) { }

	uint32_t msize;
	std::string_view version;
};

struct ParsedRAuth
{
	ParsedRAuth(Qid aqid) :
		aqid(aqid) { }

	Qid aqid;
};

struct ParsedRError
{
	ParsedRError(std::string_view ename) :
		ename(ename) { }

	std::string_view ename;
};

struct ParsedRFlush
{ };

struct ParsedRAttach
{
	ParsedRAttach(Qid qid) :
		qid(qid) { }

	Qid qid;
};

struct ParsedRWalk
{
	ParsedRWalk(std::vector<Qid> wqids) :
		wqids(wqids) { }

	std::vector<Qid> wqids;
};

struct ParsedROpen
{
	ParsedROpen(Qid qid, uint32_t iounit) :
		qid(qid), iounit(iounit) { }

	Qid qid;
	uint32_t iounit;
};

struct ParsedRCreate
{
	ParsedRCreate(Qid qid, uint32_t iounit) :
		qid(qid), iounit(iounit) { }

	Qid qid;
	uint32_t iounit;
};

struct ParsedRRead
{
	ParsedRRead(std::string_view data) :
		data(data) { }

	std::string_view data;
};

struct ParsedRWrite
{
	ParsedRWrite(uint32_t count) :
		count(count) { }

	uint32_t count;
};

struct ParsedRClunk
{ };

struct ParsedRRemove
{ };

struct ParsedRStat
{
	RStat stat;
};

struct ParsedRWstat
{ };


typedef std::variant<ParsedRVersion, ParsedRAuth, ParsedRError, ParsedRFlush, ParsedRAttach, ParsedRWalk, ParsedROpen,
	ParsedRCreate, ParsedRRead, ParsedRWrite, ParsedRClunk, ParsedRRemove, ParsedRStat, ParsedRWstat> ParsedRMessagePayload;

struct ParsedRMessage
{
	ParsedRMessage(Tag tag, ParsedRMessagePayload payload) :
		tag(tag), payload(payload) { }

	Tag tag;
	ParsedRMessagePayload payload;
};

ParsedRMessage parseMessage(std::string_view msg);
