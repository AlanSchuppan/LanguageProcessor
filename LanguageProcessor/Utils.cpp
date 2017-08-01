#include "stdafx.h"

#include <sstream>
//#include <iomanip>
#include <stdexcept>

#include "Utils.hpp"

//------------------------------------------------------------------------------
//! Function converts a UTF-8 string to a standard wide string.  The "magic"
//! numbers in the code are based on the following table.
//!
//!  Bytes | Bits | First  |  Last  |  Byte 1  |  Byte 2  |  Byte 3
//! -------+------+--------+--------+----------+----------+---------
//!    1   |   7  | U+0000 | U+007F | 0xxxxxxx |          |
//!    2   |  11  | U+0080 | U+07FF | 110xxxxx | 10xxxxxx |
//!    3   |  16  | U+0800 | U+FFFF | 1110xxxx | 10xxxxxx | 10xxxxxx
//!
std::wstring Utf8ToWStr(const std::string &utf8) {
    std::wstring Result;
    auto It = utf8.begin();
    while (It != utf8.end()) {
    //while (*It != '\0') {
            uint32_t NExtraBytes = 0;
        wchar_t WCh = 0x0000;
        if ((*It & 0x80) == 0x00) {      // If 1 byte
            WCh = static_cast<wchar_t>(*It);
        }
        else if ((*It & 0xe0) == 0xc0) { // If 2 bytes
            WCh = static_cast<wchar_t>(*It & 0x1f);
            NExtraBytes = 1;
        }
        else if ((*It & 0xf0) == 0xe0) { // If 3 bytes
            WCh = static_cast<wchar_t>(*It & 0x0f);
            NExtraBytes = 2;
        }
        else
            throw std::runtime_error("UTF8ToWStr(); Invalid UTF-8 string "
                "(lead).");
        ++It;

        while (NExtraBytes-- > 0) {
            if ((*It & 0xc0) != 0x80)
                throw std::runtime_error("UTF8ToWStr(): Invalid UTF-8 string "
                    "(follow).");
            WCh <<= 6;
            WCh |= static_cast<wchar_t>(*It++ & 0x3f);
        }

        Result.append(1, WCh);
    }
    return Result;
}

//------------------------------------------------------------------------------
//! Function converts a standard wide string to a standard UTF-8 string.  The
//! "magic" numbers in the code are based on the following table.
//!
//!  Bytes | Bits | First  |  Last  |  Byte 1  |  Byte 2  |  Byte 3
//! -------+------+--------+--------+----------+----------+---------
//!    1   |   7  | U+0000 | U+007F | 0xxxxxxx |          |
//!    2   |  11  | U+0080 | U+07FF | 110xxxxx | 10xxxxxx |
//!    3   |  16  | U+0800 | U+FFFF | 1110xxxx | 10xxxxxx | 10xxxxxx
//!
std::string WStrToUtf8(const std::wstring &wstr) {
    std::string Result;
    for (wchar_t Ch : wstr) {
        if (Ch < 0x0080) {
            Result.append(1, static_cast<char>(Ch));
        }
        else if (Ch < 0x0800) {
            std::string Buf(2, ' ');
            auto It = Buf.rbegin();
            *It++ = static_cast<char>((Ch & 0x003f) | 0x0080);
            Ch >>= 6;
            *It++ = static_cast<char>((Ch & 0x001f) | 0x00c0);
            Result.append(Buf);
        }
        else { // Ch <= 0xffff
            std::string Buf(3, ' ');
            auto It = Buf.rbegin();
            *It++ = static_cast<char>((Ch & 0x003f) | 0x0080);
            Ch >>= 6;
            *It++ = static_cast<char>((Ch & 0x003f) | 0x0080);
            Ch >>= 6;
            *It++ = static_cast<char>((Ch & 0x000f) | 0x00e0);
            Result.append(Buf);
        }
    }
    return Result;
}

//##############################################################################

