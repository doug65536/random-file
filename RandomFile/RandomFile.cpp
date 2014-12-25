#include "stdafx.h"

typedef std::int64_t file_pointer;

class IRandomProvider
{
public:
    virtual ~IRandomProvider() = 0;
    virtual std::uint8_t const* moreData() noexcept = 0;
    virtual std::size_t size() const noexcept = 0;
};

IRandomProvider::~IRandomProvider()
{
}

template<typename T>
class BufferedRandomProvider
        : public IRandomProvider
{
protected:
    BufferedRandomProvider(std::size_t size = 512)
        : buffer(size, T())
    {
        buffer.resize(size, 0);
    }

    std::size_t size() const noexcept
    {
        return buffer.size();
    }

protected:
    // BYTE because that is exactly the type CryptGenRandom expects
    typedef std::vector<std::uint8_t> BufferContainer;
    BufferContainer buffer;
};

#if defined(HAVE_WINDOWS_H)

// A Win32RandomProvider is a BufferedRandomProvider<BYTE>
class Win32RandomProvider final
        : public BufferedRandomProvider<BYTE>
{
public:
    Win32RandomProvider(std::size_t size = 512)
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

    ~Win32RandomProvider()
    {
        std::wcout << L"Releasing cryptographic provider" << std::endl;
        if( !::CryptReleaseContext( hProvider, 0 ) ) {
            std::wcout << L"Failure while closing generator." << std::endl;
        }
    }

    std::uint8_t const* moreData() noexcept
    {
        if( !::CryptGenRandom( hProvider, (DWORD)buffer.size(), buffer.data() ) ) {
          ::CryptReleaseContext( hProvider, 0 );
          std::wcout << L"Unable to generate random bytes." << std::endl;
          return nullptr;
        }

        return buffer.data();
    }

private:
    HCRYPTPROV hProvider;
};
typedef Win32RandomProvider DefaultRandomProvider;

#elif defined(HAVE_UNISTD_H)

class LinuxRandomProvider final
        : public BufferedRandomProvider<std::uint8_t>
{
public:
    LinuxRandomProvider(std::size_t size = 512)
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

    ~LinuxRandomProvider()
    {
        std::wcout << L"Releasing cryptographic provider" << std::endl;
        if( !::close( file ) ) {
          std::wcout << L"Failure while closing generator." << std::endl;
        }
    }

    std::uint8_t const* moreData() noexcept
    {
        if( !::read( file, buffer.data(), (int)buffer.size() ) ) {
          std::wcout << L"Unable to generate random bytes." << std::endl;
          return nullptr;
        }

        return buffer.data();
    }

private:
    int file;
};
typedef LinuxRandomProvider DefaultRandomProvider;
#else
#error Unrecognized platform
#endif

#ifdef WINDOWS_UTF16
std::string utf16ToUtf8(std::wstring const& utf16)
{
    std::vector<char> utf8;

    // Predict mix of small and large encodings
    // This doesn't affect final result size
    utf8.reserve(utf16.size() * 3);

    // Iterate through input
    std::wstring::const_iterator in = utf16.begin();
    while (in != utf16.end())
    {
        // Read a 16-bit character
        std::wstring::value_type ch = *in++;

        if (ch < 0x80)
        {
            // Simple character
            utf8.emplace_back(char(ch));
            // Fast path...
            continue;
        }

        // If we reached here it is going to encode multiple utf8 bytes
        std::size_t charSize;
        char32_t codepoint;

        // Just figure out the size of the output encoding
        // and figure out the actual unicode codepoint
        // that the input refers to
        if (ch < 0xD800 || ch > 0xDFFF)
        {
            // Non-surrogate pair

            if (ch <= 0x07FF)
                charSize = 2;
            else if (ch <= 0xFFFF)
                charSize = 3;

            codepoint = char32_t(ch);
        }
        else if(in == utf16.end())
        {
            // Write unicode "replacement character" to output
            // because we ran into the end of the input in the
            // middle of the decoding.
            // This input is probably malicious.
            codepoint = 0xFFFD;
            charSize = 3;
        }
        else
        {
            // Read second half of surrogate pair and advance iterator
            std::wstring::value_type loch = *in++;

            // The naming codepoint of "low" surrogates greater
            // than the codepoint of "high" surrogates.
            // Don't ask me

            codepoint = ((char32_t(ch) - 0xD800) << 10) |
                    ((loch - 0xDC00) & 0x3FF);

            if (codepoint < 0x200000)
                charSize = 4;
            else if (codepoint < 0x4000000)
                charSize = 5;
            else if (codepoint < 0x80000000)
                charSize = 6;
        }

        // Take an arbitrary codepoint and known encoding size and
        // utf8 encode it
        utf8.emplace_back(char(((signed char)0x80 >> (charSize - 1)) |
                char((unsigned char)codepoint >> (6*(charSize-1)))));
        for (std::size_t i = 1; i < charSize; ++i)
            utf8.emplace_back(char(0x80 | ((codepoint >> (6*(charSize - i))) & 0x3F)));
    }
    return std::string(std::begin(utf8), std::end(utf8));
}

