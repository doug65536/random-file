#ifndef IFILE_H
#define IFILE_H

#include <cstdint>
#include <string>

class IFile
{
public:
    typedef std::int64_t file_pointer;

    static IFile *create();

    virtual ~IFile() = 0;
    virtual bool open(std::string const& filename) noexcept = 0;
    virtual bool close() noexcept = 0;

    virtual bool read(char* data, std::size_t count) noexcept = 0;
    virtual bool write(char const* data, std::size_t count) noexcept = 0;

    // rel: -1 = begin, 0 = cur, 1 = end
    virtual file_pointer seek(file_pointer p, int rel = -1) = 0;

    virtual file_pointer size() = 0;

    // Auto-pun to char *
    template<typename T>
    bool write(T const* data, std::size_t count) noexcept
    {
        return write((char const*)data, count * sizeof(*data));
    }

    // Auto-pun to char *
    template<typename T>
    bool read(T* data, std::size_t count)
    {
        return read((char*)data, count * sizeof(*data));
    }
};

#endif // IFILE_H
