#include "stdafx.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>

#include "Utils.hpp"
#include "TextTable.hpp"
#include "Messages.hpp"
#include "Switches.hpp"

#define VERBOSE

//##############################################################################
// CLangImage
//##############################################################################
//! Creates a binary image, as shown in the following table, of the translations
//! for a given language, supporting fast look-up given a string ID.
//##############################################################################

class CLangImage {
public:
    CLangImage();
    CLangImage(const CLangImage &other);
    CLangImage(CLangImage &&other);
    ~CLangImage();

    void Fill(const std::string &language, 
              const std::vector<std::wstring> &translations,
              uint8_t langOrder, uint8_t charSet = 0);
    const uint8_t *Image() const { return mpImage; }
    const void SaveAsCode(const std::vector<std::string> &enums,
                          std::ostream &stream) const;
    //void GenerateCode(const std::vector<std::string> &enums, std::ostream &file) const;
    uint32_t Size() const { return mTotalSize; }
    void Show() const;
private:
    std::string mLanguage;
    uint8_t *mpImage;
    uint8_t *mpWrite;
    uint32_t mTotalSize;

    void Write(uint32_t value, uint32_t size);
    static void WriteLine(const uint8_t **ppimage, uint32_t maxBytes, uint32_t bytes,
                          const std::string &comment, std::ostream &file);
};


//##############################################################################
// CLangImage
//##############################################################################
//! Creates a binary image, as shown in the following table, of the translations
//! for a given language, supporting fast look-up given a string ID.
//
//        +--------------+--------------+--------------+--------------+
//        |                         Signature                         |
//        +--------------+--------------+--------------+--------------+
//        |             CRC             | NDWords Hdr  | NDWords Lang |
//        +--------------+--------------+--------------+--------------+
//        |                        Total Size                         |
//        +--------------+--------------+--------------+--------------+
//        |                         NStrings                          |
//        +--------------+--------------+--------------+--------------+
//        |    Flags     |   Version    |  Lang Order  |   Char Set   |
//        +--------------+--------------+--------------+--------------+
//
//        +--------------+--------------+--------------+--------------+
//        |                   Language Name (UTF-8)                   |
//        +--------------+--------------+--------------+--------------+
//        |                   Language Name (UTF-8)                   |
//        +--------------+--------------+--------------+--------------+
//
//        +--------------+--------------+--------------+--------------+
//        |          Offset 00          |          Offset 01          |
//        +--------------+--------------+--------------+--------------+
//        |          Offset 02          |          Offset 03          |
//        +--------------+--------------+--------------+--------------+
//        |          Offset 04          |          Offset 05          |
//        +--------------+--------------+--------------+--------------+
//        |          Offset 06          |          Offset 07          |
//        +--------------+--------------+--------------+--------------+
//        |          Offset 08          |                             |
//        +--------------+--------------+                             +
//        |                                                           |
//        |                  Null-Terminated Strings                  |
//        |                     (UCS-2 or UTF-8)                      |
//        |                                                           |
//        |                                                           |
//        +--------------+--------------+--------------+--------------+
//
// Signature is a unique code(0x628efe45) to verify this file type
//
// CRC is the cyclic redundancy check for the remainder of the file.
//
// NDWords Hdr is the size of the header in double words. Any unused portion of
// a double word will be zero for backward compatibility.
//
// NDWords Lang is the size of the UTF-8 language name plus null terminator in
// the number of double words needed to fully contain them. Any unused portion
// of a double word will be zero.
//
// Total Size is the total size of the image in bytes.
//
// NStrings is the number of strings contained in the file.
//
// Flags contains the following bits: 0000UCOO, where O is one less than the
// OffsetBytes (size of each offset in bytes) (O cannot be 2 because size cannot
// be 3), C indicates whether this is code-based data (does not need to be
// deallocated), and if U is 1, strings are unicode rather than the usual UTF-8.
//
// Version is the format version of the file.
//
// Lang Order is a zero-based number representing the position in the
// displayed list this language will be displayed. Gaps have been included in
// the original assignment to allow for new languages to be inserted as desired.
// It is not the same as the language ID.
//
// The Char Set corresponds to the character set to be used for this
// translation.
//
// The Offsets supply the location of each null-terminated string from the
// beginning of the string section, which is always aligned to the next word
// boundary by padding bytes equal to zero at the end of the offsets as needed.
// The size of the each offset value is variable. See Flags.
//
// Language Name is the UTF-8 name of the language in its own tongue.
//
// Excel spreadsheet lists language as <lang>, <dest>, <order>, <dest> where
// <lang> is the language in English, <dest> is Code, Bin, None(or blank),
// <order> is a number specifying the order of the language when shown to the
// user, and <charset> specifies the character set used fo that language.
//##############################################################################