std::wstring utf8ToUtf16(std::string const& utf8)
{
    std::vector<std::wstring::value_type> utf16;

    // Predict lots of space
    // This doesn't affect final result size
    utf16.reserve(8 + (utf8.size() >> 1));

    // Iterate through input
    std::string::const_iterator in = utf8.begin();
    while (in != utf8.end())
    {
        // Read an 8-bit character
        std::string::value_type ch = *in++;

        if ((ch & 0x80) == 0)
        {
            // Simple character
            utf16.emplace_back(std::wstring::value_type(
                std::make_unsigned<std::string::value_type>::type(ch)));
            // Fast path...
            continue;
        }

        std::size_t charSize;
        char32_t codepoint;

        // If execution reaches here we are processing a multibyte encoding
        if ((ch & 0xE0) == 0xC0)
            charSize = 2;
        else if ((ch & 0xF0) == 0xE0)
            charSize = 3;
        else if ((ch & 0xF8) == 0xF0)
            charSize = 4;
        else if ((ch & 0xFC) == 0xF8)
            charSize = 5;
        else if ((ch & 0xFE) == 0xFC)
            charSize = 6;

        codepoint = 0;

        std::string::value_type mask =
                std::string::value_type(~((signed char)0x80 >> charSize));
        codepoint = ch & mask;
        for (std::size_t i = 1; i < charSize; ++i)
        {
            if (in == utf8.end())
            {
                // Ran out of bytes in the middle of decoding
                // a character
                // Use the replacement character
                codepoint = 0xFFFD;
                break;
            }
            std::string::value_type trailer = *in++;
            if ((trailer & 0xC0) != 0x80)
            {
                // Invalid encoding
                // Use the replacement character
                codepoint = 0xFFFD;
                break;
            }

            // Merge 6 more bits into the resulting codepoint
            codepoint <<= 6;
            codepoint |= trailer & 0x3F;
        }

        // Ok, we have the codepoint
        // If it is < 0x10000 then it is simply written to output
        if (codepoint < 0x10000)
        {
            utf16.emplace_back(std::wstring::value_type(codepoint));
        }
        else
        {
            codepoint -= 0x10000;
            std::wstring::value_type hi = ((codepoint >> 10) & 0x3FF) + 0xD800;
            std::wstring::value_type lo = (codepoint & 0x3FF) + 0xDC00;

            utf16.emplace_back(hi);
            utf16.emplace_back(lo);
        }
    }
    return std::wstring(std::begin(utf16), std::end(utf16));
}
#endif

class IFile
{
public:
    typedef std::int64_t file_pointer;

