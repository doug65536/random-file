#include "startup-win32.h"

#include <vector>
#include <string>
#include <algorithm>

#include "utfhelpers.h"

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
