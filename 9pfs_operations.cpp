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
#include "9pfs_operations.h"

#include "spdlog/spdlog.h"

#include "protocol/Client.h"
#include "utils/TextUtilities.h"

namespace {

inline Client *getContextClient(DOKAN_FILE_INFO *dokan_file_info)
{
    uint64_t context_value = dokan_file_info->DokanOptions->GlobalContext;
    return reinterpret_cast<Client *>(context_value);
}

NTSTATUS DOKAN_CALLBACK ninepfs_createfile(LPCWSTR FileName, PDOKAN_IO_SECURITY_CONTEXT SecurityContext,
                                           ACCESS_MASK DesiredAccess, ULONG FileAttributes, ULONG ShareAccess,
                                           ULONG CreateDisposition, ULONG CreateOptions,
                                           PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"CreateFile: {}", FileName);

    std::wstring filename_str = FileName;
    if (filename_str == L"\\System Volume Information" || filename_str == L"\\$RECYCLE.BIN") {
        return STATUS_NO_SUCH_FILE;
    }

    return STATUS_SUCCESS;
}

void DOKAN_CALLBACK ninepfs_cleanup(LPCWSTR FileName, PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"Cleanup: {}", FileName);
}

void DOKAN_CALLBACK ninepfs_closeFile(LPCWSTR FileName, PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"CloseFile: {}", FileName);
}

NTSTATUS DOKAN_CALLBACK ninepfs_readfile(LPCWSTR file_name, LPVOID buffer, DWORD buffer_length, LPDWORD read_length,
                                         LONGLONG offset, PDOKAN_FILE_INFO dokan_file_info)
{
    spdlog::info(L"ReadFile: {}, buffer_length: {}, read_length: {}, offset: {}", file_name, buffer_length, *read_length, offset);
    Client *ninep_client = getContextClient(dokan_file_info);
    *read_length = ninep_client->readFile(file_name, offset, buffer, buffer_length);

    if (*read_length > 0) {
        return STATUS_SUCCESS;
    } else if (read_length == 0) {
        return STATUS_END_OF_FILE;
    } else {
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }
}

NTSTATUS DOKAN_CALLBACK ninepfs_writefile(LPCWSTR FileName, LPCVOID Buffer, DWORD NumberOfBytesToWrite,
                                          LPDWORD NumberOfBytesWritten, LONGLONG Offset,
                                          PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"WriteFile: {}", FileName);
    return STATUS_SUCCESS;
}

NTSTATUS DOKAN_CALLBACK ninepfs_flushfilebuffers(LPCWSTR FileName, PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"FlushFileBuffers: {}", FileName);
    return STATUS_SUCCESS;
}

void splitInt64(uint64_t input, DWORD *high, DWORD *low)
{
    *high = static_cast<DWORD>(input >> 32);
    *low = static_cast<DWORD>(input & 0xffffffff);
}

void storeTimestampIntoFiletime(uint64_t input, FILETIME *filetime)
{
    const uint64_t epoch_diff = 11'644'473'600;

    input += epoch_diff;
    input *= 10'000'000;
    splitInt64(input, &(filetime->dwHighDateTime), &(filetime->dwLowDateTime));
}

void fillByHandleFileInformation(const RStat &rstat, BY_HANDLE_FILE_INFORMATION *by_handle_file_information)
{
    by_handle_file_information->dwFileAttributes =
        (rstat.qid.type & 0x80) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
    storeTimestampIntoFiletime(rstat.mtime, &by_handle_file_information->ftCreationTime);
    storeTimestampIntoFiletime(rstat.mtime, &by_handle_file_information->ftLastWriteTime);
    storeTimestampIntoFiletime(rstat.atime, &by_handle_file_information->ftLastAccessTime);
    by_handle_file_information->dwVolumeSerialNumber = 0x11223344;

    splitInt64(rstat.length, &by_handle_file_information->nFileSizeHigh, &by_handle_file_information->nFileSizeLow);
    by_handle_file_information->nNumberOfLinks = 1;
    splitInt64(rstat.qid.path, &by_handle_file_information->nFileIndexHigh,
               &by_handle_file_information->nFileIndexLow);
}