    virtual bool open(std::string filename) noexcept = 0;
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

// Special implementation for win32 because it won't accept
// wchar* in mingw32 ofstream::open

#if !defined(HAVE_WINDOWS_H)
class CxxFile final
        : public IFile
{
public:
    bool open(std::string filename) noexcept
    {
        file.open(filename, std::ios::binary | std::ios::in | std::ios::out);
        return file;
    }

    bool close() noexcept
    {
        file.close();
        return file;
    }

    std::int64_t size()
    {
        std::ofstream::off_type save_off = file.tellp();
        return (std::min<decltype(save_off+std::int64_t(0))>)(save_off,
                (std::numeric_limits<std::int64_t>::max)());
    }

private:
    std::ofstream file;
};
typedef CxxFile DefaultFile;
#elif defined(HAVE_WINDOWS_H)
class Win32File final
        : public IFile
{
public:
    Win32File();

    ~Win32File();

    // IFile interface
private:
    bool open(std::string filename) noexcept;
    bool close() noexcept;
    file_pointer size();
    bool read(char *data, std::size_t count) noexcept;
    bool write(const char *data, std::size_t count) noexcept;
    file_pointer seek(file_pointer p, int rel) noexcept;

private:
    HANDLE file;
};

IFile::file_pointer Win32File::size()
{
    LARGE_INTEGER saveOfs;
    saveOfs.HighPart = 0;
    saveOfs.LowPart = SetFilePointer(
                file, saveOfs.LowPart,
                &saveOfs.HighPart, FILE_CURRENT);
    if (saveOfs.LowPart == INVALID_SET_FILE_POINTER &&
        GetLastError() != NO_ERROR)
        return false;

    LARGE_INTEGER result;
    result.HighPart = 0;
    result.LowPart = 0;
    result.LowPart = SetFilePointer(file, result.LowPart, &result.HighPart, FILE_END);
    if (result.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
        return -1;

    // Using QuadPart is technically UB here
    return (std::int64_t(result.HighPart) << 32) | result.LowPart;
}

bool Win32File::read(char *data, std::size_t count) noexcept
{
    DWORD didread = 0;
    return ReadFile(file, data, count, &didread, NULL) &&
            didread == count;
}

bool Win32File::write(const char *data, std::size_t count) noexcept
{
    DWORD wrote = 0;
    return WriteFile(file, data, count, &wrote, NULL) &&
            wrote == count;
}

file_pointer Win32File::seek(file_pointer p, int rel) noexcept
{
    LARGE_INTEGER ofs;
    ofs.LowPart = LONG(p);
    ofs.HighPart = LONG(p >> 32);
    LARGE_INTEGER result;
    result.HighPart = ofs.HighPart;
    result.LowPart = SetFilePointer(
                file,
                ofs.LowPart,
                &result.HighPart,
                rel < 0 ? FILE_BEGIN :
                rel > 0 ? FILE_END :
                FILE_CURRENT);
    if (result.LowPart == INVALID_SET_FILE_POINTER &&
            GetLastError() != NO_ERROR)
        return -1;
    return (file_pointer(result.HighPart) << 32) | result.LowPart;
}

Win32File::Win32File()
    : file(INVALID_HANDLE_VALUE)
{
}

Win32File::~Win32File()
{
    close();
}

bool Win32File::open(std::string filename) noexcept
{
    file = CreateFileW(utf8ToUtf16(filename).c_str(),
                       GENERIC_READ, FILE_SHARE_READ,
                       NULL, OPEN_ALWAYS,
                       FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    return file != INVALID_HANDLE_VALUE;
}

bool Win32File::close() noexcept
{
    if (file != INVALID_HANDLE_VALUE)
    {
        bool result = (CloseHandle(file) != FALSE);
        file = INVALID_HANDLE_VALUE;
        return result;
    }
    return true;
}
typedef Win32File DefaultFile;
#endif

static int realmain( int argc, char const** argv );

#if defined(HAVE_WINDOWS_H)
extern "C" int wmain( int argc, wchar_t** argv, wchar_t** envp )
{
    std::vector<char> utf8CChars;
    std::vector<char*> convertedArgv;
    if (argc > 0)
    {
        std::vector<std::wstring> utf16Argv;
        std::vector<std::string> utf8Argv;
        utf8Argv.reserve(argc+1);

        // Use implicit conversion from wchar_t* to string
        std::copy(argv, argv + argc, std::back_inserter(utf16Argv));

        // Convert all the utf16 strings to utf8
        std::transform(std::begin(utf16Argv), std::end(utf16Argv),
                       std::back_inserter(utf8Argv),
                       utf16ToUtf8);

        // Compute the total number of bytes to hold all of the strings
        // in one
        std::size_t totalBytes = std::accumulate(
                    std::begin(utf8Argv),
                    std::end(utf8Argv),
                    std::size_t(0),
                    [](std::size_t accum, std::string const& s) -> std::size_t
        {
                return accum + s.size() + 1;
        });

        std::vector<std::size_t> offsets;
        utf8CChars.reserve(totalBytes);
        std::for_each(
                    std::begin(utf8Argv),
                    std::end(utf8Argv),
                    [&](std::string const& c)
        {
            offsets.emplace_back(utf8CChars.size());
            // Copy the string characters
            std::copy(std::begin(c), std::end(c),
                      std::back_inserter(utf8CChars));
            utf8CChars.emplace_back(0);
        });

        // Populate pointer array to pass, "main"-style
        std::transform(
                    std::begin(offsets),
                    std::end(offsets),
                    std::back_inserter(convertedArgv),
                    [&](std::size_t offset) -> char*
        {
            return utf8CChars.data() + offset;
        });
    }

}
#else
int main(int argc, char *argv[])
{
  return realmain(argc, argv);
}
#endif

static int realmain( int argc, char const* argv[] )
{
  if( argc < 2 ) {
    std::wcerr << L"Missing arguments" << std::endl;
    return 1;
  }

  char const* filename = argv[ 1 ];
  file_pointer     bytes    = argc > 2 ? _atoi64( argv[ 2 ] ) : -1;

  std::wcout << L"Acquiring cryptographic provider context" << std::endl;

//  tout << _T("Opening ") << filename << std::endl;

  std::unique_ptr<IFile> outputFile(new DefaultFile);

  if (!outputFile->open( filename )) {
    std::wcerr << L"Unable to open output file." << std::endl;
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

  std::unique_ptr<IRandomProvider> random(new DefaultRandomProvider);
  
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

