#include "stdafx.h"

#include <iostream> //TODO: Remove
#include <stdexcept>

#include "Utils.hpp"
#include "TextTable.hpp"

//##############################################################################i
// CTextTable
//##############################################################################
//##############################################################################

//------------------------------------------------------------------------------
// CTextTable::CTextTable(wchar_t delimiter = L',', wchar_t quote = L'"')
//
//  Constructor sets the delimiter and quote characters.
//
CTextTable::CTextTable(wchar_t delimiter, wchar_t quote) :
    mDelimiter(delimiter), mQuote(quote), mQuoteDelimiter(2, quote) {
    mQuoteDelimiter[1] = mDelimiter;
}

//------------------------------------------------------------------------------
// CTextTable::CTextTable(const CTextTable &other)
//
// Copy constructor makes an independent copy of other.
//
CTextTable::CTextTable(const CTextTable &other) :
    mDelimiter(other.mDelimiter), mQuote(other.mQuote),
    mQuoteDelimiter(2, other.mQuote) {
    mQuoteDelimiter[1] = other.mDelimiter;
    mLine = other.mLine;
}

//------------------------------------------------------------------------------
// CTextTable::CTextTable(const CTextTable &other)
//
// Move constructor makes an identical object by taking the content of other.
//
CTextTable::CTextTable(CTextTable &&other) {
    mDelimiter = other.mDelimiter;
    mQuote = other.mQuote;
    mQuoteDelimiter = std::move(other.mQuoteDelimiter);
    mLine = std::move(other.mLine);
}

//------------------------------------------------------------------------------
// CTextTable &CTextTable::operator=(const CTextTable &other) {
//
// Copy assignment operator copies the content of other.
//
CTextTable &CTextTable::operator=(const CTextTable &other) {
    mDelimiter = other.mDelimiter;
    mQuote = other.mQuote;
    mQuoteDelimiter = other.mQuoteDelimiter;
    mLine = other.mLine;
    return *this;
}

//------------------------------------------------------------------------------
// CTextTable &CTextTable::operator=(CTextTable &&other) {
//
// Move assignment makes this object identical to other by taking its content.
//
CTextTable &CTextTable::operator=(CTextTable &&other) {
    mDelimiter = other.mDelimiter;
    mQuote = other.mQuote;
    mQuoteDelimiter = std::move(other.mQuoteDelimiter);
    mLine = std::move(other.mLine);
    return *this;
}

//------------------------------------------------------------------------------
// void CTextTable::Add(const std::wstring &value)
//
// Function adds the specified value to the output line, enclosing it in mQuotes
// as needed.
//
void CTextTable::Add(const std::wstring &value) {
    // Lead with delimiter if not first item 
    if (!mLine.empty())
        mLine.append(1, mDelimiter);

    // Double up quotes if any
    std::wstring Value(value);
    size_t OriginalLen = Value.size();
    if (OriginalLen > 0) {
        size_t Pos = OriginalLen - 1;
        Pos = Value.find_last_of(mQuote, Pos);
        while (Pos != std::wstring::npos) {
            Value.insert(Pos, 1, mQuote);
            Pos = (Pos > 0)
                ? Value.find_last_of(mQuote, Pos - 1) : std::wstring::npos;
        }
    }

    // Append value, quoting if necessary    
    bool NeedQuote = (Value.size() > OriginalLen) || // Has quote(s)
        (Value.find(mDelimiter) != std::wstring::npos); // Has delimiter(s)
    if (NeedQuote)
        mLine.append(1, mQuote);
    mLine.append(Value);
    if (NeedQuote)
        mLine.append(1, mQuote);
}

//------------------------------------------------------------------------------
// void CTextTablel::Add(const std::vector<std::wstring> &values)
//
// Function adds the specified values to the output line, enclosing them in
// mQuotes as needed.
//
void CTextTable::Add(const std::vector<std::wstring> &values) {
    for (auto Value : values) {
        Add(Value);
    }
}

//------------------------------------------------------------------------------
// void CTextTable::Parse(const std::wstring &line,
//                    std::vector<std::Wstring> &values) const
//  
//  Function parses the specified text line according to the current delimiter
//  and quote character and then returns the corresponding values as a vector
//  of strings.
//
void CTextTable::Parse(const std::wstring &line,
    std::vector<std::wstring> &values) const {
    values.clear();

    //QuoteDelimiter.append(1, mDelimiter);
    //std::cout << "Parsing \"" << WStrToUtf8(line) << "\"" << std::endl;

    size_t Pos = 0;
    size_t Len = line.size();
    bool Done = (Len == 0);
    while (!Done) {
        size_t NextPos = Len;
        if (line[Pos] == mQuote) {
            NextPos = line.find(mQuoteDelimiter, Pos);
            if (NextPos != std::wstring::npos) {
                ++NextPos;
            }
            else if (line[Len - 1] == mQuote) {
                NextPos = Len;
                Done = true;
            }
            else
                throw std::runtime_error("CTextTable::Line(): "
                    "Mismatched quote");
            //std::cout << "NextPos(1) = " << NextPos << std::endl;
        }
        else {
            NextPos = line.find(mDelimiter, Pos);
            if (NextPos == std::wstring::npos) {
                NextPos = Len;
                Done = true;
            }
            //std::cout << "NextPos(2) = " << NextPos << std::endl;
        }
        std::wstring Value(line.substr(Pos, NextPos - Pos));
        size_t ValueLen = Value.size();
        if (ValueLen >= 2 && Value[0] == mQuote &&
            Value[ValueLen - 1] == mQuote) {
            Value.erase(ValueLen - 1, 1);
            Value.erase(0, 1);
        }
        values.push_back(Value);
        Pos = NextPos + 1;
    }
}
