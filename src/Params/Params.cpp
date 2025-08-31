#include "Params/Params.h"

#include <fstream>
#include <iostream>

#ifdef __unix__
#include <licensecc/licensecc.h>
#include "ADVobfuscator_Lib/MetaString.h"
#include "ADVobfuscator_Lib/ObfuscatedCall.h"
#include "ADVobfuscator_Lib/ObfuscatedCallWithPredicate.h"
using namespace andrivet::ADVobfuscator;

void SalirPrograma1()
{
	exit(0);
}
// Obfuscate function calls
void SalirProgramaFiniteStateMachine1()
{
	using namespace andrivet::ADVobfuscator::Machine1;
	OBFUSCATED_CALL0(SalirPrograma1);
}

#endif

using namespace std;

Params::Params(std::string& filename)
{
	//02/07/2020 Quito de momento el fichero de licencia mac.lic
	//Estaba poniendo el hw_identifier DISK NUM, pero al hacer copias de tarjetas SD he visto que FruitVision se ejecutaba sin haber 
	//cambiado los mac.lic, despues he comprobado que DISK NUM se copia al hacer un clone de la SD, es un numero software , no Hardware, no me vale.
	//Solo queda la posibilidad de usar el MAC ADRESS (Ethernet), pero habian problemas en Raspberry
	//De momento dejo solo el fichero de licencia del numero de serie de la camara
	

//#ifdef LICENSE 
    //LICENSE
 
	std::size_t found = filename.find_last_of("/\\");
	string pathconfig = filename.substr(0, found);
	//=========================================================================================
	LicenseInfo licenseInfo;
	LicenseLocation licLocation = { LICENSE_PATH };
	string filelicensename = pathconfig + OBFUSCATED("/mac.lic");
	std::copy(filelicensename.begin(), filelicensename.end(), licLocation.licenseData);
	LCC_EVENT_TYPE result = acquire_license(nullptr, &licLocation, &licenseInfo);

	if (result == LICENSE_OK) {
		if (!licenseInfo.linked_to_pc) {
			cout << OBFUSCATED("$$$$$$$") << endl;
			SalirProgramaFiniteStateMachine1();
		}
	}
	else
	{
		cout << OBFUSCATED("$$$$$$$") << endl;
		SalirProgramaFiniteStateMachine1();
	}
	//=========================================================================================
//#endif

	_filename = filename;
}
	
