#include "utfhelpers.h"

#include <vector>

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