NTSTATUS DOKAN_CALLBACK ninepfs_getfileInformation(LPCWSTR file_name, LPBY_HANDLE_FILE_INFORMATION buffer,
                                                   PDOKAN_FILE_INFO dokan_file_info)
{
    spdlog::info(L"GetFileInformation: {}", file_name);

    Client *ninep_client = getContextClient(dokan_file_info);
    std::optional<RStat> rstat = ninep_client->getFileInformation(file_name);

    if (rstat) {
        fillByHandleFileInformation(*rstat, buffer);
        return STATUS_SUCCESS;
    } else {
        return STATUS_FILE_NOT_AVAILABLE;
    }
}

void fillFindDataWithRStat(const RStat &rstat, PFillFindData fill_find_data, PDOKAN_FILE_INFO dokan_file_info)
{
    WIN32_FIND_DATAW find_data;

    find_data.dwFileAttributes = (rstat.qid.type & 0x80) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
    copyUtf8StringToWcharArr(rstat.name, find_data.cFileName, MAX_PATH);
    storeTimestampIntoFiletime(rstat.mtime, &find_data.ftCreationTime);
    storeTimestampIntoFiletime(rstat.mtime, &find_data.ftLastWriteTime);
    storeTimestampIntoFiletime(rstat.atime, &find_data.ftLastAccessTime);

    splitInt64(rstat.length, &find_data.nFileSizeHigh, &find_data.nFileSizeLow);

    fill_find_data(&find_data, dokan_file_info);
}

NTSTATUS DOKAN_CALLBACK ninepfs_findfiles(LPCWSTR FileName, PFillFindData FillFindData, PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"FindFiles: {}", FileName);

    Client *ninep_client = getContextClient(DokanFileInfo);
    std::vector<RStat> rstats = ninep_client->getDirectoryContents(FileName);
    spdlog::debug(L"9P Client returned {} RStat entities as directory contents", rstats.size());

    for (const RStat &run_rstat : rstats) {
        fillFindDataWithRStat(run_rstat, FillFindData, DokanFileInfo);
    }

    return STATUS_SUCCESS;
}

NTSTATUS DOKAN_CALLBACK ninepfs_setfileattributes(LPCWSTR FileName, DWORD FileAttributes,
                                                  PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"SetFileAttributes: {}", FileName);
    return STATUS_SUCCESS;
}

NTSTATUS DOKAN_CALLBACK ninepfs_setfiletime(LPCWSTR FileName, CONST FILETIME *CreationTime,
                                            CONST FILETIME *LastAccessTime, CONST FILETIME *LastWriteTime,
                                            PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"SetFileTime: {}", FileName);
    return STATUS_SUCCESS;
}

NTSTATUS DOKAN_CALLBACK ninepfs_deletefile(LPCWSTR FileName, PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"DeleteFile: {}", FileName);
    return STATUS_SUCCESS;
}

NTSTATUS DOKAN_CALLBACK ninepfs_deletedirectory(LPCWSTR FileName, PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"DeleteDirectory: {}", FileName);
    return STATUS_SUCCESS;
}

NTSTATUS DOKAN_CALLBACK ninepfs_movefile(LPCWSTR FileName, LPCWSTR NewFileName, BOOL ReplaceIfExisting,
                                         PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"MoveFile: {} to {}", FileName, NewFileName);
    return STATUS_SUCCESS;
}

NTSTATUS DOKAN_CALLBACK ninepfs_setendoffile(LPCWSTR FileName, LONGLONG ByteOffset, PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"SetEndOfFile: {} ByteOffset {}", FileName, ByteOffset);
    return STATUS_SUCCESS;
}

NTSTATUS DOKAN_CALLBACK ninepfs_setallocationsize(LPCWSTR FileName, LONGLONG AllocSize, PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"SetAllocationSize: {} AllocSize {}", FileName, AllocSize);
    return STATUS_SUCCESS;
}

