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
#include "FileMode.h"

#include <cassert>

#include "spdlog/spdlog.h"

namespace {

const uint8_t OTRUNC_FLAG = 0x10;
const uint8_t ORCLOSE_FLAG = 0x40;

uint8_t encodeFileModeAccess(FileMode::Access access)
{
    switch (access) {
    case FileMode::Access::Read:
        return 0;
    case FileMode::Access::Write:
        return 1;
    case FileMode::Access::ReadWrite:
        return 2;
    default:
        assert(access == FileMode::Access::Execute);
        return 3;
    }
}

FileMode::Access decodeFileModeAccess(uint8_t value)
{
    uint8_t access_part = value & 0x03;
    switch (access_part) {
    case 0x00:
        return FileMode::Access::Read;
    case 0x01:
        return FileMode::Access::Write;
    case 0x02:
        return FileMode::Access::ReadWrite;
    default:
        assert(access_part == 0x03);
        return FileMode::Access::Execute;
    }
}

} // namespace

FileMode::FileMode(Access access) : access(access)
{}

uint8_t FileMode::encode() const
{
    uint8_t value = encodeFileModeAccess(access);

    if (truncate) {
        value |= OTRUNC_FLAG;
    }

    if (remove_on_close) {
        value |= ORCLOSE_FLAG;
    }

    return value;
}

FileMode decode(uint8_t value)
{
    FileMode::Access access = decodeFileModeAccess(value);
    FileMode file_mode(access);

    file_mode.truncate = value & OTRUNC_FLAG;
    file_mode.remove_on_close = value & ORCLOSE_FLAG;

    return file_mode;
}
