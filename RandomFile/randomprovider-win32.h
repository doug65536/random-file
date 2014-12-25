#ifndef RANDOMPROVIDERWIN32_H
#define RANDOMPROVIDERWIN32_H

#include <windows.h>
#include "irandomprovider.h"

// A RandomProviderWin32 is a BufferedRandomProvider<BYTE>
class RandomProviderWin32 final
        : public BufferedRandomProvider<BYTE>
{
public:
    RandomProviderWin32(std::size_t size = 512);

    ~RandomProviderWin32();

    std::uint8_t const* moreData() noexcept;

private:
    HCRYPTPROV hProvider;
};

#endif // RANDOMPROVIDERWIN32_H
