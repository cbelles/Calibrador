
#include "ExpulsionManager.h"
#include <iostream>
#include <thread>
#include <chrono>

void printActiveOutputs(const std::vector<int>& outputs) {
    std::cout << "Salidas activas: ";
    if (outputs.empty()) {
        std::cout << "ninguna";
    } else {
        for (int output : outputs) {
            std::cout << output << " ";
        }
    }
    std::cout << std::endl;
}

void simulateConveyor(ExpulsionManager& manager, int total_ticks) {
    for (int i = 0; i < total_ticks; i++) {
        std::cout << "\nTick " << i << ":" << std::endl;
        
        // Simular detección de fruta cada 5 ticks en salidas alternadas
        if (i % 5 == 0) {
            int output = (i / 5) % 3;  // Alterna entre salidas 0, 1 y 2
            std::cout << "Detectada fruta para salida " << output << std::endl;
            manager.scheduleExpulsion(output);
        }

        // Obtener y mostrar salidas activas
        auto active = manager.getActiveOutputs();
        printActiveOutputs(active);

        // Avanzar el tiempo
        manager.tick();
        
        // Pequeña pausa para mejor visualización
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main() {
    ExpulsionManager manager;

/*
    // Configurar 3 salidas con diferentes distancias
    manager.addOutput(0, 10);  // Salida 0 a 10 ticks
    manager.addOutput(1, 15);  // Salida 1 a 15 ticks
    manager.addOutput(2, 20);  // Salida 2 a 20 ticks
*/
    string filename = "/home/pi/CalibradorParams/Config/positions.json";
    manager.loadPositionsFromFile(filename);

    std::cout << "Iniciando simulación..." << std::endl;
    std::cout << "- 3 salidas configuradas (0, 1, 2)" << std::endl;
    std::cout << "- Se detectará fruta cada 5 ticks" << std::endl;
    std::cout << "- Las salidas se activarán según su distancia configurada" << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    // Simular 30 ticks de funcionamiento
    simulateConveyor(manager, 40);

    return 0;
}