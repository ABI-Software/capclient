

#ifndef _CAP_DEBUG_H
#define _CAP_DEBUG_H

#include <string>
#include <sstream>


#ifdef _MSC_VER
# define WINDOWS_LEAN_AND_MEAN
# define NOMINMAX
# include <windows.h>
#else
# include <iostream>
#endif

/**
 * Print debug message, uses printf on *nix based systems
 * and OutputDebugString on windows if using Visual Studio.
 *
 * @see dbgn
 * @param	msg	The message to print out to console.
 */
void dbg(const std::string& msg);

/**
 * Print debug message with no newline, uses printf on *nix 
 * based systems and OutputDebugString on windows if using 
 * Visual Studio.
 *
 * @see dbg
 * @param	msg	The message to print out to console.
 */
void dbgn(const std::string& msg);

#endif

