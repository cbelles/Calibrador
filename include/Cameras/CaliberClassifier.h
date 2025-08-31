#pragma once

#include "CalibrationProgram.h"
#include "Cameras/Mensaje.h"

class CaliberClassifier {
public:
    CaliberClassifier() : _program(nullptr) {}
    
    // Establecer el programa de calibración
    void setCalibrationProgram(CalibrationProgram* program) { _program = program; }

    // Devuelve el índice del calibre que corresponde al mensaje (-1 si no corresponde a ninguno)
    int classify(const Mensaje& msg) const;

private:
    // Verifica si un valor está dentro del rango del calibre para una dimensión dada
    bool isInRange(const Caliber& caliber, size_t dimIndex, double value) const;
    
    // Obtiene el valor correspondiente del mensaje según la dimensión
    double getValueFromMessage(const Mensaje& msg, const std::string& dimension) const;

    CalibrationProgram* _program;
};