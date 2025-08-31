# Explicación de la clase CaliberClassifier

La clase `CaliberClassifier` es un componente crítico dentro del sistema de clasificación de frutas, actuando como el "cerebro" algorítmico que determina a qué categoría (calibre) pertenece cada fruta analizada. A continuación, detallo sus funcionalidades principales:

## Propósito fundamental

`CaliberClassifier` implementa la lógica que permite asignar una fruta específica a uno de los calibres definidos en el programa de calibración, basándose en las mediciones obtenidas por las cámaras.

## Funcionamiento básico

1. **Configuración**: Recibe una referencia al programa de calibración actual (`CalibrationProgram`), que contiene la definición de todos los calibres posibles y sus rangos de dimensiones.

2. **Clasificación**: Al recibir un mensaje con mediciones de una fruta (objeto `Mensaje`), extrae las dimensiones relevantes (color, área, diámetros) y las compara con los rangos definidos en cada calibre.

3. **Determinación**: Aplica algoritmos de decisión para encontrar el calibre que mejor corresponde a las dimensiones detectadas, considerando:
   - Prioridad de dimensiones
   - Tolerancias configuradas
   - Reglas de exclusión

4. **Resultado**: Devuelve el índice del calibre asignado (un valor entero), o un valor especial (-1) si la fruta no corresponde a ningún calibre definido.

## Integración en el flujo de trabajo

```
Cámaras → Mensaje → CameraManager → CaliberClassifier → Calibre asignado → OutputManager → Activación de salidas
```

El clasificador recibe los datos procesados y preprocesados de las cámaras (después de la binarización, segmentación y extracción de características) y toma la decisión final de clasificación.

## Características principales

- **Eficiencia**: Diseñado para operar en tiempo real, con algoritmos optimizados para minimizar latencia.
- **Adaptabilidad**: Trabaja con diferentes configuraciones de dimensiones (1, 2 o 3 dimensiones).
- **Manejo de casos especiales**: Incluye lógica para gestionar frutas con dimensiones fuera de los rangos establecidos.
- **Análisis multidimensional**: Evalúa simultáneamente múltiples características (color, tamaño, forma) para tomar decisiones precisas.

## Aspectos técnicos

- Implementa diferentes estrategias para la resolución de ambigüedades cuando una fruta podría pertenecer a más de un calibre.
- Utiliza reglas de contenimiento para verificar si un punto multidimensional (las medidas de la fruta) está dentro del espacio definido por cada calibre.
- Mantiene un registro de las clasificaciones realizadas para análisis estadístico posterior.

El `CaliberClassifier` es, en resumen, el componente de toma de decisiones que materializa las reglas de clasificación configuradas por el usuario en el proceso de asignación de calibres durante la operación del sistema.
