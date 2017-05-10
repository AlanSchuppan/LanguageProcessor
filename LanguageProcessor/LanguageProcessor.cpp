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
#include "Switches.hpp""

#define VERBOSE

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
            for (size_t Ix = LangIx; Ix < Count; ++Ix)
                Messages.LanguageAdd(Values[Ix]);
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
        std::vector<std::wstring> Languages;
        Messages.Languages(Languages);
        for (std::wstring Language : Languages) {
            std::cout << std::endl << WStrToUtf8(Language) << ":" << std::endl;
            std::vector<std::wstring> Translations;
            Messages.Translations(Language, Translations);
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
