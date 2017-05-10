#include "stdafx.h"
#include "Switches.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>

//##############################################################################
// CSwitch
//##############################################################################
 
//------------------------------------------------------------------------------
// Constructor creates an object with a pointer to the corresponding switch
// spec and an empty list of parameters.
//
CSwitch::CSwitch(const CSwitchSpec *pswitchSpec, const std::string &switchText)
                 : mpSwitchSpec(pswitchSpec), mSwitchText(switchText) {
}

//------------------------------------------------------------------------------
// Copy constructor creates a copy of another object.
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
// Function copies the switch's parameters to parameters.
//
void CSwitch::Parameters(std::vector<std::string>  &parameters) const {
    parameters.clear();
    for (std::string Parameter : mParameters)
        parameters.push_back(Parameter);
}

//------------------------------------------------------------------------------
// Function first checks whether the switch is valid.  It the checks the number
// of parameters against the switch spec and throws exceptions if too few or
// too many.
//
void CSwitch::Check() const {
    // Check for invalid switch
    if (mpSwitchSpec->SwitchID == ESwitchID::None) {
        std::stringstream Message;
        Message << "Switch \"" << mSwitchText << "\" is not valid.";
        throw std::exception(Message.str().c_str());
    }

    // Check for too many or too few switches
    size_t NParameters = mParameters.size();
    std::string Relation;
    if (NParameters < mpSwitchSpec->MinParameters)
        Relation = "few";
    else if (NParameters > mpSwitchSpec->MaxParameters)
        Relation = "many";

    if (!Relation.empty()) {
        std::stringstream Message;
        Message << "Too " << Relation << " parameters for \""
                << mpSwitchSpec->SwitchText << "\".";
        throw std::exception(Message.str().c_str());
    }
}

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
// Copy constructor creates another object by copying the content of another
// object.
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

////------------------------------------------------------------------------------
//// Function adds switches and their parameters to the contained list of switches
//// while checking against the switch spec table.
////
//void CSwitches::ItemAdd(const std::string &item) {
//    // Check for empty item
//    if (item.size() == 0)
//        throw std::exception("Empty item.");
//
//    if (item[0] == '-') {  // If switch
//                           // Make sure previous switches are OK
//        Check();
//
//        // Find corresponding switch spec
//        const CSwitchSpec *pSwitchSpec = mpSwitchSpecTable;
//        while (pSwitchSpec->SwitchText != nullptr && item != pSwitchSpec->SwitchText)
//            ++pSwitchSpec;
//
//        // Check for unknown switch
//        if (pSwitchSpec->SwitchText == nullptr) {
//            std::stringstream Message;
//            Message << "Invalid switch: \"" << item << "\".";
//            throw std::exception(Message.str().c_str());
//        }
//
//        // Check for already defined switch
//        size_t FoundIx = Find(pSwitchSpec->SwitchID);
//        if (FoundIx != NotFound) {
//            std::stringstream Message;
//            Message << "Switch \"" << item << "\" is already defined";
//            std::string PreviousSwitchText = mSwitches[FoundIx].SwitchText();
//            if (item != PreviousSwitchText)
//                Message << " by switch \"" << PreviousSwitchText << "\"";
//            Message << ".";
//            throw std::exception(Message.str().c_str());
//        }
//
//        // Add switch
//        mSwitches.push_back(CSwitch(pSwitchSpec));
//    }
//    else {  // If parameter
//            // Add parameter if valid
//        size_t SwitchCount = mSwitches.size();
//        if (SwitchCount == 0) {
//            std::stringstream Message;
//            Message << "No switch is active for parameter \"" << item << "\".";
//            throw std::exception(Message.str().c_str());
//        }
//        size_t LastIx = SwitchCount - 1;
//        mSwitches[LastIx].ParameterAdd(item);
//    }
//}

//------------------------------------------------------------------------------
// Function adds switches and their parameters to the contained list of switches
// while checking against the switch spec table.
//
void CSwitches::ItemAdd(const std::string &item) {
    // Check for empty item
    if (item.size() == 0)
        throw std::exception("Empty item.");

    if (item[0] == '-') {  // If switch
        // Find corresponding switch spec
        const CSwitchSpec *pSwitchSpec = mpSwitchSpecTable;
        while (pSwitchSpec->SwitchText != nullptr && item != pSwitchSpec->SwitchText)
            ++pSwitchSpec;

    //    // Check for unknown switch
    //    if (pSwitchSpec->SwitchText == nullptr) {
    //        std::stringstream Message;
    //        Message << "Invalid switch: \"" << item << "\".";
    //        throw std::exception(Message.str().c_str());
    //    }

    //    // Check for already defined switch
    //    size_t FoundIx = Find(pSwitchSpec->SwitchID);
    //    if (FoundIx != NotFound) {
    //        std::stringstream Message;
    //        Message << "Switch \"" << item << "\" is already defined";
    //        std::string PreviousSwitchText = mSwitches[FoundIx].SwitchText();
    //        if (item != PreviousSwitchText)
    //            Message << " by switch \"" << PreviousSwitchText << "\"";
    //        Message << ".";
    //        throw std::exception(Message.str().c_str());
    //    }

        // Add switch
        mSwitches.push_back(CSwitch(pSwitchSpec, item));
    }
    else {  // If parameter
            // Add parameter if valid
        size_t SwitchCount = mSwitches.size();
        //if (SwitchCount == 0) {
        //    std::stringstream Message;
        //    Message << "No switch is active for parameter \"" << item << "\".";
        //    throw std::exception(Message.str().c_str());
        //}
        if (SwitchCount > 0) {
            size_t LastIx = SwitchCount - 1;
            mSwitches[LastIx].ParameterAdd(item);
        }
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
                Message << "Switch \"" << mSwitches[Iy].SwitchText()
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
// Function shows a list of switches and their corresponding parameters.
//
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
