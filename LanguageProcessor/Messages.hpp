//#pragma once

#ifndef MESSAGES_HPP
#define MESSAGES_HPP

#include <string>
#include <vector>

//##############################################################################
// CLanguageSpec
//##############################################################################
//! contains language specifications, such as name, ultimate storage location,
//! listing order, and character set.
//##############################################################################

class CLanguageSpec {
public:
    enum EStorage { None, Code, File };
    CLanguageSpec(const std::wstring &name, EStorage storage,
                  uint8_t order, uint8_t charSet);
    CLanguageSpec(const CLanguageSpec &other);
    CLanguageSpec(CLanguageSpec &&other);
    const CLanguageSpec &operator=(const CLanguageSpec &other);
    const CLanguageSpec &operator=(const CLanguageSpec &&other);
    const std::wstring Name() const;
    EStorage Storage() const;
    uint8_t  Order() const;
    uint8_t  CharSet() const;
private:
    std::wstring mName;
    EStorage     mStorage;
    uint8_t      mOrder;
    uint8_t      mCharSet;
};

// Returns the language name
inline const std::wstring CLanguageSpec::Name() const {
    return mName;
}

// Returns the intended storage of the language
inline CLanguageSpec::EStorage CLanguageSpec::Storage() const {
    return mStorage;
}

// Returns the menu listing order
inline uint8_t CLanguageSpec::Order() const {
    return mOrder;
}

// Returns the language's corresponding character set
inline uint8_t CLanguageSpec::CharSet() const {
    return mCharSet;
}

//##############################################################################
// CMessage
//##############################################################################
//! Contains a single message with a name, description, translate flag, and all
//! its translations.
//##############################################################################

class CMessage {
public:
    CMessage();
    CMessage(const CMessage &other);
    CMessage(CMessage &&other);
    CMessage &operator=(const CMessage &other);
    CMessage &operator=(CMessage &&other);

    std::wstring Name() const { return mName; }
    void         Name(const std::wstring &name) { mName = name; }
    std::wstring Description() const { return mDescription; }
    void         Description(const std::wstring &description) {
        mDescription = description;
    }
    wchar_t Translate() const { return mTranslate; }
    void Translate(wchar_t translate) { mTranslate = translate; }
    void Translate(bool translate) { mTranslate = (translate) ? L'T' : L'F'; }
    bool DoTranslate() const;

    void TranslationAdd(const std::wstring &translation);
    void TranslationAdd(const std::vector<std::wstring> &translations);
    std::wstring Translation(const std::vector<CLanguageSpec> &languageSpecs,
        const std::wstring &language) const;
    std::wstring Translation(const std::vector<CLanguageSpec> &languageSpecs,
        const std::string &language) const;

private:
    std::wstring mName;
    std::wstring mDescription;
    wchar_t      mTranslate;
    std::vector<std::wstring> mTranslations;

    void CMessage::Write(uint8_t *&pwrite, uint32_t value, uint32_t size);
};

// Returns true if this string should be translated
inline bool CMessage::DoTranslate() const {
    return (mTranslate != 'F');
}


//##############################################################################
// CMessages
//##############################################################################
//! Contains the all messages and a list of languages common to all messages.
//##############################################################################

class CMessages {
public:
    CMessages();
    //CMessages(const std::vector<std::wstring> &languages);
    CMessages(const CMessages &other) = delete;
    CMessages(CMessages &&other) = delete;
   ~CMessages();
    CMessages &operator=(const CMessages &other) = delete;
    CMessages &operator=(CMessages &&other) = delete;

    void LanguageSpecAdd(const CLanguageSpec &languageSpec);
    void LanguageSpecs(std::vector<CLanguageSpec> &languageSpecs) const;
    void EnumNames(std::vector<std::string> &enumNames);

    void MessageAdd(const CMessage &message);
    void Translations(const std::wstring &language,
        std::vector<std::wstring> &translations) const;
    void Translations(const std::string &language,
        std::vector<std::wstring> &translations) const;

    uint8_t *ImageCreate(const std::string &language, uint32_t &totalSize);

private:
    std::vector<CLanguageSpec> mLanguageSpecs;
    std::vector<std::wstring> mStorages;
    std::vector<std::wstring> mOrders;
    std::vector<std::wstring> mCharSets;
    std::vector<CMessage> mMessages;
    uint8_t *mpImage;

    void Write(uint8_t *&pwrite, uint32_t value, uint32_t size);
};

//! Sets the languages for the translations 
//inline CMessages::CMessages(const std::vector<CLanguageSpec> &languageSpecs) {
//    mLanguageSpecs = languageSpecs;
//}

//! Adds a language specification for the translations 
inline void CMessages::LanguageSpecAdd(const CLanguageSpec &languageSpec) {
    mLanguageSpecs.push_back(languageSpec);
}

//! Sets languageSpecss to the current set of language specifications, which
//! must correspond to the order of translations in each message.             
inline void CMessages::LanguageSpecs(std::vector<CLanguageSpec> &languageSpecs) const {
    languageSpecs = mLanguageSpecs;
}

//! Add a message, containing all translations, to the message list. 
inline void CMessages::MessageAdd(const CMessage &message) {
    mMessages.push_back(message);
}

//##############################################################################

uint32_t MessagesTest(std::vector<std::string> &report);

#endif // MESSAGES_HPP