NTSTATUS DOKAN_CALLBACK ninepfs_lockfile(LPCWSTR FileName, LONGLONG ByteOffset, LONGLONG Length,
                                         PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"LockFile: {} ByteOffset {} Length {}", FileName, ByteOffset, Length);
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS DOKAN_CALLBACK ninepfs_unlockfile(LPCWSTR FileName, LONGLONG ByteOffset, LONGLONG Length,
                                           PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"UnlockFile: {} ByteOffset {} Length {}", FileName, ByteOffset, Length);
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS DOKAN_CALLBACK ninepfs_getdiskfreespace(PULONGLONG FreeBytesAvailable, PULONGLONG TotalNumberOfBytes,
                                                 PULONGLONG TotalNumberOfFreeBytes, PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"GetDiskFreeSpace");
    *FreeBytesAvailable = (ULONGLONG)(512 * 1024 * 1024);
    *TotalNumberOfBytes = MAXLONGLONG;
    *TotalNumberOfFreeBytes = MAXLONGLONG;
    return STATUS_SUCCESS;
}

NTSTATUS DOKAN_CALLBACK ninepfs_getvolumeinformation(LPWSTR VolumeNameBuffer, DWORD VolumeNameSize,
                                                     LPDWORD VolumeSerialNumber, LPDWORD MaximumComponentLength,
                                                     LPDWORD FileSystemFlags, LPWSTR FileSystemNameBuffer,
                                                     DWORD FileSystemNameSize, PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"GetVolumeInformation");
    wcscpy_s(VolumeNameBuffer, VolumeNameSize, L"DM FS");
    *VolumeSerialNumber = 0x11223344;
    *MaximumComponentLength = 255;
    *FileSystemFlags = FILE_CASE_SENSITIVE_SEARCH | FILE_CASE_PRESERVED_NAMES | FILE_SUPPORTS_REMOTE_STORAGE |
                       FILE_UNICODE_ON_DISK | FILE_NAMED_STREAMS;

    wcscpy_s(FileSystemNameBuffer, FileSystemNameSize, L"NTFS");
    return STATUS_SUCCESS;
}

NTSTATUS DOKAN_CALLBACK ninepfs_mounted(PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"Mounted");
    return STATUS_SUCCESS;
}

NTSTATUS DOKAN_CALLBACK ninepfs_unmounted(PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"Unmounted");
    return STATUS_SUCCESS;
}

NTSTATUS DOKAN_CALLBACK ninepfs_getfilesecurity(LPCWSTR FileName, PSECURITY_INFORMATION SecurityInformation,
                                                PSECURITY_DESCRIPTOR SecurityDescriptor, ULONG BufferLength,
                                                PULONG LengthNeeded, PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"GetFileSecurity: {}", FileName);
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS DOKAN_CALLBACK ninepfs_setfilesecurity(LPCWSTR FileName, PSECURITY_INFORMATION SecurityInformation,
                                                PSECURITY_DESCRIPTOR SecurityDescriptor, ULONG BufferLength,
                                                PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"SetFileSecurity: {}", FileName);
    return STATUS_SUCCESS;
}

NTSTATUS DOKAN_CALLBACK ninepfs_findstreams(LPCWSTR FileName, PFillFindStreamData FillFindStreamData,
                                            PDOKAN_FILE_INFO DokanFileInfo)
{
    spdlog::info(L"FindStreams: {}", FileName);
    return STATUS_SUCCESS;
}

} // anonymous namespace

DOKAN_OPERATIONS ninepfs_operations = {ninepfs_createfile,
                                       ninepfs_cleanup,
                                       ninepfs_closeFile,
                                       ninepfs_readfile,
                                       ninepfs_writefile,
                                       ninepfs_flushfilebuffers,
                                       ninepfs_getfileInformation,
                                       ninepfs_findfiles,
                                       nullptr, // FindFilesWithPattern
                                       ninepfs_setfileattributes,
                                       ninepfs_setfiletime,
                                       ninepfs_deletefile,
                                       ninepfs_deletedirectory,
                                       ninepfs_movefile,
                                       ninepfs_setendoffile,
                                       ninepfs_setallocationsize,
                                       ninepfs_lockfile,
                                       ninepfs_unlockfile,
                                       ninepfs_getdiskfreespace,
                                       ninepfs_getvolumeinformation,
                                       ninepfs_mounted,
                                       ninepfs_unmounted,
                                       ninepfs_getfilesecurity,
                                       ninepfs_setfilesecurity,
                                       ninepfs_findstreams};
