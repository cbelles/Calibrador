#include "CalibrationProgram.h"
#include "Utils/json.hpp"
#include <iomanip>  // Añadido para std::setw
#include <fstream>  // Añadido para std::ofstream
#include <iostream> // Añadido para std::cout

using json = nlohmann::json;

void CalibrationProgram::clear()
{
    _name.clear();
    _calibers.clear();
    // Clear dimension names es poner las tres dimensiones a ""
    _dimensionNames.resize(3);
    _dimensionNames[0] = "";_dimensionNames[1] = "";_dimensionNames[2] = "";
    
}

bool CalibrationProgram::load(const string& name) {

    std::cout << "Loading program: " << name << std::endl;
    
    try {
        // Leer archivo JSON
        std::ifstream file(name);
        json j;
        file >> j;
        
        // Cargar nombre
        setName(j["name"].get<std::string>());

        //Cargar Fruta
        setFruta(j["fruta"].get<std::string>(),j["colorindexfruta"].get<std::string>());
        //Cargar Variedad
        setVariedad(j["variedad"].get<std::string>());

        //Cargar nombres de las dimensiones
        _dimensionNames.clear();
        for (const auto& dimName : j["dimensionNames"]) {
            _dimensionNames.push_back(dimName.get<std::string>());
        }

        // Cargar calibres
        _calibers.clear();
        for (const auto& caliberJson : j["calibers"]) {
            Caliber caliber(caliberJson["name"].get<std::string>());
            
            // Cargar dimensiones
            CalibrationDimension dimension;
            for (const auto& dimJson : caliberJson["dimensions"]) {
                int index = dimJson["type"].get<int>();
                string name = _dimensionNames[index];
                dimension.setDimension(index, name);
                
                pair<int,int> domain = CalibrationDimension::_dimension_domain_value[index];
                dimension.setDomain(domain);
                dimension.setMinValue(dimJson["min"].get<int>());
                dimension.setMaxValue(dimJson["max"].get<int>());
 
                caliber.addDimension(dimension);
            }
            
            addCaliber(caliber);
        }

         // Cargar las salidas
        _salidas.clear();
        if (j.contains("salidas") && j["salidas"].is_array()) {
            for (const auto& caliber_obj : j["salidas"]) {
                // Cada elemento es un objeto que contiene un array
                for (auto it = caliber_obj.begin(); it != caliber_obj.end(); ++it) {
                    // it.value() contiene el array de valores
                    if (it.value().is_array()) {
                        vector<int> salidas_row;
                        for (const auto& valor : it.value()) {
                            if (valor.is_number()) {
                                salidas_row.push_back(valor.get<int>());
                            }
                        }
                        _salidas.push_back(salidas_row);
                    }
                }
            }
        }
        
       
        return true;

    } catch (const std::exception& e) {
        //qDebug() << "Error loading JSON: " << e.what();
        return false;
    }
}

bool CalibrationProgram::save(const string& path) {

    try {
        // Crear objeto JSON
        json j;
        
        string name = path + "/" + getName()+ ".json";

        // Guardar nombre del programa
        j["name"] = getName();

        // Guardar Fruta
        j["fruta"] = getFruta();
        //Guardar Variedad
        j["variedad"] = getVariedad();
        //Guardar color index de la fruta
        j["colorindexfruta"] = getColorIndexFruta();
        
        // Guardar calibres
        j["calibers"] = json::array();
        for (const auto& caliber : getCalibers()) {
            json caliberJson;
            caliberJson["name"] = caliber.getName();
            
            // Guardar dimensiones
            caliberJson["dimensions"] = json::array();
            for (const auto& dim : caliber.getDimensions()) {
                json dimJson;
                dimJson["type"] = dim.getIndex(); 
                dimJson["min"] = dim.getMinValue();
                dimJson["max"] = dim.getMaxValue();
                caliberJson["dimensions"].push_back(dimJson);
            }
            
            j["calibers"].push_back(caliberJson);
        }

        //Guardar nombres de las dimensiones
        j["dimensionNames"] = json::array();
        for (const auto& dimName : _dimensionNames) {
            j["dimensionNames"].push_back(dimName);
        }

        int size = _salidas.size();
        
        //Guardar la matriz de Salidas
        j["salidas"] = json::array();
        int cont=0;
        for (const auto& salidas : _salidas) {
            json salidasJson;
            //Nombre de la fila, sera el nombre del calibre
            string fila = getCalibers()[cont++].getName();
            salidasJson[fila] = salidas;
            j["salidas"].push_back(salidasJson);
        }

        // Escribir al archivo
        std::ofstream file(name);
        file << std::setw(4) << j << std::endl;
        return true;

    } catch (const std::exception& e) {
        //qDebug() << "Error saving JSON: " << e.what();
        return false;
    }
}

