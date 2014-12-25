#ifndef RANDOMPROVIDERURANDOM_H
#define RANDOMPROVIDERURANDOM_H

#include "irandomprovider.h"
#include "config.h"

class RandomProviderURandom final
        : public BufferedRandomProvider<std::uint8_t>
{
public:
    RandomProviderURandom(std::size_t size = 512);

    ~RandomProviderURandom();

    std::uint8_t const* moreData() noexcept;

private:
    int file;
};
typedef RandomProviderURandom DefaultRandomProvider;

#endif // RANDOMPROVIDERURANDOM_H
