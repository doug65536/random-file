#include "stdafx.h"
#include <fstream>

#ifdef _UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif

int _tmain( int argc, _TCHAR* argv[] ) {
  if( argc < 2 ) {
    std::wcout << L"Missing arguments" << std::endl;
    return 1;
  }

  tstring filename = argv[ 1 ];
  int     bytes    = _ttoi( argv[ 2 ] );

  HCRYPTPROV hProvider = 0;

  if( !::CryptAcquireContextW( &hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT ) ) {
    std::wcout << L"Unable to acquire cryptographic context." << std::endl;
    return 1;
  }

  std::ofstream outputFile;
  outputFile.open( filename, std::ios::binary );
  
  if (outputFile.bad()) {
    std::wcout << L"Unable to open output file." << std::endl;
    return 1;
  }
  int bytesRemaining = bytes;
  while( 0 < bytesRemaining ) {
    const DWORD bufferLength             = 512;
    BYTE        pbBuffer[ bufferLength ] = {};

    if( !::CryptGenRandom( hProvider, bufferLength, pbBuffer ) ) {
      ::CryptReleaseContext( hProvider, 0 );
      std::wcout << L"Unable to generate random bytes." << std::endl;
      return 1;
    }

    int bytesToWrite = bufferLength;
    if( bufferLength > bytesRemaining ) {
      bytesToWrite = bytesRemaining;
    }

    outputFile.write( (const char*)pbBuffer, bytesToWrite );
    
    bytesRemaining -= bufferLength;
  }

  outputFile.close();

  if( !::CryptReleaseContext( hProvider, 0 ) ) {
    std::wcout << L"Failure while releasing cryptographic context." << std::endl;
    return 1;
  }

  return 0;
}