void CalibrationProgram::setSalida(int row, int col, int value) {
    cout << "Setting salida " <<  endl;
    if (row >= 0 && row < static_cast<int>(_salidas.size()) &&
        col >= 0 && col < static_cast<int>(_salidas[row].size())) {
        cout << "Setting salida " << row << " " << col << " to " << value << endl;
        _salidas[row][col] = value;
    }
}

void CalibrationProgram::addCaliber(const Caliber& caliber)
{ 
    _calibers.push_back(caliber);
}

int CalibrationProgram::getNumDimensions() const 
{
    int count = 0;
    for (const auto& dim : _dimensionNames) {
        if (!dim.empty()) {
            count++;
        }
    }
    return count;
}

std::vector<int> CalibrationProgram::sortCalibers() {

    // Crear vector de índices
    std::vector<int> indices(_calibers.size());
    for(size_t i = 0; i < indices.size(); ++i) {
        indices[i] = i;
    }

    // Ordenar los índices basados en los calibres
    std::sort(indices.begin(), indices.end(),
        [this](int a, int b) {
            return compareCalibersByDimensions(_calibers[a], _calibers[b]);
        });

    // Crear copias temporales
    auto calibers_copy = _calibers;
    auto salidas_copy = _salidas;

    // Reordenar los calibres y salidas según los índices
    for(size_t i = 0; i < indices.size(); ++i) {
        _calibers[i] = calibers_copy[indices[i]];
        if(i < _salidas.size()) {
            _salidas[i] = salidas_copy[indices[i]];
        }
    }

    return indices;
}

void CalibrationProgram::printCalibers() const {
    std::cout << "\n=== Calibres del Programa: " << _name << " ===\n";
    std::cout << "Dimensiones: ";
    for (const auto& dim : _dimensionNames) {
        if (!dim.empty()) {
            std::cout << dim << " ";
        }
    }
    std::cout << "\n\n";

    for (size_t i = 0; i < _calibers.size(); ++i) {
        const auto& caliber = _calibers[i];
        std::cout << "Calibre " << i + 1 << ": " << caliber.getName() << "\n";
        
        int cont=0;
        for (const auto& dim : caliber.getDimensions()) {
            std::cout << "  " << getDimension(cont++)
                     << ": [" << dim.getMinValue() 
                     << ", " << dim.getMaxValue() << "]\n";
        }
        std::cout << "\n";
    }
    std::cout << "================================\n";
}

//PRIVATE

bool CalibrationProgram::compareCalibersByDimensions(const Caliber& a, const Caliber& b) {
    const auto& dimA = a.getDimensions();
    const auto& dimB = b.getDimensions();
    
    // Comparar cada dimensión
    for(size_t i = 0; i < std::min(dimA.size(), dimB.size()); i++) {
        // Si los valores mínimos son diferentes
        if(dimA[i].getMinValue() != dimB[i].getMinValue()) {
            return dimA[i].getMinValue() < dimB[i].getMinValue();
        }
        
        // Si los valores mínimos son iguales, comparar los máximos
        if(dimA[i].getMaxValue() != dimB[i].getMaxValue()) {
            return dimA[i].getMaxValue() < dimB[i].getMaxValue();
        }
    }
    
    // Si todas las dimensiones son iguales, comparar por nombre
    return a.getName() < b.getName();
}

const std::vector<int>& CalibrationProgram::getSalidasCalibre(int caliber) const {
    static const std::vector<int> empty;
    if (caliber < 0 || caliber >= static_cast<int>(_salidas.size())) {
        return empty;
    }
    return _salidas[caliber];
}
