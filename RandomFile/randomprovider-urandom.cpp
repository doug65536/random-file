#include "randomprovider-urandom.h"

#include "config.h"
#include <stdexcept>
#include <limits>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

IRandomProvider* IRandomProvider::create()
{
    return new RandomProviderURandom;
}

RandomProviderURandom::RandomProviderURandom(std::size_t size)
    : BufferedRandomProvider(size)
    , file(-1)
{
    if (size > (std::numeric_limits<int>::max)())
        throw std::invalid_argument("size limited to int range");
    file = ::open("/dev/urandom", O_LARGEFILE);
    if (file < 0) {
        std::wcout << L"Unable to open /dev/urandom" << std::endl;
        throw std::runtime_error("Failed to initialize generator");
    }
}

RandomProviderURandom::~RandomProviderURandom()
{
    std::wcout << L"Releasing cryptographic provider" << std::endl;
    if( ::close( file ) != 0) {
        std::wcout << L"Failure while closing generator." << std::endl;
    }
}

const uint8_t *RandomProviderURandom::moreData() noexcept
{
    if( !::read( file, buffer.data(), (int)buffer.size() ) ) {
        std::wcout << L"Unable to generate random bytes." << std::endl;
        return nullptr;
    }

    return buffer.data();
}
