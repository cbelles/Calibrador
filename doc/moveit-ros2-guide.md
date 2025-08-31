##  INDICE   
### 1. [Introducción](#Introducción )   
### 2. [¿Qué MoveIt se está ejecutando?](#QueMoveit)
### 3. [Configuración de Entorno Virtual Python](#Virtual)
### 4. [Programación de DOBOT con Python](#Python)

# Guía para Programar Movimientos con MoveIt 2 en ROS 2

## 1. Introducción 

Cuando ejecutas `ros2 launch dobot_moveit dobot_moveit.launch.py`, se abren tres componentes clave:
1. **RViz** (Visualización del robot y entorno)
2. **MoveIt Motion Planning Plugin** (Ventana de planificación)
3. **MoveGroup Node** (Backend para cálculos cinemáticos)

## 1.1. Partes de la Ventana de MoveIt

| Sección | Función |
|---------|---------|
| **Displays** (Panel izquierdo) | Configura qué elementos visualizar (ej: colisiones) |
| **Motion Planning** (Panel derecho) | Interfaz principal para planificación |
| **Planning Library** | Algoritmos disponibles (OMPL, CHOMP, etc.) |
| **Planning Group** | Selección del grupo articular (ej: `arm_group`) |
| **Start State** | Estado inicial del robot |
| **Goal State** | Configuración del movimiento deseado |

## 1.2. Movimientos Básicos Paso a Paso

### A. Movimiento Articular (Joint Space)
1. **Selecciona** la pestaña *"Planning"* → *"Joint Goals"*
2. **Arrastra los sliders** para cada articulación a la posición deseada
3. **Planifica**:
   * Click en *"Plan"* (solo calcula la trayectoria)
   * Click en *"Plan & Execute"* (calcula y ejecuta en simulación)

### B. Movimiento Cartesiano (Pose Goal)
1. **Cambia** a la pestaña *"Pose Goal"*
2. **Haz click** en el enlace *"Select Goal State"*
3. **Arrastra** el marcador interactivo (flechas RGB) en RViz
4. **Plan & Execute**


<a id="QueMoveit"></a>
# 2. ¿Qué MoveIt se está ejecutando?

## 2.1. Jerarquía de Paquetes en ROS 2

ROS 2 sigue un orden de prioridad específico para encontrar paquetes, determinado por el `ROS_PACKAGE_PATH` (que se configura al hacer `source` de los workspaces). El orden es:

1. **Workspace actual (dobot_ws)** → Si el paquete existe aquí, se usa esta versión.
2. **Otros workspaces (moveit_ws)** → Solo si no se encontró en el workspace principal.
3. **Instalación global de ROS 2 (/opt/ros/<distro>)** → Último recurso.

## 2.2 ¿Qué MoveIt se está ejecutando?

Cuando lanzas:

```bash
ros2 launch dobot_moveit dobot_moveit.launch.py
```

**ROS 2 hace lo siguiente**:

1. Busca el paquete `dobot_moveit` en el workspace **dobot_ws** (porque hiciste `source` de `dobot_ws/install/setup.bash`).
2. Si el paquete `dobot_moveit` depende de `moveit_ros_planning_interface`, ROS 2 buscará esa dependencia en:
   * Primero en **dobot_ws** (si existe una versión modificada localmente).
   * Si no, en **moveit_ws** (si lo has configurado).
   * Finalmente, en la instalación global de ROS 2 (`/opt/ros/<distro>`).

## 2.3. Verifica qué MoveIt se está usando

### Opción A: Usando `ros2 pkg prefix`

Ejecuta esto en una terminal:

```bash
ros2 pkg prefix moveit_ros_planning_interface
```

* Si la ruta es `/opt/ros/<distro>`, estás usando la versión **global de ROS 2**.
* Si la ruta apunta a `dobot_ws/install` o `moveit_ws/install`, estás usando la versión **local de tu workspace**.

### Opción B: Usando `ldd` (solo Linux)

Si el paquete ya está corriendo:

```bash
ldd /opt/ros/<distro>/lib/moveit_ros_planning_interface/*.so
```

(O reemplaza con la ruta de tu ejecutable). Esto muestra las bibliotecas vinculadas.


<a id="Virtual"></a>
# Configuración de Entorno Virtual Python

