
#pragma once
#include <string>
#include "Utils/json.hpp"

class Mensaje {
public:
    Mensaje() = default;
    bool parse(const std::string& jsonStr);

    // Getters para los diferentes campos
    int getCarril() const;
    bool getPieza() const;
    double getArea() const;
    int getCalidad() const;
    int getColor() const;
    double getDiMayor() const;
    double getDiMenor() const;
    std::string getIndexColor() const;
    int getTimeStampIn() const;
    int getTimeStampOut() const;

    // Para depuración
    void print() const;

private:
    nlohmann::json _data;
    bool _valid = false;

    // Helper para acceso seguro a campos
    template<typename T>
    T getValueSafe(const std::string& field, const T& defaultValue = T()) const;
};

