#include "stdafx.h"
#include "Switches.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>

//##############################################################################
// CSwitch
//##############################################################################
// Class contains the switch, its ID, text, and all its parameters. It can also
// check the switch's validity.
//##############################################################################

//------------------------------------------------------------------------------
// Constructor creates an object with a pointer to the corresponding switch
// spec within the SwitchSpec table and an empty list of parameters.
//
CSwitch::CSwitch(const CSwitchSpec *pswitchSpecTable,
                 const std::string &switchText)
    : mpSwitchSpec(pswitchSpecTable), mSwitchText(switchText) {
    // Find the appropriate Switch Spec or ESwitchID::None if not found
    while (mpSwitchSpec->SwitchID != ESwitchID::None &&
        switchText != mpSwitchSpec->SwitchText)
        ++mpSwitchSpec;
}

//------------------------------------------------------------------------------
// Copy constructor creates a copy of another CSwitch.
//
CSwitch::CSwitch(const CSwitch &other) : mpSwitchSpec(other.mpSwitchSpec),
                                         mSwitchText(other.mSwitchText) {
    for (std::string Parameter : other.mParameters)
        mParameters.push_back(Parameter);
}

//------------------------------------------------------------------------------
// Function adds a parameter to the switch.
//
void CSwitch::ParameterAdd(const std::string &parameter) {
    mParameters.push_back(parameter);
}

//------------------------------------------------------------------------------
// Function outputs the switch's parameters to parameters.
//
void CSwitch::Parameters(std::vector<std::string>  &parameters) const {
    parameters.clear();
    for (std::string Parameter : mParameters)
        parameters.push_back(Parameter);
}

//------------------------------------------------------------------------------
// Function first checks whether the switch is valid.  It then checks the number
// of parameters against the switch spec. An exception is thrown if there are
// any problems.
//
void CSwitch::Check() const {
    // Check for invalid (unknown) switch
    if (mpSwitchSpec->SwitchID == ESwitchID::None) {
        std::stringstream Message;
        Message << "CSwitch::Check(): " << "Switch \"" << mSwitchText
                << "\" is not valid.";
        throw std::exception(Message.str().c_str());
    }

    // Check for wrong number of switch parameters
    size_t NParameters = mParameters.size();
    if (NParameters < mpSwitchSpec->MaxParameters ||
        NParameters > mpSwitchSpec->MaxParameters) {
        std::stringstream Message;
        Message << "CSwitch::Check(): "
                << "Number of parameters expected for \"" 
                << mpSwitchSpec->SwitchText << "\": " 
                << mpSwitchSpec->MinParameters;
        if (mpSwitchSpec->MaxParameters > mpSwitchSpec->MinParameters)
            Message << " to " << mpSwitchSpec->MaxParameters;
        Message << ".";
        throw std::exception(Message.str().c_str());
    }
}

//##############################################################################
// CSwitches
//##############################################################################
// Class contains a list of switches.
//##############################################################################

//------------------------------------------------------------------------------
// Constructor creates a container for a list of switches a pointer to an
// external switch spec table, so that it knows what switches are valid and how
// many parameters to expect.
//
CSwitches::CSwitches(const CSwitchSpec *pswitchSpecTable) : 
    mpSwitchSpecTable(pswitchSpecTable), mUnknownSwitchMode(false) {
}


//------------------------------------------------------------------------------
// Copy constructor initializes this object with the content of another.
//
CSwitches::CSwitches(const CSwitches &other) :
    mpSwitchSpecTable(other.mpSwitchSpecTable) {
    for (CSwitch Switch : other.mSwitches)
        mSwitches.push_back(Switch);
}

//------------------------------------------------------------------------------
// Function accepts the executable path, strips the executable file name, and
// saves the path only.
//
void CSwitches::ExecPath(const std::string &path) {
    size_t LastIx = path.find_last_of("\\/");
    if (LastIx == std::string::npos)
        LastIx = path.size() - 1;
    mExecPath = path.substr(0, LastIx + 1);
}

