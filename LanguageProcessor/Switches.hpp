//#pragma once

#ifndef SWITCHES_DEFD
#define SWITCHES_DEFD

#include <string>
#include <vector>

enum class ESwitchID { None, Help, Verbose, Pause, Language };

//##############################################################################

struct CSwitchSpec {
    char     *SwitchText;
    ESwitchID SwitchID;
    uint16_t  MinParameters;
    uint16_t  MaxParameters;
};

//##############################################################################

class CSwitch {
public:
    CSwitch(const CSwitchSpec *pswitchSpec, const std::string &switchText);
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