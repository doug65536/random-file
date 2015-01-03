#ifndef FILEWIN32_H
#define FILEWIN32_H

#include <windows.h>
#include "ifile.h"

class FileWin32 final
        : public IFile
{
public:
    FileWin32();

    ~FileWin32();

    // IFile interface
private:
    bool open(std::string const& filename, OpenFlags flags) noexcept;
    bool close() noexcept;
    file_pointer size();
    bool read(char *data, std::size_t count) noexcept;
    bool write(const char *data, std::size_t count) noexcept;
    file_pointer seek(file_pointer p, int rel) noexcept;

private:
    HANDLE file;
};

#endif // FILEWIN32_H
