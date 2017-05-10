//#pragma once

#ifndef MESSAGES_HPP
#define MESSAGES_HPP

#include <string>
#include <vector>

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
    wchar_t      Translate() const { return mTranslate; }
    void         Translate(wchar_t translate) { mTranslate = translate; }
    bool         DoTranslate() const;

    void TranslationAdd(const std::wstring &translation);
    void TranslationAdd(const std::vector<std::wstring> &translations);
    std::wstring Translation(const std::vector<std::wstring> &languages,
        const std::wstring &language) const;

private:
    std::wstring mName;
    std::wstring mDescription;
    wchar_t      mTranslate;
    std::vector<std::wstring> mTranslations;
};

inline bool CMessage::DoTranslate() const {
    return (mTranslate != L'F');
}

//##############################################################################
// CMessages
//##############################################################################
//! Contains the all messages and a list of languages common to all messages.
//##############################################################################

class CMessages {
public:
    CMessages();
    CMessages(const std::vector<std::wstring> &languages);
    CMessages(const CMessages &other) = delete;
    CMessages(CMessages &&other) = delete;
    CMessages &operator=(const CMessages &other) = delete;
    CMessages &operator=(CMessages &&other) = delete;

    void LanguageAdd(const std::wstring &language);
    void Languages(std::vector<std::wstring> &languages) const;
    void MessageAdd(const CMessage &message);
    void Translations(const std::wstring &language,
        std::vector<std::wstring> &translations) const;

private:
    std::vector<std::wstring> mLanguages;
    std::vector<CMessage> mMessages;
};

//! Sets the languages for the translations 
inline CMessages::CMessages(const std::vector<std::wstring> &languages) {
    mLanguages = languages;
}

//! Adds a language for the translations 
inline void CMessages::LanguageAdd(const std::wstring &language) {
    mLanguages.push_back(language);
}

//! Sets languages to the current set of languages, which must correspond to the
//! order of translations in each message. 
inline void CMessages::Languages(std::vector<std::wstring> &languages) const {
    languages = mLanguages;
}

//! Add a message, containing all translations, to the message list. 
inline void CMessages::MessageAdd(const CMessage &message) {
    mMessages.push_back(message);
}

//##############################################################################

uint32_t MessagesTest(std::vector<std::string> &report);

#endif // MESSAGES_HPP
