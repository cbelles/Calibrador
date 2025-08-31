##  INDICE  
### 0. [ROS2 & MOVEIT](#ros2moveit) 
### 1. [DOBOT_6Axis-ROS2_V4: Guía de Configuración](#dobotconfig)
### 2. [Guía para Programar Movimientos con MoveIt 2 en ROS 2](/moveit-ros2-guide.md)

<a id="ros2moveit"></a>
# ROS2 & MOVEIT

Instalacion separada de ROS 2 y MoveIt2

Moveit2 source install
https://moveit.ai/install-moveit2/source/

>> ros2 launch moveit_resources_panda_moveit_config demo.launch.py   
TEST: No debe de haber ningun ERROR en el lanzamiento anterior!!!

### 1. MoveGroupInterface class

He intentado compilar moveit2 tutorial, pero habian problemas. He elegido estos ejemplos, que han compilado mas facil.   
https://github.com/ut-ims-robotics/pool-thesis-2023-moveit2-examples  
moveit2 tutorial usa MoveIt Task Constructor (MTC) que no se encontraba. Esto ejemplos parece que solo usan MoveGroupInterface, Empezaremos por MoveGroupInterface

Compilar:
```   
cd ~/ws_moveit2
colcon build --packages-select cpp_examples --symlink-install
source install/setup.bash
```

```
~/ws_moveit2/install/cpp_examples/
├── lib
│   └── cpp_examples  <-- AQUÍ ESTÁN TUS EJECUTABLES (pose_goal, joint_goal, etc.)
├── share
│   ├── ament_index
│   │   └── resource_index
│   │       └── packages
│   │           └── cpp_examples
│   ├── cpp_examples
│   │   └── cmake
│   │       ├── cpp_examplesConfig.cmake
│   │       └── cpp_examplesConfig-version.cmake
│   └── package.xml
```
>> source ~/ws_moveit2/install/setup.bash   
>> ros2 run cpp_examples pose_goal  

### 2. MoveIt Task Constructor (MTC)

**MoveIt Task Constructor (MTC)** es un framework dentro de MoveIt diseñado para **planificar y ejecutar tareas de manipulación complejas y multi-etapa**, como recoger y colocar (pick and place), ensamblaje, o inspección.

A diferencia de la `MoveGroupInterface`, que se enfoca en planificar movimientos hacia un único objetivo (una pose o un estado articular), MTC permite definir una tarea como una **secuencia (o árbol) de etapas interconectadas**.

https://github.com/ut-ims-robotics/pool-thesis-2023-moveit2-examples


<a id="dobotconfig"></a>
# DOBOT_6Axis-ROS2_V4: Guía de Configuración

https://github.com/Dobot-Arm/DOBOT_6Axis_ROS2_V4

## 1. Introducción al DOBOT_6Axis-ROS2_V4

Es un paquete de desarrollo oficial de Dobot para controlar sus brazos robóticos de 6 ejes usando ROS 2. Características clave:
* Protocolo de comunicación: TCP/IP
* Lenguajes soportados: C++ y Python
* Funcionalidad: Conexión, control de movimiento, y programación avanzada

## 2. Requisitos Previos

### Configuración de Red

| Tipo de Conexión | IP Controlador | IP Computadora |
|------------------|----------------|----------------|
| Cable Ethernet   | 192.168.5.1    | 192.168.5.XXX  |
| WiFi             | 192.168.1.6    | 192.168.1.XXX  |

### Requisitos de Sistema
* Ubuntu 22.04 o 24.04
* ROS 2 instalado
* Git instalado

## 3. Guía Paso a Paso

### 1. Crear Workspace ROS
```bash
mkdir -p ~/dobot_ws/src  # Crea estructura de directorios
cd ~/dobot_ws/src        # Entra al directorio de código fuente
```

### 2. Clonar Repositorio
```bash
git clone https://github.com/Dobot-Arm/DOBOT_6Axis_ROS2_V4.git
```

### 3. Compilar el Paquete
```bash
cd ~/dobot_ws       # Regresa al root del workspace
colcon build        # Inicia proceso de compilación
source install/local_setup.sh  # Activa el entorno
```

### 4. Configuración Permanente
```bash
# Agrega auto-carga al iniciar terminal
echo "source ~/dobot_ws/install/local_setup.sh" >> ~/.bashrc

# Configura IP del robot (ejemplo para conexión cableada)
echo "export IP_address=192.168.5.1" >> ~/.bashrc

# Selecciona modelo del robot (CR3 o CR5)
echo "export DOBOT_TYPE=cr3" >> ~/.bashrc  # Para CR3
# echo "export DOBOT_TYPE=cr5" >> ~/.bashrc  # Para CR5

# Aplica cambios inmediatamente
source ~/.bashrc
```

