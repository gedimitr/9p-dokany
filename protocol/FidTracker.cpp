#include "FidTracker.h"

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
