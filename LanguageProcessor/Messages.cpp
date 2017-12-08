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
//! Copy assignment operator copies the content of another object.
//
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
//! Function gets the translation associated with the specified language, whose
//! location is determined from the specified list of languages.
// 
std::wstring CMessage::Translation(const std::vector<CLanguageSpec> &languageSpecs,
    const std::wstring &language) const {
    size_t Ix = 0;
    size_t Count = languageSpecs.size();
    while (Ix < Count && languageSpecs[Ix].Name() != language)
        ++Ix;
    if (Ix >= Count) // If language not found
        return L"???";
    if (!DoTranslate()) // If do not translate
        Ix = 0;
    return (Ix < mTranslations.size()) ? mTranslations[Ix] : std::wstring(L"");
}

//------------------------------------------------------------------------------
//! Function gets the translation associated with the specified language, whose
//! location is determined from the specified list of languages.
// 
std::wstring CMessage::Translation(const std::vector<CLanguageSpec> &languageSpecs,
    const std::string &language) const {
    return Translation(languageSpecs, Utf8ToWStr(language));
}

//##############################################################################
// CLanguageSpec
//##############################################################################
//! contains language specifications, such as name, ultimate storage location,
//! listing order, and character set.
//##############################################################################

//------------------------------------------------------------------------------
//! Constructor initializes the language specification
//
CLanguageSpec::CLanguageSpec(const std::wstring &name, EStorage storage,
                             uint8_t order, uint8_t charSet) : 
                             mName(name), mStorage(storage), mOrder(order),
                             mCharSet(charSet) {
}

CLanguageSpec::CLanguageSpec(const CLanguageSpec &other) :
    mName(other.mName), mStorage(other.mStorage),
    mOrder(other.mOrder), mCharSet(other.mCharSet) {
}

CLanguageSpec::CLanguageSpec(CLanguageSpec &&other) :
    mName(std::move(other.mName)), mStorage(other.mStorage),
    mOrder(other.mOrder), mCharSet(other.mCharSet) {
}

const CLanguageSpec &CLanguageSpec::operator=(const CLanguageSpec &other) {
    mName    = other.mName;
    mStorage = other.mStorage;
    mOrder   = other.mOrder;
    mCharSet = other.mCharSet;
    return *this;
}

