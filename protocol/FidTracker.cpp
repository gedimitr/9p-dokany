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
 */#include "FidTracker.h"

#include <cassert>

void FidTracker::setRoot(Fid fid, Qid qid)
{
    std::vector<std::string> wnames;
    m_root_entry.emplace(fid, wnames, qid);
}

void FidTracker::addFid(Fid fid, const std::vector<std::string> &wnames, Qid qid)
{
    m_fid_entries.emplace_back(fid, wnames, qid);
}

const FidEntry *FidTracker::getRootEntry() const
{
    if (m_root_entry) {
        return &(m_root_entry.value());
    } else {
        return nullptr;
    }
}

const FidEntry *FidTracker::findEntry(Fid fid) const
{
    for (const auto &run_entry : m_fid_entries) {
        if (run_entry.fid == fid) {
            return &run_entry;
        }
    }

    return nullptr;
}
