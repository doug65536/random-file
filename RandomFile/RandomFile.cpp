#include "stdafx.h"

#include "ifile.h"
#include "irandomprovider.h"

typedef std::int64_t file_pointer;

int realmain( int argc, char const* const* argv )
{
  if( argc < 2 ) {
    std::wcerr << L"Missing arguments" << std::endl;
    return 1;
  }

  char const* filename = argv[ 1 ];
  file_pointer     bytes    = argc > 2
          ? std::strtoll( argv[ 2 ], nullptr, 10 )
          : -1;

  std::wcout << L"Acquiring cryptographic provider context" << std::endl;

//  tout << _T("Opening ") << filename << std::endl;

  std::unique_ptr<IFile> outputFile(IFile::create());

  if (!outputFile->open( filename )) {
    std::wcerr << L"Unable to open output file \"" << filename << "\"." << std::endl;
    return 1;
  }

  // If we set bytes == -1 then auto-detect size
  if (-1 == bytes) {
      std::wcout << L"Detecting file size" << std::endl;
      bytes = outputFile->size();
      if (outputFile->seek(0) != 0) {
          std::wcout << L"Seek error" << std::endl;
          return 1;
      }

      std::cout << "Detected " << bytes << " bytes" << std::endl;
  }

  std::unique_ptr<IRandomProvider> random(IRandomProvider::create());
  
  file_pointer bytesRemaining = bytes;
  while( 0 < bytesRemaining ) {
    std::size_t bytesToWrite = random->size();
    bytesToWrite = (std::min<decltype(bytesToWrite + bytesRemaining)>)(
                bytesToWrite, bytesRemaining );

    std::wcout << L"Writing " << bytesToWrite << std::endl;
    outputFile->write( (char const*)random->moreData(), random->size());
    
    bytesRemaining -= bytesToWrite;
  }

  std::wcout << L"Closing file" << std::endl;
  outputFile->close();


  return 0;
}

