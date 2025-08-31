#include "CalibrationDimension.h"

// Definición de las variables estáticas
vector<string> CalibrationDimension::_dimension_names; 
vector<pair<int, int>> CalibrationDimension::_dimension_domain_value; 

void CalibrationDimension::setDimension(int index, string& name) {
    _index = index;
    _name = name;
}

void CalibrationDimension::setDomain(pair<int,int>& domain) {
    _domain_min = domain.first;
    _domain_max = domain.second;
}


bool CalibrationDimension::setMinValue(int min_value) {
    if (min_value< _domain_min) return false;
    _min_value = min_value;
    return true;
}

bool CalibrationDimension::setMaxValue(int max_value) {
    if (max_value > _domain_max) return false;
    _max_value = max_value;
    return true;
}
