#include "Params/ParamsFruta.h"
#include <fstream>
#include <iostream>

ParamsFruta::ParamsFruta(string& filename) : Params(filename) {
    // Initialize maps
    _variedadesPorFruta.clear();
    _colorIndexPorFruta.clear();
}

bool ParamsFruta::save()
{
	return true;
}

bool ParamsFruta::load() {
    try {
        std::ifstream file(_filename);
        if (!file.is_open()) return false;

        nlohmann::json j;
        file >> j;

        _variedadesPorFruta.clear();
        _colorIndexPorFruta.clear();
        
        // Iterate directly over the root object
        for (auto it = j.begin(); it != j.end(); ++it) {
            string fruta = it.key();
            
            // Store color index
            _colorIndexPorFruta[fruta] = it.value()["colorIndex"];
            
            // Store varieties
            vector<string> variedades;
            for (const auto& var : it.value()["VARIEDADES"]) {
                variedades.push_back(var);
            }
            _variedadesPorFruta[fruta] = variedades;
        }

        return true;
    }
    catch (const exception& e) {
        return false;
    }
}

// Métodos adicionales para acceder a la información
string ParamsFruta::getColorIndex(const string& fruta) const {
    auto it = _colorIndexPorFruta.find(fruta);
    return (it != _colorIndexPorFruta.end()) ? it->second : "";
}

vector<string> ParamsFruta::getVariedades(const string& fruta) const {
    auto it = _variedadesPorFruta.find(fruta);
    return (it != _variedadesPorFruta.end()) ? it->second : vector<string>();
}

vector<string> ParamsFruta::getFrutas() const {  
    vector<string> frutas;
    for (const auto& [fruta, _] : _variedadesPorFruta) {
        frutas.push_back(fruta);
    }
    return frutas;
}

