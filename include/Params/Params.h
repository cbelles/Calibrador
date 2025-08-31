#ifndef _PARAMS_H
#define _PARAMS_H

#include <string>
#include <fstream>

class Params
{


public:

	/*! */
	Params(std::string& filename);

	/*! */
	std::string get_filename() { return _filename; }
	
public:

	/*! */
	std::string _filename;


};

#endif 