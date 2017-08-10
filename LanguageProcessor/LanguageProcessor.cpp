//#ifdef _WIN32 || _WIN64
//#ifdef _M_IX86 || _M_X64
#include "stdafx.h"
//#endif // _WIN32 || _WIN64

#include <direct.h>
#include <errno.h>

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
#include "LangImage.hpp"

//##############################################################################

void MakeFolder(const std::string &path);

void MakeFolder(const std::string &path) {
    std::vector<std::string> Path;
    size_t StartIx = 0;
    size_t PathLength = path.size();
    while (StartIx < PathLength) {
        size_t EndIx = path.find_first_of("/\\", StartIx);
        if (EndIx == std::string::npos)
            EndIx = PathLength;
        Path.push_back(path.substr(StartIx, EndIx - StartIx));
        StartIx = EndIx + 1;
    }

    std::string Folder;
    size_t Count = Path.size();
    for (size_t Ix = 0; Ix < Count; ++Ix) {
        Folder += Path[Ix];
        if (!Folder.empty() && _mkdir(Folder.c_str()) != 0 && errno == ENOENT) {
            std::string Message("Could not create \"");
            Message += Folder;
            Message += "\".";
            throw std::runtime_error(Message);
        }
        Folder += "/";
    }
}

int main(int argc, char** argv) {
    static const size_t NameIx = 0;
    static const size_t DescIx = 1;
    static const size_t TypeIx = 2;
    static const size_t LangIx = 3;
    static const CSwitchSpec SwitchSpecs[] = {
        { "-h",    ESwitchID::Help,     0, 0 },
        { "-?",    ESwitchID::Help,     0, 0 },
        { "-cpp",  ESwitchID::Cpp,      1, 1 },
        { "-l",    ESwitchID::Language, 1, 1 },
        { "-m",    ESwitchID::Messages, 1, 1 },
        { "-p",    ESwitchID::Pause,    0, 0 },
        { "-v",    ESwitchID::Verbose,  0, 0 },
        { "-t",    ESwitchID::Test,     0, 0 },
        { nullptr, ESwitchID::None,     0, 0 } // Terminator
    };

    int ExitCode = 0;
    CSwitches Switches(SwitchSpecs);
    static const char *Indent = "    ";
    size_t IndentLen = std::string(Indent).size();

    try {
        Switches.ExecPath(argv[0]);
        for (int Ix = 1; Ix < argc; ++Ix)
            Switches.ItemAdd(argv[Ix]);
        Switches.Check();
        if (Switches.Exists(ESwitchID::Verbose)) {
            Switches.Show();
            std::cout << std::endl;
        }

        // If help requested, display help and quit
        if (Switches.Exists(ESwitchID::Help)) {
            const char *HelpMessage[] = {
                "LangProcessor [<options>] <language Excel file>",
                "  -cpp",
                "     Specifies the folder where the C++ source code will be placed.  If not",
                "     present, no C++ source code will be generated.",
                "",
                "  -l <language folder>",
                "     Specifies the folder where language files will be placed.  if not",
                "     provided, no language files will be generated.",
                "",
                //"  -n",
                //"     Cause the generated C plus plus file to use namespaces.",
                //"",
                //"  -c[r] <config source folder>",
                //"     Specifies the folder where source configuration files can be found, where",
                //"     the optional 'r' makes the search recursive.  If not provided, no",
                //"     configuration files will be processed.",
                //"",
                //"  -cd <config destination folder>",
                //"     Specifies the folder where processed (default) configuration files will",
                //"     be placed.  If not provided, the source folder will be used.",
                //"",
                //"  -u <character usage folder>",
                //"     Specifies the folder where character usage files will be placed.  if not",
                //"     provided, no character usage files will be generated.",
                //"",
                //"  -w",
                //"     Causes a new working configuration file to be generated as a copy of each",
                //"     default file.",
                //"",
                "  -v",
                "     Enables the verbose mode, which generates extra informational output.",
                "",
                "  -p",
                "     Causes the program to pause and wait for a key press before exiting so",
                "     that the standard output can be read.",
                "",
                "  -h",
                "  -?",
                "     Outputs this message and stops.  (All other switches are ignored.)",
                "",
                nullptr
            };

            for (const char **pLine = HelpMessage; *pLine != nullptr; ++pLine)
                std::wcout << *pLine << std::endl;
            if (Switches.Exists(ESwitchID::Pause)) {
                std::cout << std::endl << "Press ENTER to exit..." << std::endl;
                std::cin.get();
            }
            return 0;
        }

        // Check for specified message file
        if (!Switches.Exists(ESwitchID::Messages))
            throw(std::runtime_error("Must include message file."));

       CMessages Messages;
       { // Load the message file data
            std::vector<std::string> Parameters;
            Switches.Parameters(ESwitchID::Messages, Parameters);

            char Buf[256];
            CTextTable TextTable;
            std::string MessagesFileName(Parameters[0]);

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
        }

        // If requested, show extra information
        if (Switches.Exists(ESwitchID::Verbose)) {
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
        }

        // If requested, perform some tests
        if (Switches.Exists(ESwitchID::Test)) {
            std::cout << std::endl;
            std::string Text("This is a CRC test:");
            std::cout << Text << std::endl;
            {
                CModbusCRC CRC;
                size_t Len = Text.size();
                for (size_t Ix = 0; Ix < Len; ++Ix)
                    CRC.Add(Text[Ix]);
                std::cout << "  CRC by byte = "
                    << ToUpperHex(CRC.Value(), static_cast<size_t>(4))
                    << std::endl;
            }
            {
                CModbusCRC CRC;
                size_t Len = Text.size();
                CRC.Add(reinterpret_cast<const uint8_t *>(Text.c_str()), Len);
                std::cout << "  CRC by buffer = "
                    << ToLowerHex(CRC.Value(), static_cast<size_t>(4))
                    << std::endl;
            }

            {
                uint32_t NErrors = 0;
                //std::cout << std::endl;
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

        // If requested, generate C++ code for code-based messages
        if (Switches.Exists(ESwitchID::Cpp)) {
            std::vector<std::string> Parameters;
            Switches.Parameters(ESwitchID::Cpp, Parameters);
            MakeFolder(Parameters[0]);
            std::string File(Parameters[0]);
            File += "/Messages.h";
            std::ofstream OutStream(File, std::ifstream::out);

            OutStream << "// This file was generated by LangProcessor and should not be edited." << std::endl;
            OutStream << std::endl;
            OutStream << "#if !defined(SOURCE) && !defined(LANG_PROCESSOR)" << std::endl;
            OutStream << "#define LANG_PROCESSOR" << std::endl;

            std::vector<std::string> EnumNames;
            Messages.EnumNames(EnumNames);

            { // Output enums
                OutStream << std::endl;
                OutStream << "// Define message indices." << std::endl;
                OutStream << "enum class EMsg {";
                EnumNames.push_back("NMessages");
                size_t LineLen = 80;
                for (std::string EnumName : EnumNames) {
                    size_t NCommas = (EnumName != "NMessages") ? 1 : 0;
                    LineLen += EnumName.size() + NCommas + 1;
                    if (LineLen > 80) {
                        OutStream << std::endl << Indent << EnumName;
                        LineLen = EnumName.size() + IndentLen + NCommas;
                    } 
                    else
                        OutStream << ' ' << EnumName;
                    if (NCommas > 0)
                        OutStream << ',';
                }
                EnumNames.erase(EnumNames.end() - 1);
            }
            OutStream << std::endl << "};" << std::endl;

            OutStream << std::endl;
            OutStream << "#endif //LANG_PROCESSOR" << std::endl;
            OutStream << std::endl;
            OutStream << std::endl;
            OutStream << "#ifdef SOURCE" << std::endl;

            std::vector<CLanguageSpec> LanguageSpecs;
            Messages.LanguageSpecs(LanguageSpecs);

            for (auto It = LanguageSpecs.begin(); It != LanguageSpecs.end(); ++It)
                if (It->Storage() == CLanguageSpec::EStorage::Code) {
                    std::string Language(WStrToUtf8(It->Name()));
                    CLangImage LangImage;
                    std::vector<std::wstring> Translations;
                    Messages.Translations(Language, Translations);
                    LangImage.Fill(Language, Translations, It->Order(), It->CharSet());
                    OutStream << std::endl;
                    LangImage.SaveAsCode(EnumNames, OutStream, Indent);
                }

            OutStream << std::endl;
            OutStream << "// Pointers to code-based language images" << std::endl;
            OutStream << "const unsigned char *LangImage[] = {" << std::endl;
            for (auto It = LanguageSpecs.begin(); It != LanguageSpecs.end(); ++It)
                if (It->Storage() == CLanguageSpec::EStorage::Code)
                    OutStream << Indent << WStrToUtf8(It->Name()) << ',' << std::endl;
            OutStream << Indent << "nullptr" << std::endl;
            OutStream << "};" << std::endl;
            OutStream << std::endl;
        }

        // if specified, generate binary files fo file-based languages
        if (Switches.Exists(ESwitchID::Language)) {
            std::vector<std::string> Parameters;
            Switches.Parameters(ESwitchID::Language, Parameters);
            MakeFolder(Parameters[0]);
            _mkdir(Parameters[0].c_str());

            std::vector<CLanguageSpec> LanguageSpecs;
            Messages.LanguageSpecs(LanguageSpecs);
            for (auto It = LanguageSpecs.begin(); It != LanguageSpecs.end(); ++It)
                if (It->Storage() == CLanguageSpec::EStorage::File) {
                    std::string Language(WStrToUtf8(It->Name()));
                    std::string File(Parameters[0]);
                    File += "\\";
                    File += Language;

                    std::ofstream OutStream(File, std::ifstream::out);
                    if (!OutStream.good()) {
                        std::string Message("Could not create \"");
                        Message += File;
                        Message += "\".";
                        throw std::runtime_error(Message);
                    }
                    CLangImage LangImage;
                    std::vector<std::wstring> Translations;
                    // Messages.Translations(L"German", Translations);
                    Messages.Translations(Language, Translations);
                    LangImage.Fill(Language, Translations, 0, 0);
                    OutStream.write(reinterpret_cast<const char *>(LangImage.Image()), LangImage.Size());
                }

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