//------------------------------------------------------------------------------
//! Constructor creates an empty image.
//
CLangImage::CLangImage() : mpImage(nullptr), mpWrite(nullptr), mTotalSize(0) {
}

//------------------------------------------------------------------------------
//! Copy constructor duplicates the content of other.
//
CLangImage::CLangImage(const CLangImage &other) : mpImage(nullptr),
mpWrite(nullptr),
mTotalSize(0) {
    mpWrite = mpImage = new uint8_t[other.mTotalSize];
    mTotalSize = other.mTotalSize;
    for (uint32_t Ix = 0; Ix < mTotalSize; ++Ix)
        mpImage[Ix] = other.mpImage[Ix];
}

//------------------------------------------------------------------------------
//! Move constructor takes the content of other.
//
CLangImage::CLangImage(CLangImage &&other) {
    mpWrite = mpImage = other.mpImage;
    other.mpImage = nullptr;
    mTotalSize = other.mTotalSize;
    other.mTotalSize = 0;
    other.mpWrite = nullptr;
}


//------------------------------------------------------------------------------
//! Destructor deletes the image.
//
CLangImage::~CLangImage() {
    delete[] mpImage;
}

//------------------------------------------------------------------------------
//! Private function writes the specified portion of the unsigned integer to the
//! image at the current position specified by mpWrite and increments the
//! position accordingly.
//
void CLangImage::Write(uint32_t value, uint32_t size) {
    if (size > sizeof(value))
        size = sizeof(value);
    for (uint32_t Ix = 0; Ix < size; ++Ix) {
        *mpWrite++ = static_cast<uint8_t>(value);
        value >>= 8;
    }
}

//------------------------------------------------------------------------------
//! Function writes the specified number of hex bytes to the specified stream
//! and then fills the remaining spec for maxBytes before writing the specified
//! comment.
//!
void CLangImage::WriteLine(const uint8_t **ppimage, uint32_t maxBytes, uint32_t nBytes,
    const std::string &comment, std::ostream &file) {
    file << "    \"";

    for (uint32_t Ix = 0; Ix < nBytes; ++Ix)
        file << ToUpperHex(*(*ppimage)++, 2, '\\');
    file << "\"";
    if (!comment.empty()) {
        for (uint32_t Ix = nBytes; Ix < maxBytes; ++Ix)
            file << "    ";
        file << " // " << comment;
    }
    file << std::endl;
}