//------------------------------------------------------------------------------
// Taking one command line token at a time, this function adds switches and
// their parameters to the contained list of switches while checking against
// the switch spec table.
//
void CSwitches::ItemAdd(const std::string &item) {
    // Check for empty item
    if (item.size() == 0)
        throw std::exception("CSwitches::ItemAdd(): Empty item.");

    if (item[0] == '-') {  // If switch
        // Find corresponding switch spec
        const CSwitchSpec *pSwitchSpec = mpSwitchSpecTable;
        while (pSwitchSpec->SwitchID != ESwitchID::None &&
               item != pSwitchSpec->SwitchText)
            ++pSwitchSpec;

        // Add switch
        mSwitches.push_back(CSwitch(pSwitchSpec, item));
    }
    else {  // Add parameter if valid and switch active
        size_t SwitchCount = mSwitches.size();
        if (SwitchCount == 0) {
            std::stringstream Message;
            Message << "CSwitches::ItemAdd(): " 
                    << "No switch is active for parameter \"" << item << "\".";
            throw std::exception(Message.str().c_str());
        }
        mSwitches[SwitchCount - 1].ParameterAdd(item);
    }
}

//------------------------------------------------------------------------------
// Private function returns the index of the specified switch ID. If not found,
// NotFound is returned.
//
size_t CSwitches::Find(ESwitchID switchID) const {
    size_t Count = mSwitches.size();
    for (size_t Ix = 0; Ix < Count; ++Ix)
        if (mSwitches[Ix].SwitchID() == switchID)
            return Ix;
    return NotFound;
}

//------------------------------------------------------------------------------
// Function returns true if the specified switch has been defined.
//
bool CSwitches::Exists(ESwitchID switchID) const {
    size_t Ix = Find(switchID);
    return (Ix != NotFound);
}

//------------------------------------------------------------------------------
// Function returns true if the specified switch has been defined and copies
// the corresponding parameters into parameters. If not found, the list of
// parameters will be empty and false is returned.
//
bool CSwitches::Parameters(ESwitchID switchID, std::vector<std::string>  &parameters) const {
    size_t Ix = Find(switchID);
    bool Found = (Ix != NotFound);
    if (Found)
        mSwitches[Ix].Parameters(parameters);
    else
        parameters.clear();
    return Found;
}

//------------------------------------------------------------------------------
// Function checks for invalid switches, checks the parameter count for all
// switches, checks for repeated switches, and then throws exceptions
// accordingly.
//
void CSwitches::Check() const {
    // Check for invalid switches and incorrect parameter counts
    for (CSwitch Switch : mSwitches)
        Switch.Check();

    // Check for repeated switches (must be last)
    size_t Count = mSwitches.size();
    for (size_t Iy = 1; Iy < Count; ++Iy) {
        ESwitchID SwitchID = mSwitches[Iy].SwitchID();
        for (size_t Ix = 0; Ix < Iy; ++Ix) {
            if (mSwitches[Ix].SwitchID() == SwitchID) {
                std::stringstream Message;
                Message << "CSwitches::Check(): " << "Switch \"" 
                        << mSwitches[Iy].SwitchText()
                        << "\" is already specified";
                if (mSwitches[Ix].SwitchText() != mSwitches[Iy].SwitchText())
                    Message << " by \"" << mSwitches[Ix].SwitchText() << "\"";
                Message << ".";
                throw std::exception(Message.str().c_str());
            }
        }
    }

}

//------------------------------------------------------------------------------
// Function shows, on std out, a list of switches and their corresponding
// parameters.
void CSwitches::Show() const {
    std::cout << mExecPath;
    std::cout << std::endl << "Switches" << std::endl;
     for (const CSwitch Switch : mSwitches) {
        std::cout << "  " << Switch.SwitchText() << std::endl;
        std::vector<std::string> Parameters;
        Switch.Parameters(Parameters);
        for (std::string Parameter : Parameters)
            std::cout << "   " << Parameter << std::endl;
    }
}
