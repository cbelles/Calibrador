#ifndef _PARAMSGENERAL_H
#define _PARAMSGENERAL_H

#include "Params/Params.h"
#include <string>
#include "Utils/json.hpp"
#include <vector>

using  json = nlohmann::json;
using namespace std;

class ParamsGeneral : public Params
{

public:

	ParamsGeneral(string& name);

	bool load();
	bool save();

	//dimwindow
	
	int get_dimX_window() { return _dimX_window; }
	int get_dimY_window() { return _dimY_window; }
	
	//logo
	
	bool get_showLogo() { return _showLogo; }
	int get_logo_dimX() { return _logo_dimX; }
	int get_logo_dimY() { return _logo_dimY; }

	//general

	bool get_debugMode() { return _debugMode; }
	int get_debugMode_ciclo_ms() { return _debugMode_ciclo_ms; }
	
	string get_pathPrograms() { return _pathPrograms; }
	string get_pathConfig() { return _pathConfig; }
	string get_pathPartidas() { return _pathPartidas; }
	

	//config

	int get_num_lineas() { return _num_lineas; }
	int get_num_salidas() { return _num_salidas; }
	string get_idioma() { return _idioma; }
	void get_dimensiones_name(vector<string>& dimensiones_name) { dimensiones_name = _dimensiones_name; }
	//const vector<int>& get_dimensiones_min_value() { return _dimensiones_min_value; }
	//const vector<int>& get_dimensiones_max_value() { return _dimensiones_max_value; }
	
	void get_dimensiones_domain_value(vector<pair<int,int>>& dimensiones_domain_value) { 
        dimensiones_domain_value.clear();
        // Make sure both vectors have the same size
        if (_dimensiones_domain_min_value.size() == _dimensiones_domain_max_value.size()) {
            for (size_t i = 0; i < _dimensiones_domain_min_value.size(); i++) {
                dimensiones_domain_value.push_back(
                    make_pair(_dimensiones_domain_min_value[i], _dimensiones_domain_max_value[i])
                );
            }
        }
    }
	
private:

	//dimwindow
	int _dimX_window;
	int _dimY_window;

	//logo
	bool _showLogo;
	int _logo_dimX;
	int _logo_dimY;

	//general

	bool _debugMode;
	int _debugMode_ciclo_ms;
	string _pathPrograms;
	string _pathConfig;
	string _pathPartidas;

	//config

	int _num_lineas;
	int _num_salidas;
	string _idioma;
	vector<string> _dimensiones_name;
	vector<int> _dimensiones_min_value;
	vector<int> _dimensiones_max_value;
    vector<pair<int,int>> _dimensiones_domain_value;
	vector<int> _dimensiones_domain_min_value;
	vector<int> _dimensiones_domain_max_value;

};

#endif
