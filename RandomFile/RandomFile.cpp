#include "stdafx.h"

#include "ifile.h"
#include "irandomprovider.h"

typedef std::int64_t file_pointer;

int realmain( int argc, char const* const* argv )
{
  if( argc < 2 ) {
    std::cerr << "Missing arguments" << std::endl;
    std::cerr << "Specify a filename" << std::endl <<
                  "and optionally a number of random"
                  "bytes to write" << std::endl;
    std::cerr << "If the size is not specified,"
                 " the whole file size is used" << std::endl;
    return 1;
  }

  char const* filename = argv[ 1 ];
  file_pointer     bytes    = argc > 2
          ? std::strtoll( argv[ 2 ], nullptr, 10 )
          : -1;

  std::cout << "Acquiring cryptographic provider context" << std::endl;

//  tout << _T("Opening ") << filename << std::endl;

  std::unique_ptr<IFile> outputFile(IFile::create());

  if (!outputFile->open( filename, IFile::CREATE )) {
    std::cerr << "Unable to open output file \"" <<
                 filename << "\"." << std::endl;
    return 1;
  }

  // If we set bytes == -1 then auto-detect size
  if (-1 == bytes) {
      std::cout << "Detecting file size" << std::endl;
      bytes = outputFile->size();
      if (outputFile->seek(0) != 0) {
          std::cout << "Seek error" << std::endl;
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

    //std::cout << "Writing " << bytesToWrite << std::endl;
    if (!outputFile->write( (char const*)random->moreData(), bytesToWrite))
    {
        std::cerr << "Write error! Giving up"  << std::endl;
        exit(2);
    }
    
    bytesRemaining -= bytesToWrite;
  }

  std::cout << "Closing file" << std::endl;
  outputFile->close();


  return 0;
}

