
#pragma once

#include <vector>
#include <map>
#include "CalibrationProgram.h"

class OutputManager {
public:
    explicit OutputManager(CalibrationProgram* program = nullptr);

    // Establecer el programa de calibración
    void setCalibrationProgram(CalibrationProgram* program);

    // Procesar los calibres y obtener las salidas a activar
    std::vector<int> processCalibres(const std::vector<int>& calibres);

    // Reiniciar los contadores de salidas
    void reset();

private:
    // Obtener la siguiente salida para un calibre específico
    int getNextOutput(int calibre);

private:
    CalibrationProgram* _program;

    // Estructura para mantener el estado de las salidas para cada calibre
    struct OutputState {
        std::vector<std::pair<int, int>> outputs;  // <salida, repeticiones_necesarias>
        size_t currentOutputIndex;                  // Índice de la salida actual
        int currentRepetitionCount;                // Contador de repeticiones actual
    };

    // Mapa de estados de salida para cada calibre
    std::map<int, OutputState> _outputStates;
};