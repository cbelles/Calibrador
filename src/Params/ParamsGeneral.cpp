#include "Params/ParamsGeneral.h"

ParamsGeneral::ParamsGeneral(string& filename) : Params(filename)
{
}

bool ParamsGeneral::load()
{
	// read a JSON file
	ifstream i(_filename);
	if (!i.is_open())
		return false;

	json j;
	i >> j;

	_dimX_window = j["dimwindow"]["dimX"]; 
	_dimY_window = j["dimwindow"]["dimY"]; 

	_showLogo = j["logo"]["showLogo"];
	_logo_dimX = j["logo"]["dimX"];
	_logo_dimY = j["logo"]["dimY"];

	 _debugMode = j["general"]["debugMode"];
	 _debugMode_ciclo_ms = j["general"]["debugMode_ciclo_ms"];
	_pathPrograms = j["general"]["pathPrograms"];
	_pathConfig = j["general"]["pathConfig"];
	_pathPartidas = j["general"]["pathPartidas"];

	
	_num_lineas = j["config"]["numLineas"];
	_num_salidas = j["config"]["numSalidas"];
	_idioma 	= j["config"]["idioma"];

	_dimensiones_name.clear();
	for (const auto& dim : j["dimensiones"]["nombre"]) {
		_dimensiones_name.push_back(dim);
	}

	_dimensiones_min_value.clear();
	for (const auto& min : j["dimensiones"]["domain_min_value"]) {
		_dimensiones_domain_min_value.push_back(min);
	}

	_dimensiones_max_value.clear();
	for (const auto& max : j["dimensiones"]["domain_max_value"]) {
		_dimensiones_domain_max_value.push_back(max);
	}

	return true;
}

bool ParamsGeneral::save()
{
	json j;

	j["dimwindow"]["dimX"] = _dimX_window;
	j["dimwindow"]["dimY"] = _dimY_window;

	j["logo"]["showLogo"] = _showLogo;
	j["logo"]["dimX"] = _logo_dimX;
	j["logo"]["dimY"] = _logo_dimY;

	j["general"]["debugMode"] = _debugMode;	
	j["general"]["debugMode_ciclo_ms"] = _debugMode_ciclo_ms; 
	j["general"]["pathPrograms"] = _pathPrograms;
	j["general"]["pathConfig"] = _pathConfig;
	j["general"]["pathPartidas"] = _pathPartidas;


	j["config"]["numLineas"] = _num_lineas;
	j["config"]["numSalidas"] = _num_salidas;
	j["config"]["idioma"] = _idioma;	

	j["dimensiones"]["nombre"] = _dimensiones_name;
	j["dimensiones"]["domain_min_value"] = _dimensiones_domain_min_value;
	j["dimensiones"]["domain_max_value"] = _dimensiones_domain_max_value;

	ofstream o(_filename);
	const auto s2 = j.dump(4);    //Dum es necesario para no perder la estructura
	o << s2 << endl;
	o.close();

	return true;
}

