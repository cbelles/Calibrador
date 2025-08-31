#ifndef _PARAMSFRUTA_H
#define _PARAMSFRUTA_H

#include "Params/Params.h"
#include <string>
#include <map>
#include "Utils/json.hpp"
#include <vector>

using json = nlohmann::json;
using namespace std;

class ParamsFruta : public Params {
public:
    ParamsFruta(string& name);

    bool load();
    bool save();

    string getColorIndex(const string& fruta) const;
    vector<string> getVariedades(const string& fruta) const;
    vector<string> getFrutas() const; 

private:
    // Map que almacena las variedades por fruta
    std::map<string, vector<string>> _variedadesPorFruta;
    // Map que almacena el índice de color por fruta
    std::map<string, string> _colorIndexPorFruta;
};

#endif // _PARAMSFRUTA_H
