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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
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

namespace msg_tag {

constexpr MsgTag TVersion = 100;
constexpr MsgTag RVersion = 101;
constexpr MsgTag TAuth = 102;
constexpr MsgTag RAuth = 103;
constexpr MsgTag TAttach = 104;
constexpr MsgTag RAttach = 105;
constexpr MsgTag RError = 107;
constexpr MsgTag TFlush = 108;
constexpr MsgTag RFlush = 109;
constexpr MsgTag TWalk = 110;
constexpr MsgTag RWalk = 111;
constexpr MsgTag TOpen = 112;
constexpr MsgTag ROpen = 113;
constexpr MsgTag TCreate = 114;
constexpr MsgTag RCreate = 115;
constexpr MsgTag TRead = 116;
constexpr MsgTag RRead = 117;
constexpr MsgTag TWrite = 118;
constexpr MsgTag RWrite = 119;
constexpr MsgTag TClunk = 120;
constexpr MsgTag RClunk = 121;
constexpr MsgTag TRemove = 122;
constexpr MsgTag RRemove = 123;
constexpr MsgTag TStat = 124;
constexpr MsgTag RStat = 125;
constexpr MsgTag TWStat = 126;
constexpr MsgTag RWStat = 127;

}