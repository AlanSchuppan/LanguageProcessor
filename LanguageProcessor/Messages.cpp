#include "stdafx.h"

#include <stdexcept>

#include "Utils.hpp"
#include "Messages.hpp"

//##############################################################################
// CMessage
//##############################################################################
//! Contains a single message with a name, description, translate flag, and all
//! its translations.
//##############################################################################

//------------------------------------------------------------------------------
//! Default constructor initialize an empty message.
//
CMessage::CMessage() : mTranslate(L'T') {
}

//------------------------------------------------------------------------------
//! Copy constructor makes a copy of another message.
//
CMessage::CMessage(const CMessage &other) : mName(other.mName),
mDescription(other.mDescription), mTranslate(other.mTranslate) {
    mTranslations = other.mTranslations;
}

//------------------------------------------------------------------------------
//! Move constructor takes the content of the other object.
//
CMessage::CMessage(CMessage &&other) : mTranslate(other.mTranslate) {
    mName = std::move(other.mName);
    mDescription = std::move(other.mDescription);
    mTranslations = std::move(other.mTranslations);
}

//------------------------------------------------------------------------------
//!! Copy assignment operator copies the content of another object.
CMessage &CMessage::operator=(const CMessage &other) {
    mName = other.mName;
    mDescription = other.mDescription;
    mTranslate = other.mTranslate;
    mTranslations = other.mTranslations;
    return *this;
}

//------------------------------------------------------------------------------
//! Move assignment operator takes the content of another object.
//
CMessage &CMessage::operator=(CMessage &&other) {
    mName = std::move(other.mName);
    mDescription = std::move(other.mDescription);
    mTranslate = other.mTranslate;
    mTranslations = std::move(other.mTranslations);
    return *this;
}

//------------------------------------------------------------------------------
//! Function appends a translation the list of translations.  All translations
//! need to be added in the order of the languages known by the containing
//! CMessages object.
//
void CMessage::TranslationAdd(const std::wstring &translation) {
    mTranslations.push_back(translation);
}

//------------------------------------------------------------------------------
//! Function appends a vector of translation the list of translations.  All
//! translations need to be added in the order of the languages known by the
//! containing CMessages object.
//
void CMessage::TranslationAdd(const std::vector<std::wstring> &translations) {
    auto TransIt = translations.begin();
    while (TransIt != translations.end())
        mTranslations.push_back(*TransIt++);
}

//------------------------------------------------------------------------------
//! Function gets the translation associated with the specified languagei, whose
//! location is determined from the specified list of languages.
// 
std::wstring CMessage::Translation(const std::vector<std::wstring> &languages,
    const std::wstring &language) const {
    size_t Ix = 0;
    size_t Count = languages.size();
    while (Ix < Count && languages[Ix] != language)
        ++Ix;
    if (Ix >= Count) // If language not found
        return L"???";
    if (!DoTranslate()) // If do not translate
        Ix = 0;
    return (Ix < mTranslations.size()) ? mTranslations[Ix] : std::wstring(L"");
}

//##############################################################################
// CMessages
//##############################################################################
//! Contains the all messages and a list of languages common to all messages.
//##############################################################################

//------------------------------------------------------------------------------
//! Default constructor does nothing beyond the autmatic construction of
//! mLanguages and mTranslations.
//
CMessages::CMessages() {
}

//------------------------------------------------------------------------------
//! Function returns the translations for the specified language. 
//
void CMessages::Translations(const std::wstring &language,
    std::vector<std::wstring> &translations) const {
    translations.clear();
    for (auto MsgIt = mMessages.begin(); MsgIt != mMessages.end(); ++MsgIt)
        translations.push_back(MsgIt->Translation(mLanguages, language));
}

//------------------------------------------------------------------------------
//! Static function tests CMessage and CMessages.
//
uint32_t MessagesTest(std::vector<std::string> &report) {
    struct CFileTable {
        bool Translate;
        const wchar_t *Items[4];
    };
    CFileTable FileTable[] = {
        { false, L"English", L"German", L"French", nullptr },
        { true,  L"English", L"German", L"French", nullptr },
        { true,  L"Eng1",    L"Ger1",   L"Fr1",    nullptr },
        { false, L"Eng2",    L"Ger2",   L"Fr2",    nullptr },
        { true,  L"Eng3",    L"Ger3",   L"Fr3",    nullptr }
    };
    size_t FileTableLen = sizeof(FileTable) / sizeof(*FileTable);

    // Initialize
    //
    uint32_t NErrors = 0;
    report.push_back("");
    report.push_back("Messages Test:");

    // Read languages
    std::vector<std::wstring> Languages;
    for (size_t Iy = 0; FileTable[0].Items[Iy] != nullptr; ++Iy)
        Languages.push_back(FileTable[0].Items[Iy]);
    CMessages Messages(Languages);

    // Read Translations
    for (size_t Ix = 1; Ix < FileTableLen; ++Ix) {
        CMessage Message;
        Message.Translate(FileTable[Ix].Translate);
        for (size_t Iy = 0; FileTable[Ix].Items[Iy] != nullptr; ++Iy)
            Message.TranslationAdd(FileTable[Ix].Items[Iy]);
        Messages.MessageAdd(Message);
    }

    // Check results
    size_t Ix = 0;
    for (auto LangIt = Languages.begin(); LangIt != Languages.end(); ++LangIt) {
        std::vector<std::wstring> Translations;
        Messages.Translations(*LangIt, Translations);
        size_t Iy = 1;
        for (std::wstring Translation : Translations) {
            size_t Iz = (FileTable[Iy].Translate) ? Ix : 0;
            if (Translation != FileTable[Iy].Items[Iz]) {
                std::wstring Report(L"  Incorrect ");
                Report += *LangIt;
                Report += L": \"";
                Report += Translation;
                Report += L"\" should be \"";
                Report += FileTable[Iy].Items[Iz];
                Report += L"\".";
                report.push_back(WStrToUtf8(Report));
                ++NErrors;
            }
            ++Iy;
        }
        ++Ix;
    }

    return NErrors;
}