//------------------------------------------------------------------------------
//! Function creates the binary language image for the specified language from
//! the provided translations, language order, and character set. In doing so,
//! it automatically makes choices to achieve the smallest data footprint. See
//!  thelanuguage file format documentation for details. It assumes the third
//! item in text contains the language name in its own tongue.
//
void CLangImage::Fill(const std::string &language,
                      const std::vector<std::wstring> &translations, 
                      uint8_t langOrder, uint8_t charSet) {
    static const uint32_t kSignature = 0x628efe45;
    static const uint8_t  kNDblWordsHdr = 5;
    static const uint8_t  kVersion = 1;
    static const uint32_t kBytesPerDWord = sizeof(uint32_t);

    mLanguage = language;

    uint32_t NStrings = translations.size();
    if (NStrings < 3)
        throw(std::exception("Not enough strings to provide language name."));

    std::string LanguageName = WStrToUtf8(translations[2]);
    uint8_t NDblWordsLang =
        static_cast<uint8_t>((LanguageName.size() + sizeof(uint32_t)) / sizeof(uint32_t));

    // Compute size of wide string translations
    uint32_t WideSize = translations.size(); // Number of null terminators
    for (std::wstring Text : translations)
        WideSize += Text.size();
    WideSize *= sizeof(wchar_t);

    // Compute size of narrow string translations
    uint32_t NarrowSize = translations.size(); // Number of null terminators
    for (std::wstring Text : translations)
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

    mTotalSize = StringSize
        + (NDblWordsLang + kNDblWordsHdr) * sizeof(uint32_t)
        + NStrings * OffsetSize;
    if (AlignStrings)
        ++mTotalSize;

    delete mpImage;
    mpWrite = mpImage = new uint8_t[mTotalSize];

    // Fill for test purposes:
    //for (uint32_t Ix = 0; Ix < mTotalSize; ++Ix)
    //    mpImage[Ix] = 0xff;

    // Write header
    Write(kSignature, 4);
    Write(0, 2); // Temporary CRC
    Write(kNDblWordsHdr, 1);
    Write(NDblWordsLang, 1);
    Write(mTotalSize, 4);
    Write(NStrings, 4);
    Write(Flags, sizeof(Flags));
    Write(kVersion, sizeof(kVersion));
    Write(langOrder, sizeof(langOrder));
    Write(charSet, sizeof(charSet));

    { // Write language name
        for (char Ch : LanguageName)
            Write(Ch, sizeof(Ch));
        size_t LangSize = NDblWordsLang * sizeof(uint32_t);
        static const char Ch = '\0';
        for (size_t Ix = LanguageName.size(); Ix < LangSize; ++Ix)
            Write(Ch, sizeof(Ch));
    }

    if (WideStrings) {
        // Write the wide offsets and then pad to achieve word boundary
        uint32_t Offset = 0;
        for (std::wstring Text : translations) {
            Write(Offset, OffsetSize);
            Offset += 2 * (Text.size() + 1);
        }
        if (AlignStrings)
            Write(static_cast<uint8_t>(0), 1);

        // Write the wide strings
        for (std::wstring Text : translations) {
            for (wchar_t Ch : Text)
                Write(Ch, sizeof(Ch));
            static const wchar_t Ch = '\0';
            Write(Ch, sizeof(Ch));
        }
    }
    else {
        // Write the wide offsets and then pad to achieve word boundary
        uint32_t Offset = 0;
        for (std::wstring Text : translations) {
            Write(Offset, OffsetSize);
            Offset += WStrToUtf8(Text).size() + 1;
        }
        if (AlignStrings)
            Write(static_cast<uint8_t>(0), 1);

        // Write the narrow strings
        for (std::wstring Text : translations) {
            std::string UTF8Text = WStrToUtf8(Text);
            for (char Ch : UTF8Text)
                Write(Ch, sizeof(Ch));
            static const char Ch = '\0';
            Write(Ch, sizeof(Ch));
        }
    }

    // Compute and write the CRC
    static const uint32_t CRCSkip = sizeof(uint32_t) + sizeof(uint16_t);
    CModbusCRC ModbusCRC;
    ModbusCRC.Add(mpImage + CRCSkip, mTotalSize - CRCSkip);
    uint16_t CRC = ModbusCRC.Value();
    mpWrite = mpImage + CRCSkip - sizeof(CRC);
    Write(CRC, sizeof(CRC));
}

