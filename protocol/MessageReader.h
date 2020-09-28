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

#include <variant>
#include <vector>

#include "DataTypes.h"

struct ParsedRVersion
{
	Tag tag{};
	uint32_t msize{};
	std::string_view version;
};

struct ParsedRAuth
{
	Tag tag;
	Qid aqid;
};

struct ParsedRError
{
	Tag tag;
	std::string_view ename;
};

struct ParsedRFlush
{
	Tag tag;
};

struct ParsedRAttach
{
	Tag tag;
	Qid qid;
};

struct ParsedRWalk
{
	Tag tag;
	std::vector<Qid> wqids;
};

struct ParsedROpen
{
	Tag tag;
	Qid qid;
	uint32_t iounit;
};

struct ParsedRCreate
{
	Tag tag;
	Qid qid;
	uint32_t iounit;
};

struct ParsedRRead
{
	Tag tag;
	std::string_view data;
};

struct ParsedRWrite
{
	Tag tag;
	uint32_t count;
};

struct ParsedRClunk
{
	Tag tag;
};

struct ParsedRRemove
{
	Tag tag;
};

struct ParsedRStat
{
	Tag tag;
	RStat stat;
};

struct ParsedRWstat
{
	Tag tag;
};


typedef std::variant<ParsedRVersion, ParsedRAuth, ParsedRError, ParsedRFlush, ParsedRAttach, ParsedRWalk, ParsedROpen,
	ParsedRCreate, ParsedRRead, ParsedRWrite, ParsedRClunk, ParsedRRemove, ParsedRStat, ParsedRWstat> ParsedRMessage;

ParsedRMessage parseMessage(const std::string_view& msg);
