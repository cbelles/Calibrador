
#include "Cameras/OutputManager.h"

OutputManager::OutputManager(CalibrationProgram* program)
    : _program(program)
{
    if (_program) {
        reset();
    }
}

void OutputManager::setCalibrationProgram(CalibrationProgram* program) {
    _program = program;
    reset();
}

void OutputManager::reset() {
    _outputStates.clear();
    
    if (!_program) return;

    // Inicializar estados para cada calibre
    for (size_t i = 0; i < _program->getNumCalibers(); ++i) {
        const auto& caliber = _program->getCalibers()[i];
        OutputState state;
        state.currentOutputIndex = 0;
        state.currentRepetitionCount = 0;

        // Obtener las salidas configuradas para este calibre
        const auto& salidas = _program->getSalidas()[i];
        for (size_t j = 0; j < salidas.size(); ++j) {
            if (salidas[j] > 0) {
                state.outputs.push_back({j, salidas[j]});
            }
        }

        _outputStates[i] = state;
    }
}

std::vector<int> OutputManager::processCalibres(const std::vector<int>& calibres) {
    std::vector<int> outputs(calibres.size(), -1);
    
    for (size_t i = 0; i < calibres.size(); ++i) {
        if (calibres[i] >= 0) {  // -1 indica que no hay pieza
            outputs[i] = getNextOutput(calibres[i]);
        }
    }
    
    return outputs;
}

int OutputManager::getNextOutput(int calibre) {
    if (!_program || calibre < 0 || calibre >= _program->getNumCalibers()) {
        return -1;
    }

    auto& state = _outputStates[calibre];
    if (state.outputs.empty()) {
        return -1;
    }

    auto& currentOutput = state.outputs[state.currentOutputIndex];
    int output = currentOutput.first;
    
    // Incrementar el contador de repeticiones
    state.currentRepetitionCount++;
    
    // Si alcanzamos el número necesario de repeticiones
    if (state.currentRepetitionCount >= currentOutput.second) {
        // Pasar a la siguiente salida
        state.currentOutputIndex = (state.currentOutputIndex + 1) % state.outputs.size();
        state.currentRepetitionCount = 0;
    }

    return output;
}