//------------------------------------------------------------------------------
//! Function saves the language image as C++ code using the provided enums.
//
const void CLangImage::SaveAsCode(const std::vector<std::string> &enums,
                                  std::ostream &stream) const {
    stream << "// " << mLanguage << " binary image" << std::endl;
    stream << "static const unsigned char " << mLanguage << "[] = {" << std::endl;
    stream << "    // Header" << std::endl;
    const uint8_t *pRead = mpImage;
    WriteLine(&pRead, 4, 4, "Signature", stream);
    WriteLine(&pRead, 4, 2, "CRC", stream);
    WriteLine(&pRead, 4, 1, "NDWordsHdr", stream);
    uint8_t NWordsLang = *pRead;
    WriteLine(&pRead, 4, 1, "NDWordsLang", stream);
    WriteLine(&pRead, 4, 4, "Total Size", stream);
    uint32_t NStrings = *reinterpret_cast<const uint32_t *>(pRead);
    WriteLine(&pRead, 4, 4, "NStrings", stream);
    uint8_t Flags = *pRead;
    WriteLine(&pRead, 4, 1, "Flags", stream);
    WriteLine(&pRead, 4, 1, "Version", stream);
    WriteLine(&pRead, 4, 1, "Lang Order", stream);
    WriteLine(&pRead, 4, 1, "Lang Char Set", stream);

    {
        std::string Native("Native Language Name");
        stream << std::endl << "    // " << Native << std::endl;
        while (NWordsLang-- > 0) {
            WriteLine(&pRead, 4, 4, Native, stream);
            Native.clear();
        }
    }

    uint32_t NEnums = enums.size();

    uint32_t NOffsetBytes = (Flags & 3) + 1;
    {  // List offset table
        stream << std::endl;
        stream << "    // Offset Table (" << 8 * NOffsetBytes << "-bit)"
            << std::endl;
        for (uint32_t Ix = 0; Ix < NStrings; ++Ix)
            WriteLine(&pRead, NOffsetBytes, NOffsetBytes,
            (Ix < NEnums) ? enums[Ix] : "", stream);
        if ((reinterpret_cast<const uint32_t>(pRead) & 1) != 0)
            WriteLine(&pRead, NOffsetBytes, 1, "** Alignment padding", stream);
    }

    {   // List strings
        bool Unicode = ((Flags & 8) != 0);
        const uint8_t *pStrings = pRead;
        uint32_t NOffsetChars = 2 * NOffsetBytes;
        stream << std::endl;
        stream << "    // Strings ("
            << (Unicode ? "Unicode" : "UTF-8") << ")"
            << std::endl;
        for (uint32_t Ix = 0; Ix < NStrings; ++Ix) {
            uint32_t NBytes;

            // Search fo end of string:
            if (Unicode) {
                const uint16_t *pNext =
                    reinterpret_cast<const uint16_t *>(pRead);
                while (*pNext != L'\0')
                    ++pNext;
                NBytes = reinterpret_cast<const uint8_t *>(pNext) - pRead + 2;
            }
            else {
                const uint8_t *pNext = pRead;
                while (*pNext != '\0')
                    ++pNext;
                NBytes = pNext - pRead + 1;
            }

            // Output the string bytes
            uint32_t Location = static_cast<const uint32_t>(pRead - pStrings);
            std::string Comment(ToUpperHex(Location, NOffsetChars));
            Comment += ": ";
            Comment += (Ix < NEnums) ? enums[Ix] : "";
            while (NBytes > 0) {
                uint32_t ThisNBytes = NBytes;
                if (ThisNBytes > 8)
                    ThisNBytes = 8;
                WriteLine(&pRead, 8, ThisNBytes, Comment, stream);
                NBytes -= ThisNBytes;
                Comment.clear();
            }
        }
    }
    std::wcout << "};";
}

//------------------------------------------------------------------------------
//! Function Generates the C++ code for a code-based language. It requires the
//! image to be generated first.
//
//void CLangImage::GenerateCode(const std::vector<std::string> &enums,
//                              const std::string &lang, std::ostream &file) const {
//    file << "// " << lang << " binary image" << std::endl;
//    file << "static const unsigned char " << lang << "[] = {" << std::endl;
//
//    file << "  // Header" << std::endl;
//    file << "" << std::endl;
//    file << "" << std::endl;
//    file << "" << std::endl;
//    file << "" << std::endl;
//    
//    file << "};" << std::endl;
//}

