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

namespace msg_type {

constexpr MsgType TVersion = 100;
constexpr MsgType RVersion = 101;
constexpr MsgType TAuth = 102;
constexpr MsgType RAuth = 103;
constexpr MsgType TAttach = 104;
constexpr MsgType RAttach = 105;
constexpr MsgType RError = 107;
constexpr MsgType TFlush = 108;
constexpr MsgType RFlush = 109;
constexpr MsgType TWalk = 110;
constexpr MsgType RWalk = 111;
constexpr MsgType TOpen = 112;
constexpr MsgType ROpen = 113;
constexpr MsgType TCreate = 114;
constexpr MsgType RCreate = 115;
constexpr MsgType TRead = 116;
constexpr MsgType RRead = 117;
constexpr MsgType TWrite = 118;
constexpr MsgType RWrite = 119;
constexpr MsgType TClunk = 120;
constexpr MsgType RClunk = 121;
constexpr MsgType TRemove = 122;
constexpr MsgType RRemove = 123;
constexpr MsgType TStat = 124;
constexpr MsgType RStat = 125;
constexpr MsgType TWStat = 126;
constexpr MsgType RWStat = 127;

}