//#pragma once
#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <string>

//#############################################################################

std::wstring Utf8ToWStr(const std::string  &utf8);
std::string  WStrToUtf8(const std::wstring &wstr);

//#############################################################################

std::string  ToUpperHex(uint32_t value, size_t width = 8);
std::string  ToLowerHex(uint32_t value, size_t width = 8);

//#############################################################################
// CModbusCRC
//#############################################################################
//#############################################################################

class CModbusCRC {
    static const uint16_t sInitialValue = 0xffff;
public:
    CModbusCRC(uint16_t value = sInitialValue);

    uint16_t Value() const { return mValue; }
    void     Value(uint16_t value) { mValue = value; }

    void Clear() { mValue = sInitialValue; }
    void Add(const uint8_t *pbuf, uint32_t count);
    void Add(uint8_t byte);

private:
    static const uint16_t sLookup[];
    uint16_t mValue;
};

//#############################################################################

uint32_t UtilsTest(std::vector<std::string> &report);

#endif // UTILS_HP
