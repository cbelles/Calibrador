#ifndef CALIBER_H
#define CALIBER_H


#include <CalibrationDimension.h>

#include <string>
#include <vector>

using namespace std;

class Caliber {

public:
    Caliber(const string& name) { _name = name; }

    void setName(const string& name) { _name = name; }
    
    string getName() const { return _name; }

    void addDimension(const CalibrationDimension& dimension) {
        if (_dimensions.size() < 3) {
            _dimensions.push_back(dimension);
        }
    }

    bool isValid() const {
        return !_dimensions.empty() && _dimensions.size() <= 3;
    }

    vector<CalibrationDimension>& getDimensions() { return _dimensions; }
    
    const vector<CalibrationDimension>& getDimensions() const { return _dimensions; }

private:
    string _name;
    vector<CalibrationDimension> _dimensions;  // Vector que contiene objetos de tipo CalibrationDimension
};
#endif // CALIBER_H