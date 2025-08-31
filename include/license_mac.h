#ifdef __unix__ 	
//LICENSE MAC
//=========================================================================================
LicenseInfo licenseInfo;
LicenseLocation licLocation = { LICENSE_PATH };
string filelicensename = pathconfig + OBFUSCATED("/mac.lic");
std::copy(filelicensename.begin(), filelicensename.end(), licLocation.licenseData);
LCC_EVENT_TYPE result = acquire_license(nullptr, &licLocation, &licenseInfo);
if (result == LICENSE_OK) {
	if (!licenseInfo.linked_to_pc) {
		cout << OBFUSCATED("1$$$$$$$") << endl;
		SalirProgramaFiniteStateMachine0();
	}
}
else
{
	cout << OBFUSCATED("1$$$$$$$") << endl;
	SalirProgramaFiniteStateMachine0();
}
//=========================================================================================
#endif

