
#include "Cameras/Mensaje.h"
#include <iostream>

bool Mensaje::parse(const std::string& jsonStr) {
    try {
        // Eliminar el caracter '#' del final si existe
        std::string cleanJson = jsonStr;
        if (!cleanJson.empty() && cleanJson.back() == '#') {
            cleanJson.pop_back();
        }
        
        _data = nlohmann::json::parse(cleanJson);
        _valid = true;
        return true;
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        _valid = false;
        return false;
    }
}

template<typename T>
T Mensaje::getValueSafe(const std::string& field, const T& defaultValue) const {
    if (!_valid) return defaultValue;
    
    try {
        auto value = _data["datos"][field];
        if (value.is_array()) {
            return value[0].get<T>();
        }
        return value.get<T>();
    } catch (...) {
        return defaultValue;
    }
}

int Mensaje::getCarril() const { return getValueSafe<int>("Carril"); }
bool Mensaje::getPieza() const { return getValueSafe<bool>("Pieza"); }
double Mensaje::getArea() const { return getValueSafe<double>("area"); }
int Mensaje::getCalidad() const { return getValueSafe<int>("calidad"); }
int Mensaje::getColor() const { return getValueSafe<int>("color"); }
double Mensaje::getDiMayor() const { return getValueSafe<double>("di mayor"); }
double Mensaje::getDiMenor() const { return getValueSafe<double>("di menor"); }
std::string Mensaje::getIndexColor() const { return getValueSafe<std::string>("indexcolor"); }
int Mensaje::getTimeStampIn() const { return getValueSafe<int>("time stamp in"); }
int Mensaje::getTimeStampOut() const { return getValueSafe<int>("time stamp out"); }

void Mensaje::print() const {
    if (!_valid) {
        std::cout << "Invalid message" << std::endl;
        return;
    }
    
    std::cout << "Carril: " << getCarril() << std::endl
              << "Pieza: " << getPieza() << std::endl
              << "Area: " << getArea() << std::endl
              << "Calidad: " << getCalidad() << std::endl
              << "Color: " << getColor() << std::endl
              << "Di Mayor: " << getDiMayor() << std::endl
              << "Di Menor: " << getDiMenor() << std::endl
              << "Index Color: " << getIndexColor() << std::endl
              << "Time Stamp In: " << getTimeStampIn() << std::endl
              << "Time Stamp Out: " << getTimeStampOut() << std::endl;
}