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

#include <optional>
#include <string>
#include <vector>

#include "DataTypes.h"

struct FidEntry
{
    FidEntry(Fid fid, const std::vector<std::string> &wnames, Qid qid) : fid(fid), wnames(wnames), qid(qid)
    {}

    Fid fid;
    std::vector<std::string> wnames;
    Qid qid;
};

class FidTracker
{
public:
    void setRoot(Fid fid, Qid qid);
    void addFid(Fid fid, const std::vector<std::string> &wnames, Qid qid);

    const FidEntry *getRootEntry() const;
    const FidEntry *findEntry(Fid fid) const;

private:
    std::optional<FidEntry> m_root_entry;
    std::vector<FidEntry> m_fid_entries;
};