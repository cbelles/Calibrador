#ifndef _LICENSE_H
#define _LICENSE_H

#ifdef __unix__ 
#define OS_Linux 1
#elif defined(_WIN32) || defined(WIN32)
#define OS_Linux 0
#endif

#ifdef __unix__ 
#include <licensecc/licensecc.h>
#include "hw_identifier_facade.hpp"
#include "ADVobfuscator_Lib/MetaString.h"
#include "ADVobfuscator_Lib/ObfuscatedCall.h"
#include "ADVobfuscator_Lib/ObfuscatedCallWithPredicate.h"
using namespace andrivet::ADVobfuscator;
using namespace license::hw_identifier;
#endif

#ifdef __unix__
void SalirPrograma0()
{
	exit(0);
}
// Obfuscate function calls
void SalirProgramaFiniteStateMachine0()
{
	using namespace andrivet::ADVobfuscator::Machine1;
	OBFUSCATED_CALL0(SalirPrograma0);
}

#include <mutex>

std::mutex mutex_HwIdentifierFacade_serial_number;

#endif

#ifdef __unix__ 
#include "Linux/network.h"
#endif

#endif 