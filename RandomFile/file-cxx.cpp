#include "file-cxx.h"

#include <limits>

IFile* IFile::create()
{
    return new CxxFile;
}

bool CxxFile::open(std::string const& filename) noexcept
{
    file.open(filename, std::ios::binary | std::fstream::in | std::fstream::out);
    if (!file.is_open())
        file.open(filename, std::ios::binary | std::fstream::in | std::fstream::out | std::fstream::trunc);
    return file.is_open();
}

bool CxxFile::close() noexcept
{
    file.close();
    return !file.fail();
}

IFile::file_pointer CxxFile::size() noexcept
{
    std::fstream::off_type save_off = file.tellp();
    return (std::min<decltype(save_off+std::int64_t(0))>)(save_off,
            (std::numeric_limits<std::int64_t>::max)());
}

bool CxxFile::read(char *data, std::size_t count) noexcept
{
    return !file.read(data, count).fail();
}

bool CxxFile::write(const char *data, std::size_t count) noexcept
{
    return !file.write(data, count).fail();
}

CxxFile::file_pointer CxxFile::seek(file_pointer p, int rel) noexcept
{
    std::ios::seekdir dir = rel < 0
            ? std::ios::beg
            : rel == 0
            ? std::ios::cur
            : std::ios::end;
    file.seekp(p, dir);
    file.seekg(p, dir);
    return !file.fail() ? p : -1;
}
