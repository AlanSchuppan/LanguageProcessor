//#pragma once

#ifndef TEXT_TABLE_DEFD
#define TEXT_TABLE_DEFD 

#include <string>
#include <vector>

//##############################################################################
// CTextTable
//##############################################################################
//##############################################################################

class CTextTable {
    wchar_t mDelimiter;
    wchar_t mQuote;
    std::wstring mQuoteDelimiter;
    std::wstring mLine;
public:
    CTextTable(wchar_t delimiter = L',', wchar_t quote = L'"');
    CTextTable(const CTextTable &other);
    CTextTable(CTextTable &&other);
    CTextTable &operator=(const CTextTable &other);
    CTextTable &operator=(CTextTable &&other);

    void Clear();
    void Add(const std::wstring &value);
    void Add(const std::vector<std::wstring> &values);
    std::wstring Line() const;
    void Parse(const std::wstring &line,
               std::vector<std::wstring> &values) const;
};

// Clears the accumulated output line
inline void CTextTable::Clear() {
    mLine.clear();
}

// Returns the accumulated output line
inline std::wstring CTextTable::Line() const {
    return mLine;
}

//##############################################################################

#endif // TEXT_TABLE_DEFD