//------------------------------------------------------------------------------
//! Function converts a 32-bit, unsigned integer to a hex string. using the
//! specified digit character table and limiting the width as specified, which
//! means some upper digits could be discarded.
//
std::string ToHex(const char *pdigits, uint32_t value, size_t width, char lead) {
    std::string Hex(width + 2, '-');

    if (width > 8)
        width = 8;
    size_t Ix = width + 1;
    while (width-- > 0) {
        Hex[Ix--] = pdigits[value % 16];
        value /= 16;
    }
    Hex[Ix--] = 'x';
    Hex[Ix--] = lead;
    return Hex;
}

//------------------------------------------------------------------------------
//! Function converts a 32-bit, unsigned integer to an upper case hex string,
//! limiting the width as specified, which means some upper digits could be
//! discarded.
//
std::string ToUpperHex(uint32_t value, size_t width, char lead) {
    return ToHex("0123456789ABCDEF", value, width, lead);
}

//------------------------------------------------------------------------------
//! Function converts a 32-bit, unsigned integer to a lower case hex string,
//! limiting the width as specified, which means isome upper digits could be
//! discarded.
//
std::string ToLowerHex(uint32_t value, size_t width, char lead) {
    return ToHex("0123456789abcdef", value, width, lead);
}

//##############################################################################
// CModbusCRC
//##############################################################################
//##############################################################################

const uint16_t CModbusCRC::sLookup[]{
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

//------------------------------------------------------------------------------
//! Constructor initializes the current CRC value as specified or to the default
//! Modbus CRC value if not specified.
//
CModbusCRC::CModbusCRC(uint16_t value) : mValue(value) {
}

//------------------------------------------------------------------------------
//! Functions adds the specified buffer of bytes to the accumulated CRC.
//
void CModbusCRC::Add(const uint8_t *pbuf, uint32_t count) {
    const uint8_t *pend = pbuf + count;
    while (pbuf < pend) {
        uint8_t Iy = static_cast<uint8_t>(mValue ^ *pbuf++);
        mValue >>= 8;
        mValue ^= sLookup[Iy];
    }
}

//------------------------------------------------------------------------------
//! Functions adds the specified byte to the accumulated CRC.
//
void CModbusCRC::Add(uint8_t byte) {
    byte ^= static_cast<uint8_t>(mValue);
    mValue >>= 8;
    mValue ^= sLookup[byte];
}

//##############################################################################

uint32_t UtilsTest(std::vector<std::string> &report) {
    struct CHexTable {
        uint32_t Value;
        const char *Upper;
        const char *Lower;
    };

    static const CHexTable HexTable[] = {
        { 0x01234567, "0x01234567", "0x01234567" },
        { 0x89ABCDEF, "0x89ABCDEF", "0x89abcdef" },
    };
    size_t HexTableLen = sizeof(HexTable) / sizeof(*HexTable);

    uint32_t NErrors = 0;
    report.push_back("UTilsTest:");

    // Check string conversions
    {
        std::wstring SourceString =
            L"\x0001\x003f\x007f\x0080\x0437\x07ff\x0800\x8421\xffff";
        std::wstring DestString = Utf8ToWStr(WStrToUtf8(SourceString));
        if (DestString != SourceString) {
            report.push_back("  Dest string doesn't match source string.");
            ++NErrors;
        }
    }

    // Check Hex conversions
    for (size_t Width = 0; Width <= 8; ++Width) {
        for (size_t Ix = 0; Ix < HexTableLen; ++Ix) {
            std::string Actual = ToUpperHex(HexTable[Ix].Value, Width);
            std::string Expected = HexTable[Ix].Upper;
            Expected.erase(2, 8 - Width);
            if (Actual != Expected) {
                std::stringstream Message;
                Message << "ToUpperHex: Expected \"" << Expected
                    << "\". Received \"" << Actual << "\".";
                report.push_back(Message.str());
            }
        }
        for (size_t Ix = 0; Ix < HexTableLen; ++Ix) {
            std::string Actual = ToLowerHex(HexTable[Ix].Value, Width);
            std::string Expected = HexTable[Ix].Lower;
            Expected.erase(2, 8 - Width);
            if (Actual != Expected) {
                std::stringstream Message;
                Message << "ToLowerHex: Expected \"" << Expected
                    << "\". Received \"" << Actual << "\".";
                report.push_back(Message.str());
            }
        }
    }

    return NErrors;
}
