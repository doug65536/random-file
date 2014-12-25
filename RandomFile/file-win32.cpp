#include "file-win32.h"

#include "utfhelpers.h"

IFile *IFile::create()
{
    return new FileWin32;
}

IFile::file_pointer FileWin32::size()
{
    LARGE_INTEGER saveOfs;
    saveOfs.HighPart = 0;
    saveOfs.LowPart = SetFilePointer(
                file, saveOfs.LowPart,
                &saveOfs.HighPart, FILE_CURRENT);
    if (saveOfs.LowPart == INVALID_SET_FILE_POINTER &&
        GetLastError() != NO_ERROR)
        return false;

    LARGE_INTEGER result;
    result.HighPart = 0;
    result.LowPart = 0;
    result.LowPart = SetFilePointer(file, result.LowPart, &result.HighPart, FILE_END);
    if (result.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
        return -1;

    // Using QuadPart is technically UB here
    return (std::int64_t(result.HighPart) << 32) | result.LowPart;
}

bool FileWin32::read(char *data, std::size_t count) noexcept
{
    DWORD didread = 0;
    return ReadFile(file, data, count, &didread, NULL) &&
            didread == count;
}

bool FileWin32::write(const char *data, std::size_t count) noexcept
{
    DWORD wrote = 0;
    return WriteFile(file, data, count, &wrote, NULL) &&
            wrote == count;
}

FileWin32::file_pointer FileWin32::seek(file_pointer p, int rel) noexcept
{
    LARGE_INTEGER ofs;
    ofs.LowPart = LONG(p);
    ofs.HighPart = LONG(p >> 32);
    LARGE_INTEGER result;
    result.HighPart = ofs.HighPart;
    result.LowPart = SetFilePointer(
                file,
                ofs.LowPart,
                &result.HighPart,
                rel < 0 ? FILE_BEGIN :
                rel > 0 ? FILE_END :
                FILE_CURRENT);
    if (result.LowPart == INVALID_SET_FILE_POINTER &&
            GetLastError() != NO_ERROR)
        return -1;
    return (file_pointer(result.HighPart) << 32) | result.LowPart;
}

FileWin32::FileWin32()
    : file(INVALID_HANDLE_VALUE)
{
}

FileWin32::~FileWin32()
{
    close();
}

bool FileWin32::open(std::string const& filename) noexcept
{
    file = CreateFileW(utf8ToUtf16(filename).c_str(),
                       GENERIC_READ, FILE_SHARE_READ,
                       NULL, OPEN_ALWAYS,
                       FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    return file != INVALID_HANDLE_VALUE;
}

bool FileWin32::close() noexcept
{
    if (file != INVALID_HANDLE_VALUE)
    {
        bool result = (CloseHandle(file) != FALSE);
        file = INVALID_HANDLE_VALUE;
        return result;
    }
    return true;
}