const CLanguageSpec &CLanguageSpec::operator=(const CLanguageSpec &&other) {
    mName    = std::move(other.mName);
    mStorage = other.mStorage;
    mOrder   = other.mOrder;
    mCharSet = other.mCharSet;
    return *this;
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
CMessages::CMessages() : mpImage(nullptr) {
}

//------------------------------------------------------------------------------
//! Destructor ddeletes the image.
//
CMessages::~CMessages() {
    delete mpImage;
}

//------------------------------------------------------------------------------
//! Function returns the translations for the specified language. 
//
void CMessages::Translations(const std::wstring &language,
    std::vector<std::wstring> &translations) const {
    translations.clear();
    for (auto MsgIt = mMessages.begin(); MsgIt != mMessages.end(); ++MsgIt)
        translations.push_back(MsgIt->Translation(mLanguageSpecs, language));
}

//------------------------------------------------------------------------------
//! Function returns the translations for the specified language. 
//
void CMessages::Translations(const std::string &language,
    std::vector<std::wstring> &translations) const {
    Translations(Utf8ToWStr(language), translations);
}

void CMessages::EnumNames(std::vector<std::string> &enumNames) {
    enumNames.clear();
    for (auto MsgIt = mMessages.begin(); MsgIt != mMessages.end(); ++MsgIt)
        enumNames.push_back(WStrToUtf8(MsgIt->Name()));
}

//------------------------------------------------------------------------------
//! Private function writes the specified portion of the unsigned integer to the
//! image at the current position specified by pwrite and increments the
//! position accordingly.
//
void CMessages::Write(uint8_t *&pwrite, uint32_t value, uint32_t size) {
    if (size > sizeof(value))
        size = sizeof(value);
    for (uint32_t Ix = 0; Ix < size; ++Ix) {
        *pwrite++ = static_cast<uint8_t>(value);
        value >>= 8;
    }
}

//------------------------------------------------------------------------------
//! Function creates the binary language image for the specified language from
//! the provided translations, language order, and character set. In doing so,
//! it automatically makes choices to achieve the smallest data footprint. See
//! the lanuguage file format documentation for details. It assumes the third
//! item in text contains the language name in its own tongue.
//
uint8_t *CMessages::ImageCreate(const std::string &language, uint32_t &totalSize) {
    static const uint32_t kSignature = 0x628efe45;
    static const uint8_t  kNDblWordsHdr = 5;
    static const uint8_t  kVersion = 1;
    static const uint32_t kBytesPerDWord = sizeof(uint32_t);

    std::vector<std::wstring> Translations;
    this->Translations(language, Translations);
    uint32_t NStrings = Translations.size();
    if (NStrings < 3)
        throw(std::exception("Not enough strings to provide language name."));

    std::string LanguageName = WStrToUtf8(Translations[2]);
    uint8_t NDblWordsLang = static_cast<uint8_t>((LanguageName.size()
        + sizeof(uint32_t)) / sizeof(uint32_t));

    // Compute size of wide string translations
    uint32_t WideSize = NStrings; // Number of null terminators
    for (std::wstring Text : Translations)
        WideSize += Text.size();
    WideSize *= sizeof(wchar_t);

    // Compute size of narrow string translations
    uint32_t NarrowSize = Translations.size(); // Number of null terminators
    for (std::wstring Text : Translations)
        NarrowSize += WStrToUtf8(Text).size();

    //NarrowSize = 1000; // TODO: Remove

    bool WideStrings = (WideSize < NarrowSize);
    uint32_t StringSize = (WideStrings) ? WideSize : NarrowSize;

    // Determine offset size (Not exact, but good enough)
    uint8_t OffsetSize = 2;
    if (StringSize >= 65536)
        OffsetSize = 4;
    else if (StringSize < 256)
        OffsetSize = 1;

    // Set the Unicode (UCS-2) flag if needed
    uint8_t Flags = static_cast<uint8_t>(OffsetSize - 1);
    if (WideStrings)
        Flags |= 0x08;

    // Determine whether strings need to be aligne to word boundary:
    bool AlignStrings = (NStrings * OffsetSize % 2 == 1);

    totalSize = StringSize
        + (NDblWordsLang + kNDblWordsHdr) * sizeof(uint32_t)
        + NStrings * OffsetSize;
    if (AlignStrings)
        ++totalSize;

    //delete mpImage;
    uint8_t *pImage = new uint8_t[totalSize];

    // Fill for test purposes:
    //for (uint32_t Ix = 0; Ix < totalSize; ++Ix)
    //    mpImage[Ix] = 0xff;

    // Write header
    uint8_t *pWrite = pImage;
    Write(pWrite, kSignature, 4);
    Write(pWrite, 0, 2); // Temporary CRC
    Write(pWrite, kNDblWordsHdr, 1);
    Write(pWrite, NDblWordsLang, 1);
    Write(pWrite, totalSize, 4);
    Write(pWrite, NStrings, 4);
    Write(pWrite, Flags, sizeof(Flags));
    Write(pWrite, kVersion, sizeof(kVersion));

    { // Write language order and character set
        bool Found = false;
        std::wstring WLanguage(Utf8ToWStr(language));
        for (CLanguageSpec LanguageSpec : mLanguageSpecs)
            if (LanguageSpec.Name() == WLanguage) {
                Write(pWrite, LanguageSpec.Order(), sizeof(LanguageSpec.Order()));
                Write(pWrite, LanguageSpec.CharSet(), sizeof(LanguageSpec.CharSet()));
                Found = true;
                break;
            }
        if (!Found) {
            std::string Msg(" CMessages::ImageCreate(): Cannot find language \"");
            Msg += language;
            Msg += "\".";
            throw std::runtime_error(Msg);
        }
    }

    { // Write language name
        for (char Ch : LanguageName)
            Write(pWrite, Ch, sizeof(Ch));
        size_t LangSize = NDblWordsLang * sizeof(uint32_t);
        static const char Ch = '\0';
        for (size_t Ix = LanguageName.size(); Ix < LangSize; ++Ix)
            Write(pWrite, Ch, sizeof(Ch));
    }

    if (WideStrings) {
        // Write the wide offsets and then pad to achieve word boundary
        uint32_t Offset = 0;
        for (std::wstring Text : Translations) {
            Write(pWrite, Offset, OffsetSize);
            Offset += 2 * (Text.size() + 1);
        }
        if (AlignStrings)
            Write(pWrite, static_cast<uint8_t>(0), 1);

        // Write the wide strings
        for (std::wstring Text : Translations) {
            for (wchar_t Ch : Text)
                Write(pWrite, Ch, sizeof(Ch));
            static const wchar_t Ch = '\0';
            Write(pWrite, Ch, sizeof(Ch));
        }
    }
    else {
        // Write the wide offsets and then pad to achieve word boundary
        uint32_t Offset = 0;
        for (std::wstring Text : Translations) {
            Write(pWrite, Offset, OffsetSize);
            Offset += WStrToUtf8(Text).size() + 1;
        }
        if (AlignStrings)
            Write(pWrite, static_cast<uint8_t>(0), 1);

        // Write the narrow strings
        for (std::wstring Text : Translations) {
            std::string UTF8Text = WStrToUtf8(Text);
            for (char Ch : UTF8Text)
                Write(pWrite, Ch, sizeof(Ch));
            static const char Ch = '\0';
            Write(pWrite, Ch, sizeof(Ch));
        }
    }

    // Compute and write the CRC
    static const uint32_t CRCSkip = sizeof(uint32_t) + sizeof(uint16_t);
    CModbusCRC ModbusCRC;
    ModbusCRC.Add(mpImage + CRCSkip, totalSize - CRCSkip);
    uint16_t CRC = ModbusCRC.Value();
    pWrite = mpImage + CRCSkip - sizeof(CRC);
    Write(pWrite, CRC, sizeof(CRC));

    return pImage;
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
    CMessages Messages;
    for (size_t Iy = 0; FileTable[0].Items[Iy] != nullptr; ++Iy) {
        CLanguageSpec LanguageSpec(FileTable[0].Items[Iy], CLanguageSpec::None, 0, 0);
        Messages.LanguageSpecAdd(LanguageSpec);
    }

    // Read Translations
    for (size_t Ix = 1; Ix < FileTableLen; ++Ix) {
        CMessage Message;
        Message.Translate(FileTable[Ix].Translate);
        for (size_t Iy = 0; FileTable[Ix].Items[Iy] != nullptr; ++Iy)
            Message.TranslationAdd(FileTable[Ix].Items[Iy]);
        Messages.MessageAdd(Message);
    }

    std::vector<CLanguageSpec> LanguageSpecs;
    Messages.LanguageSpecs(LanguageSpecs);

    // Check results
    size_t Ix = 0;
    for (auto LangIt = LanguageSpecs.begin(); LangIt != LanguageSpecs.end(); ++LangIt) {
        std::vector<std::wstring> Translations;
        Messages.Translations(LangIt->Name(), Translations);
        size_t Iy = 1;
        for (std::wstring Translation : Translations) {
            size_t Iz = (FileTable[Iy].Translate) ? Ix : 0;
            if (Translation != FileTable[Iy].Items[Iz]) {
                std::wstring Report(L"  Incorrect ");
                Report += LangIt->Name();
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
