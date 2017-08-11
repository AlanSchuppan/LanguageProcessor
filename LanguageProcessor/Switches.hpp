//#pragma once

#ifndef SWITCHES_DEFD
#define SWITCHES_DEFD

#include <string>
#include <vector>

// Define IDs corresponding to each switch. None indicates no switch and can be 
// used in error situations or list termination.
enum class ESwitchID { None, Help, Verbose, Test, Pause, Cpp, Language, Messages };

//##############################################################################
// CSwitches
//##############################################################################
// Structure describes a switch by providing its text, ID, and the minimum and
// maximum number of expected parameters.
//##############################################################################

struct CSwitchSpec {
    char     *SwitchText;
    ESwitchID SwitchID;
    uint16_t  MinParameters;
    uint16_t  MaxParameters;
};

//##############################################################################
// CSwitch
//##############################################################################
// Class contains the switch, its ID, text, and all its parameters.
//##############################################################################

class CSwitch {
public:
    CSwitch(const CSwitchSpec *pswitchSpecTable, const std::string &switchText);
    CSwitch(const CSwitch &other);

    void ParameterAdd(const std::string &parameter);
    void Parameters(std::vector<std::string>  &parameters) const;
    ESwitchID SwitchID() const { return mpSwitchSpec->SwitchID; }
    std::string SwitchText() const { return mSwitchText; }
    void Check() const;

private:
    const CSwitchSpec *mpSwitchSpec;
    std::string mSwitchText;
    std::vector<std::string>  mParameters;
};

//##############################################################################
// CSwitches
//##############################################################################
// Class contains a list of command line switches.
//##############################################################################

class CSwitches {
public:
    CSwitches(const CSwitchSpec *pswitchSpecTable);
    CSwitches(const CSwitches &other);

    std::string ExecPath() const { return mExecPath; }
    void ExecPath(const std::string &path);
    void ItemAdd(const std::string &item);
    bool Exists(ESwitchID switchID) const;
    bool Parameters(ESwitchID switchID,
                    std::vector<std::string> &parameters) const;
    void Show() const;
    void Check() const;

private:
    static const size_t NotFound = 0xffff;

    std::string mExecPath;
    const CSwitchSpec *mpSwitchSpecTable;
    std::vector<CSwitch> mSwitches;
    std::string mUnknownSwitch;
    bool mUnknownSwitchMode;

    size_t Find(ESwitchID switchID) const;
};

//------------------------------------------------------------------------------

#endif // SWITCHES_DEFD