#include <wiringPi.h>
#include <stdio.h>

int main(void) {
    // Inicializar wiringPi
    if (wiringPiSetup() == -1) {  // Usar modo WiringPi
        printf("Error al inicializar WiringPi\n");
        return 1;
    }

    printf("WiringPi inicializado correctamente\n");
    
    // Usar wPi 0 (pin físico 3, GPIO16)
    const int PIN = 0;  // wPi 0 corresponde al pin físico 3
    
    pinMode(PIN, INPUT);
    pullUpDnControl(PIN, PUD_UP);

    // Leer el valor del pin
    while(1) {
        int valor = digitalRead(PIN);
        printf("El valor del pin físico 3 (wPi 0) es: %d\n", valor);
        delay(1000); // Esperar 1 segundo entre lecturas
    }

    return 0;
}
