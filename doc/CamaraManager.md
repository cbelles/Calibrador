# Explicación de la clase CameraManager

La clase `CameraManager` es un componente central en el sistema de calibración de frutas, responsable de coordinar la comunicación con múltiples cámaras y procesar los datos capturados para la clasificación de productos.

## Propósito principal

El `CameraManager` actúa como intermediario entre los dispositivos físicos (cámaras) y el sistema de clasificación, gestionando todo el ciclo de vida de la captura de imágenes, desde la detección de un producto hasta la determinación de su calibre y la activación de los mecanismos de salida.

## Funcionalidades clave

### 1. Gestión de conexiones con cámaras
- Establece y mantiene conexiones TCP con múltiples cámaras (en la red 169.254.1.X)
- Implementa mecanismos de reconexión automática con timeouts optimizados
- Monitorea el estado de conexión de cada cámara y lo reporta al sistema

### 2. Procesamiento de capturas
- Recibe señales desde la fotocélula cuando un producto es detectado
- Envía comandos "PULSO" a todas las cámaras para capturar imágenes
- Procesa las respuestas de forma asíncrona y en paralelo para minimizar latencia
- Extrae las dimensiones relevantes (color, área, diámetros) de cada captura

### 3. Clasificación de productos
- Utiliza la clase `CaliberClassifier` para determinar el calibre de cada producto
- Aplica las reglas definidas en el programa de calibración actual
- Actualiza el historial de calibres para cada línea de producción

### 4. Control de salidas
- Determina la salida apropiada para cada producto basándose en su calibre
- Programa la activación de los mecanismos de expulsión en el momento preciso
- Gestiona la cola de expulsiones y envía las señales al `OutputBoardManager`

### 5. Interacción con la interfaz de usuario
- Emite señales Qt (`updateCameraLEDsRequested`, `updateOutputLEDsRequested`, etc.)
- Actualiza los indicadores visuales de estado de cámaras y salidas
- Proporciona datos en tiempo real para widgets de dimensiones y contadores

### 6. Registro y almacenamiento
- Mantiene un historial completo de calibres durante una partida
- Permite exportar los resultados a formato JSON y PDF
- Guarda estadísticas detalladas para análisis posterior

## Modos de operación

La clase soporta dos modos principales de funcionamiento:

1. **Modo normal:** Depende de la fotocélula para detectar productos y activar las capturas. Ideal para la operación en línea de producción.
2. **Modo debug:** Genera capturas periódicas sin necesidad de detección física. Útil para pruebas y diagnósticos del sistema.

## Arquitectura técnica

- Implementa un diseño multihilo para procesar múltiples cámaras en paralelo
- Utiliza Boost.Asio para la comunicación de red asíncrona
- Emplea un patrón de señales/slots de Qt para la comunicación con la interfaz
- Implementa manejo de errores robusto con timeouts y reconexiones automáticas

---

Esta clase representa el núcleo del sistema de procesamiento, conectando la entrada de datos visuales con el sistema de clasificación y la activación de los mecanismos físicos de separación de productos.