//------------------------------------------------------------------------------
//! Function Generates the C++ header file for a code-based language. It
//! requires the image to be generated first.
//
//void CLangImage::GenerateCode(const std::vector<std::string> &enums, 
//std::ostream &file) {
//    if (mpImage == nullptr)
//        throw std::runtime_error("Attempt to generate language code from empty "
//                                 "image.");
//    file << "// This file was generated by LanguageProcessor and should not be "
//        "edited." << std::endl;
//    file << std::endl;
//    file << "#if !defined(SOURCE) && !defined(LANG_PROCESSOR)" << std::endl;
//    file << "#define LANG_PROCESSOR" << std::endl;
//    file << std::endl;
//    file << "// Define message indices:" << std::endl;
//    file << "enum ELngMess {" << std::endl;
//
//
//    file << "};" << std::endl;
//    file << std::endl;
//    file << "#endif  //LANG_PROCESSOR" << std::endl;
//    file << std::endl;
//    file << std::endl;
//    file << "#ifdef SOURCE" << std::endl;
//    file << std::endl;
//
//    file << "" << std::endl;
//    file << "" << std::endl;
//    file << "" << std::endl;
//    file << "" << std::endl;
//    file << "" << std::endl;
//    file << "" << std::endl;
//    file << "" << std::endl;
//    file << "" << std::endl;
//}

//------------------------------------------------------------------------------
//! Function displays the content of the language file image.
//
void CLangImage::Show() const {
    static const uint32_t BytesPerLine = 4;
    uint32_t HeaderSize = mpImage[6] * sizeof(uint32_t);
    uint32_t LanguageSize = HeaderSize + mpImage[7] * sizeof(uint32_t);

    std::cout << "Image:" << std::setfill('0') << std::setbase(16);
    for (uint32_t Ix = 0; Ix < mTotalSize; ++Ix) {
        if (Ix % BytesPerLine == 0) {
            if (Ix == HeaderSize || Ix == LanguageSize)
                std::cout << std::endl;
            std::wcout << std::endl;
        }
        std::cout << " " << std::setw(2) << static_cast<uint32_t>(mpImage[Ix]) << std::setw(0);
    }
    std::cout << std::setfill('\0') << std::setbase(10) << std::endl;

    std::wcout << std::endl;

    std::vector<std::string> Enums;
    Enums.push_back("None");
    Enums.push_back("Empty");
    Enums.push_back("LanguageName");
    Enums.push_back("First");
    Enums.push_back("Second");
    Enums.push_back("Third");
    Enums.push_back("Fourth");

    SaveAsCode(Enums, std::cout);
}

//##############################################################################

