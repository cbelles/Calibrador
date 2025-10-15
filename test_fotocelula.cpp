#include "include/Fotocelula.h"
#include <iostream>
#include <csignal>
#include <atomic>

using namespace std;

atomic<bool> running(true);

void signalHandler(int signum) {
    running = false;
    cout << "\nTest terminado por el usuario (Ctrl+C)" << endl;
}

int main(int argc, char* argv[]) {
    int pin = 0;
    if (argc > 1) {
        pin = atoi(argv[1]);
    }
    cout << "Test Fotocelula en pin GPIO (wPi): " << pin << endl;
    cout << "Pulsa Ctrl+C para salir..." << endl;

    signal(SIGINT, signalHandler);

    try {
        Fotocelula sensor(pin);
        sensor.start();
        while (running) {
            if (sensor.waitForDetection(1000)) {
                cout << "Señal detectada en el sensor!" << endl;
            }
        }
        sensor.stop();
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}
