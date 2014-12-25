#include "randomprovider-win32.h"

#include <limits>
#include <stdexcept>
#include <iostream>

IRandomProvider *IRandomProvider::create()
{
    return new RandomProviderWin32;
}


RandomProviderWin32::RandomProviderWin32(std::size_t size)
    : BufferedRandomProvider(size)
    , hProvider(0)
{
    if (size > (std::numeric_limits<DWORD>::max)())
        throw std::invalid_argument("size limited to DWORD range");
    if( !::CryptAcquireContextW( &hProvider, 0, 0,
                                 PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT ) ) {
        std::wcout << L"Unable to acquire cryptographic context." << std::endl;
        throw std::runtime_error("Failed to initialize generator");
    }
}

RandomProviderWin32::~RandomProviderWin32()
{
    std::wcout << L"Releasing cryptographic provider" << std::endl;
    if( !::CryptReleaseContext( hProvider, 0 ) ) {
        std::wcout << L"Failure while closing generator." << std::endl;
    }
}

const std::uint8_t *RandomProviderWin32::moreData() noexcept
{
    if( !::CryptGenRandom( hProvider, (DWORD)buffer.size(), buffer.data() ) ) {
        ::CryptReleaseContext( hProvider, 0 );
        std::wcout << L"Unable to generate random bytes." << std::endl;
        return nullptr;
    }

    return buffer.data();
}
