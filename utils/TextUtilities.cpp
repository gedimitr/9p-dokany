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
#include "TextUtilities.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

std::wstring convertUtf8ToWstring(const std::string_view &str)
{
    if (str.size() == 0) {
        return std::wstring();
    }

    int wsize = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), NULL, 0);
    std::wstring wstr(wsize, 0);

    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), wstr.data(), wsize);
    return wstr;
}

std::string convertWstringToUtf8(const std::wstring_view &wstr)
{
    if (wstr.size() == 0) {
        return std::string();
    }

    int csize = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(csize, 0);

    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), str.data(), csize, NULL, NULL);

    return str;
}

std::vector<std::string> splitToPathComponents(const std::string &str)
{
    std::vector<std::string> path_components;
    char delim = '\\';

    size_t start = 0;
    for (size_t end = str.find(delim); end != std::string::npos; end = str.find(delim, start)) {
        std::string path_component = str.substr(start, end - start);
        if (!path_component.empty()) {
            path_components.push_back(path_component);
        }
        
        start = end + 1;
    }

    std::string last_component = str.substr(start);
    if (!last_component.empty()) {
        path_components.push_back(last_component);
    }

    return path_components;
}
