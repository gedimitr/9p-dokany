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
#include "TxMessageBuilder.h"

#include "MessageTypes.h"
#include "TxMessage.h"

TxMessageBuilder::TxMessageBuilder(TxMessage* tx_message) :
	m_tx_message(tx_message) { }

void TxMessageBuilder::buildTVersion(uint32_t msize, const std::string_view &version)
{
	m_tx_message->initialize(msg_type::TVersion);

	Tag tag = static_cast<Tag>(~0);
	m_tx_message->writeInteger(tag);
	m_tx_message->writeInteger(msize);
	m_tx_message->writeString(version);
}

void TxMessageBuilder::buildTAuth(Tag tag, Fid afid, const std::string_view& uname, const std::string_view& aname)
{
	m_tx_message->initialize(msg_type::TAuth);

	m_tx_message->writeInteger(tag);
	m_tx_message->writeInteger(afid);
	m_tx_message->writeString(uname);
	m_tx_message->writeString(aname);
}

void TxMessageBuilder::buildTFlush(Tag tag, Tag oldtag)
{
	m_tx_message->initialize(msg_type::TFlush);

	m_tx_message->writeInteger(tag);
	m_tx_message->writeInteger(oldtag);
}

void TxMessageBuilder::buildTAttach(Tag tag, Fid fid, Fid afid, const std::string_view& uname,
									const std::string_view& aname)
{
	m_tx_message->initialize(msg_type::TAttach);

	m_tx_message->writeInteger(tag);
	m_tx_message->writeInteger(fid);
	m_tx_message->writeInteger(afid);
	m_tx_message->writeString(uname);
	m_tx_message->writeString(aname);
}

void TxMessageBuilder::buildTWalk(Tag tag, Fid fid, Fid newfid, const std::vector<std::string_view>& wnames)
{
	m_tx_message->initialize(msg_type::TWalk);

	m_tx_message->writeInteger(tag);
	m_tx_message->writeInteger(fid);
	m_tx_message->writeInteger(newfid);

	uint16_t num_wnames = static_cast<uint16_t>(wnames.size());
	m_tx_message->writeInteger(num_wnames);
	for (auto const& run_wname : wnames) {
		m_tx_message->writeString(run_wname);
	}
}

void TxMessageBuilder::buildTOpen(Tag tag, Fid fid, uint8_t mode)
{
	m_tx_message->initialize(msg_type::TOpen);

	m_tx_message->writeInteger(tag);
	m_tx_message->writeInteger(fid);
	m_tx_message->writeInteger(mode);
}

void TxMessageBuilder::buildTCreate(Tag tag, Fid fid, const std::string_view& name, uint32_t perm, uint8_t mode)
{
	m_tx_message->initialize(msg_type::TCreate);

	m_tx_message->writeInteger(tag);
	m_tx_message->writeInteger(fid);
	m_tx_message->writeString(name);
	m_tx_message->writeInteger(perm);
	m_tx_message->writeInteger(mode);
}

void TxMessageBuilder::buildTRead(Tag tag, Fid fid, uint64_t offset, uint32_t count)
{
	m_tx_message->initialize(msg_type::TRead);

	m_tx_message->writeInteger(tag);
	m_tx_message->writeInteger(fid);
	m_tx_message->writeInteger(offset);
	m_tx_message->writeInteger(count);
}

void TxMessageBuilder::buildTWrite(Tag tag, Fid fid, uint64_t offset, const std::string_view& data)
{
	m_tx_message->initialize(msg_type::TWrite);

	m_tx_message->writeInteger(tag);
	m_tx_message->writeInteger(fid);
	m_tx_message->writeInteger(offset);

	uint32_t count = static_cast<uint32_t>(data.length());
	m_tx_message->writeInteger(count);

	m_tx_message->writeRawData(data);
}

void TxMessageBuilder::buildTClunk(Tag tag, Fid fid)
{
	m_tx_message->initialize(msg_type::TClunk);

	m_tx_message->writeInteger(tag);
	m_tx_message->writeInteger(fid);
}

void TxMessageBuilder::buildTRemove(Tag tag, Fid fid)
{
	m_tx_message->initialize(msg_type::TRemove);

	m_tx_message->writeInteger(tag);
	m_tx_message->writeInteger(fid);
}

void TxMessageBuilder::buildTStat(Tag tag, Fid fid)
{
	m_tx_message->initialize(msg_type::TStat);

	m_tx_message->writeInteger(tag);
	m_tx_message->writeInteger(fid);
}


