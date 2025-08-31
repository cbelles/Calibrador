#ifndef CALIBRATIONPROGRAM_H
#define CALIBRATIONPROGRAM_H

#include <vector>

#include <Caliber.h>

using namespace std;

class CalibrationProgram {

public:

    CalibrationProgram() {}
        
    void setName(const string& name) { _name = name; }

    int getNumDimensions() const;

    void setDimension(int index, const string& name) {
        if (index >= 0 && index < static_cast<int>(_dimensionNames.size())) {
            _dimensionNames[index] = name;
        }
    }

    void setDimensionDomain(int index, const pair<int,int>& domain_min_max) {
        if (index >= 0 && index < static_cast<int>(_dimensionNames.size())) {
            _dimensionDomains[index] = domain_min_max;
        }
    }

    string getDimension(int index) const {  // Add const here
        if (index >= 0 && index < static_cast<int>(_dimensionNames.size())) {
            return _dimensionNames[index];
        }
        return "";  // Add a default return value
    }
    
    void clear();  
    
    bool load(const string& name);

    bool save(const string& path);

    void setSalidas(std::vector<std::vector<int>>& salidas) {_salidas = salidas;}

    void setSalida(int row, int col, int value);  

    const std::vector<std::vector<int>>& getSalidas() const { return _salidas; }
    
    const std::vector<int>& getSalidasCalibre(int caliber) const;

    string getName() const { return _name; }

    void addNewCaliber(); 

    void addCaliber(const Caliber& caliber); 

    void removeCaliber(int index) {
        if (index >= 0 && index < static_cast<int>(_calibers.size())) {
            _calibers.erase(_calibers.begin() + index);
        }
    }

    Caliber getCaliber(int index) {
        if (index >= 0 && index < static_cast<int>(_calibers.size())) {
            return _calibers[index];
        }
    }

    bool isValid() const {
        if (_calibers.empty()) return false;
        
        // Verificar que todos los calibres sean válidos
        for (const auto& caliber : _calibers) {
            if (!caliber.isValid()) return false;
        }

        // TODO: Verificar que el programa cubra todo el espacio
        return true;
    }

    std::vector<Caliber>& getCalibers() { return _calibers; }
    const std::vector<Caliber>& getCalibers() const { return _calibers; }
    int getNumCalibers() const { return _calibers.size(); }

    const string& getFruta() const { return _fruta; }
    const string& getVariedad() const { return _variedad; }
    const string& getColorIndexFruta() const { return _colorIndexFruta; }
    void setFruta(const string& fruta, const string& colorIndexFruta) { _fruta = fruta; _colorIndexFruta = colorIndexFruta; }
    void setVariedad(const string& variedad) { _variedad = variedad; }

    std::vector<int> sortCalibers();
    void printCalibers() const;

private:

    static bool compareCalibersByDimensions(const Caliber& a, const Caliber& b);


private:
    
    string _name;
    std::vector<Caliber> _calibers;  // Vector que contiene objetos de tipo Caliber
    std::vector<string> _dimensionNames;
    std::vector<pair<int,int>> _dimensionDomains;

    std::vector<std::vector<int>> _salidas;

    string _fruta;
    string _variedad;
    string _colorIndexFruta;

};
#endif // CALIBRATIONPROGRAM_H