## 4. Estructura Clave del Paquete
```
DOBOT_6Axis_ROS2_V4/
├── config/          # Archivos de calibración
├── launch/          # Launch files ROS
├── src/
│   ├── dobot_driver # Código del driver TCP/IP
│   └── dobot_utils  # Herramientas auxiliares
└── test/            # Ejemplos de prueba
```

## 5. Comandos de Verificación
Después de la instalación:
```bash
# Verifica variables de entorno
echo $IP_address
echo $DOBOT_TYPE

```

# Demostración de Uso del DOBOT_6Axis-ROS2_V4

## Simulación en RViz   
```bash   
#FUNCIONA APARECE EN RViz EL ROBOT!!! 
``` 
```bash
ros2 launch dobot_rviz dobot_rviz.launch.py
```

* **Qué hace**: Inicia RViz con la configuración visual del brazo Dobot
* **Interacción**:
   * Usa `joint_state_publisher_gui` para ajustar ángulos articulares manualmente
   * Los cambios se reflejan en tiempo real en RViz
* **Uso típico**: Verificación de cinemática y diseño de trayectorias visuales

```bash
ros2 run joint_state_publisher_gui joint_state_publisher_gui
```
```bash   
#APARECE UNA PEQUEÑA VENTANA CON LOS SLIDERS PARA CADA UNO DE LOS JOINTS 
``` 

```bash
ros2 launch dobot_moveit dobot_moveit.launch.py
```

```bash   
#LOS SLIDERS NO FUNCIONABAN SI SE LANZA: ros2 launch dobot_rviz dobot_rviz.launch.py, PERO SI FUNCIONAN
#SI SE LANZA ros2 launch dobot_moveit dobot_moveit.launch.py
    #Inicia todos los componentes necesarios para que MoveIt controle tu Dobot:

        #Servidor de planificación (move_group): El núcleo de MoveIt que maneja la planificación de movimientos.

        #Robot State Publisher: Publica las transformaciones TF del robot.

        #RViz: La interfaz visual para previsualizar y controlar el robot.

        #Controladores: Gestiona la comunicación con los motores reales del Dobot.

    #Carga parámetros clave en el servidor de parámetros de ROS 2:

        #robot_description: El modelo URDF del robot.

        #planning_pipelines: Configuración de los planificadores (OMPL, CHOMP, etc.).

        #kinematics.yaml: Parámetros para el solver cinemático.

    #Configura el entorno de planificación:

        #Carga el modelo 3D del robot.

        #Inicia los plugins de percepción (colisiones, octomap, etc.).

        #Conecta con los topics de joint_states y joint_trajectory_controller.
``` 

## Control con MoveIt
```bash
ros2 launch dobot_moveit dobot_moveit.launch.py
```

**Pasos de operación**:
1. Arrastra los marcadores interactivos de las articulaciones
2. Haz clic en **"Plan and Execute"**
3. Observa:
   * Plan de movimiento generado automáticamente
   * Simulación de ejecución en RViz

**Características clave**:
* Planificación de colisiones
* Generación de trayectorias óptimas
* Visualización de espacios de trabajo

## Simulación Física en Gazebo     
```bash   
#PARA LA SIMULACION FISICA CREO QUE USARE ISAAC SIM
``` 
```bash
ros2 launch dobot_gazebo dobot_gazebo.launch.py
```

**Componentes incluidos**:
* Modelo físico del Dobot CR3/CR5
* Entorno de simulación con gravedad
* Interacción con objetos virtuales

**Casos de uso**:
* Pruebas de agarre de objetos
* Simulación de procesos industriales
* Desarrollo de controladores PID

## Control del Robot Real   
```bash   
#PARA CONTROLAR EL ROBOT REAL HABRA QUE ESPERAR !!!
``` 
```bash
ros2 launch cr_robot_ros2 dobot_bringup_ros2.launch.py
```

**Flujo de trabajo**:
1. Establece conexión TCP/IP con el controlador
2. Habilita serviciós ROS 2 para:
   * `/dobot/move_joint` (Movimiento articular)
   * `/dobot/get_pose` (Lectura de posición)
   * `/dobot/set_io` (Control de E/S digitales)

**Verificación de servicios**:
```bash
ros2 service list
# Salida esperada:
# /dobot/emergency_stop
# /dobot/get_current_pose
# /dobot/move_cartesian
# ...etc
```
