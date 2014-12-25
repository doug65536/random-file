#ifndef IRANDOMPROVIDER_H
#define IRANDOMPROVIDER_H

#include <cstdint>
#include <vector>

class IRandomProvider
{
public:
    static IRandomProvider *create();

    virtual ~IRandomProvider() = 0;
    virtual std::uint8_t const* moreData() noexcept = 0;
    virtual std::size_t size() const noexcept = 0;
};

template<typename T>
class BufferedRandomProvider
        : public IRandomProvider
{
protected:
    BufferedRandomProvider(std::size_t size = 512);
    std::size_t size() const noexcept;

protected:
    // BYTE because that is exactly the type CryptGenRandom expects
    typedef std::vector<std::uint8_t> BufferContainer;
    BufferContainer buffer;
};

template<typename T>
BufferedRandomProvider<T>::BufferedRandomProvider(std::size_t size)
    : buffer(size, T())
{
    buffer.resize(size, 0);
}

template<typename T>
std::size_t BufferedRandomProvider<T>::size() const noexcept
{
    return buffer.size();
}

#endif // IRANDOMPROVIDER_H