## 1. Crear y activar el entorno virtual

```bash
# Instalar virtualenv (si no lo tienes)
sudo apt install python3.12-venv

# Crear el entorno (ej. en ~/dobot_venv)
python3 -m venv ~/dobot_venv

# Activar el entorno
source ~/dobot_venv/bin/activate
```

**Nota**: Usa `python3.10` si tu ROS 2 (Humble/Iron) requiere esa versión específica:
```bash
python3.10 -m venv ~/dobot_venv
```

## 2. Configurar el entorno para ROS 2 y MoveIt

### A. Instalar solo dependencias Python no-ROS

Dentro del entorno virtual, instala librerías complementarias (ej. para procesamiento de datos):
```bash
pip install numpy opencv-python matplotlib
```

### B. Enlazar ROS 2 al entorno virtual

**¡No instales** `rclpy` o `moveit` con pip! En su lugar, haz que el entorno virtual "vea" los paquetes ROS 2 del sistema:

```bash
# Opción 1: Crear un archivo .pth para apuntar a los paquetes de ROS
echo "/opt/ros/<distro>/lib/python3.10/site-packages" > ~/dobot_venv/lib/python3.10/site-packages/ros.pth
```

Reemplaza `<distro>` (ej. `humble`, `iron`) y verifica la ruta exacta con:
```bash
python3 -c "import rclpy; print(rclpy.__file__)"
```

Es correcto. No puedes instalar el paquete `ros-jazzy-moveit-py` directamente dentro de tu entorno virtual `dobot_venv`. Esto se debe a que los paquetes ROS 2 se instalan a nivel del sistema mediante el gestor de paquetes APT, no mediante pip dentro del entorno virtual.

Lo que necesitas hacer es:

1. **Instalar el paquete a nivel del sistema** (fuera del entorno virtual):

   ```bash
   sudo apt install ros-jazzy-moveit-py    #instalara  moveit 2.12.2
   ```

2. **Hacer que tu entorno virtual "vea" este paquete** instalado en el sistema:
   
   Como ya configuraste anteriormente con el archivo `.pth`, tu entorno virtual debería poder acceder a los paquetes de ROS 2 instalados en el sistema, siempre que el archivo `.pth` esté correctamente configurado.

   Verifica que el archivo `.pth` esté apuntando a la ubicación correcta:
   ```bash
   cat ~/dobot_venv/lib/python3*/site-packages/ros.pth
   ```

Esta es precisamente la razón por la cual configuramos un archivo `.pth` en tu entorno virtual, para que pueda "ver" los paquetes ROS 2 instalados a nivel del sistema sin tener que instalarlos dentro del entorno virtual.

Después de instalar el paquete a nivel del sistema, activa tu entorno virtual y deberías poder importar `moveit_py` o los módulos relevantes de MoveIt 2.


<a id="Python"></a>
## 3. Programación de DOBOT con Python (Visual Studio Code Extension for ROS)

https://moveit.picknik.ai/main/index.html  (MoveIt 2 Documentation)


```python
#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from moveit_msgs.msg import DisplayTrajectory
from moveit.planning import MoveIt2, PlanningComponents

class DobotControl(Node):
    def __init__(self):
        super().__init__('dobot_control')
        
        # Inicializa MoveIt 2 para tu robot
        self.moveit2 = MoveIt2(
            node=self,
            joint_names=['joint1', 'joint2', 'joint3', 'joint4'],  # Reemplaza con los nombres de tus joints
            base_link_name='base_link',
            end_effector_name='tool_link',  # Reemplaza con tu efector final
            group_name='dobot_arm'  # Nombre del grupo en tu MoveIt config
        )
        
        # Mueve el robot a una pose objetivo
        self.move_to_target_pose()

    def move_to_target_pose(self):
        # Define una pose objetivo (ajusta los valores según tu robot)
        target_pose = {
            'position': {'x': 0.2, 'y': 0.0, 'z': 0.3},
            'orientation': {'x': 0.0, 'y': 0.0, 'z': 0.0, 'w': 1.0}
        }
        
        # Planifica y ejecuta
        self.moveit2.move_to_pose(target_pose)
        self.get_logger().info("¡Movimiento completado!")

def main(args=None):
    rclpy.init(args=args)
    node = DobotControl()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
```
