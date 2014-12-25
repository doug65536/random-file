#ifndef FILECXX_H
#define FILECXX_H

#include "ifile.h"
#include <fstream>

class CxxFile final
        : public IFile
{
    // IFile interface
protected:
    bool open(std::string const& filename) noexcept;
    bool close() noexcept;
    file_pointer size() noexcept;
    bool read(char *data, std::size_t count) noexcept;
    bool write(const char *data, std::size_t count) noexcept;
    file_pointer seek(file_pointer p, int rel) noexcept;

private:
    std::fstream file;
};
typedef CxxFile DefaultFile;

#endif // FILECXX_H