int main(int argc, char** argv) {
    static const size_t NameIx = 0;
    static const size_t DescIx = 1;
    static const size_t TypeIx = 2;
    static const size_t LangIx = 3;
    static const CSwitchSpec SwitchSpecs[] = {
        { "-h",    ESwitchID::Help,     0, 0 },
        { "-?",    ESwitchID::Help,     0, 0 },
        { "-l",    ESwitchID::Language, 1, 4 },
        { "-p",    ESwitchID::Pause,    0, 0 },
        { "-v",    ESwitchID::Verbose,  0, 0 },
        { nullptr, ESwitchID::None,     0, 0 } // Terminator
    };

    int ExitCode = 0;
    CSwitches Switches(SwitchSpecs);

    try {
        Switches.ExecPath(argv[0]);
        for (int Ix = 1; Ix < argc; ++Ix)
            Switches.ItemAdd(argv[Ix]);
        Switches.Check();
        if (Switches.Exists(ESwitchID::Verbose))
            Switches.Show();

        char Buf[256];
        CTextTable TextTable;
        CMessages Messages;
        std::string MessagesFileName("Messages.txt");

        std::ifstream InStream(MessagesFileName, std::ifstream::in);
        if (!InStream.good()) {
            std::string Message("Failed to open \"");
            Message += MessagesFileName;
            Message += "\".";
            throw std::runtime_error(Message);
        }

        // Read and capture the heading line
        std::cout << std::endl;
        InStream.getline(Buf, sizeof(Buf) - 1);
        {
            std::wstring Line(Utf8ToWStr(Buf));
            std::cout << "\"" << Buf << "\"" << std::endl;
            std::vector<std::wstring> Values;
            TextTable.Parse(Line, Values);
            size_t Count = Values.size();
            for (size_t Ix = LangIx; Ix < Count; ++Ix) {
                std::vector<std::wstring> SubValues;
                TextTable.Parse(Values[Ix], SubValues);
                size_t NSubValues = SubValues.size();
                size_t Iy = 0;


                std::wstring LangName = (Iy < NSubValues) ? SubValues[Iy++] : L"???";

                CLanguageSpec::EStorage Storage = CLanguageSpec::None;
                if (Iy < NSubValues) {
                    if (SubValues[1] == L"Code")
                        Storage = CLanguageSpec::Code;
                    else if (SubValues[1] == L"Bin")
                        Storage = CLanguageSpec::File;
                    ++Iy;
                }

                uint32_t Order = 0;
                if (Iy < NSubValues) {
                    std::wstringstream StringStream(SubValues[Iy++]);
                    StringStream >> Order;
                }

                uint32_t CharSet = 0;
                if (Iy < NSubValues) {
                    std::wstringstream StringStream(SubValues[Iy++]);
                    StringStream >> CharSet;
                }

                CLanguageSpec LanguageSpec(LangName, Storage, 
                    static_cast<uint8_t>(Order), static_cast<uint8_t>(CharSet));
                Messages.LanguageSpecAdd(LanguageSpec);
            }
        }

        // Read and capture the translations
        while (!InStream.eof()) {
            InStream.getline(Buf, sizeof(Buf) - 1);
            if (*Buf != '\0') {
                std::cout << "\"" << Buf << "\"" << std::endl;
                std::wstring Line(Utf8ToWStr(Buf));
                std::vector<std::wstring> Values;
                TextTable.Parse(Line, Values);
                if (Values.size() < LangIx) {
                    std::stringstream Message;
                    Message << "Invalid record: \"" << Buf << "\".";
                    throw std::exception(Message.str().c_str());
                }
                CMessage Message;
                Message.Name(Values[NameIx]);
                Message.Description(Values[DescIx]);
                Message.Translate(Values[TypeIx] == L"T");
                size_t Count = Values.size();
                for (size_t Ix = LangIx; Ix < Count; ++Ix)
                    Message.TranslationAdd(Values[Ix]);
                Messages.MessageAdd(Message);
            }
        }

        InStream.close();

#ifdef VERBOSE
        // List translations for each language
        std::vector<CLanguageSpec> LanguageSpecs;
        Messages.LanguageSpecs(LanguageSpecs);
        for (CLanguageSpec LanguageSpec : LanguageSpecs) {
            std::cout << std::endl << WStrToUtf8(LanguageSpec.Name()) << ":" << std::endl;
            std::vector<std::wstring> Translations;
            Messages.Translations(LanguageSpec.Name(), Translations);
            for (std::wstring Translation : Translations)
                std::cout << "  \"" << WStrToUtf8(Translation) << "\""
                << std::endl;
        }
#endif // VERBOSE

        std::cout << std::endl;
        std::string Text("This is a CRC test:");
        std::cout << Text << std::endl;
        {
            CModbusCRC CRC;
            size_t Len = Text.size();
            for (size_t Ix = 0; Ix < Len; ++Ix)
                CRC.Add(Text[Ix]);
            std::cout << "CRC by byte = "
                << ToUpperHex(CRC.Value(), static_cast<size_t>(4))
                << std::endl;
        }
        {
            CModbusCRC CRC;
            size_t Len = Text.size();
            CRC.Add(reinterpret_cast<const uint8_t *>(Text.c_str()), Len);
            std::cout << "CRC by buffer = "
                << ToLowerHex(CRC.Value(), static_cast<size_t>(4))
                << std::endl;
        }

        {
            uint32_t NErrors = 0;
            std::cout << std::endl;
            std::vector<std::string> Report;

            NErrors += UtilsTest(Report);
            NErrors += MessagesTest(Report);

            std::cout << std::endl;
            for (std::string ReportLine : Report)
                std::cout << ReportLine << std::endl;

            std::cout << std::endl << "Number of errors: " << NErrors << "."
                << std::endl;
        }
        
        {
            std::vector<std::wstring> Translations;
            Messages.Translations(L"German", Translations);
            CLangImage LangImage;
            LangImage.Fill("German", Translations, 55, 1);
            std::cout << std::endl;
            LangImage.Show();
        }
    }
    catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        ExitCode = -1;
    }

    if (Switches.Exists(ESwitchID::Pause)) {
        std::cout << std::endl << "Press ENTER to exit..." << std::endl;
        std::cin.get();
    }
    return ExitCode;
}
