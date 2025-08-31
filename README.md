
# Calibrador: Software de control de un calibrador de vision de frutas

![](./doc/Esquema.png)
 
![](./doc/Pantalla.png)


## Ficheros de Configuracion   

### config.json
```bash
{
  "logo": {
    "showLogo": false,
    "dimX": 400,
    "dimY":200
  },
  
  "general": {
    "debugMode": true,
    "pathPrograms": "/home/pi/CalibradorParams/Programs",
    "pathConfig": "/home/pi/CalibradorParams/Config",
    "pathPartidas": "/home/pi/CalibradorParams/Partidas"
  },

  "config": {
    "numLineas": 2, 	
    "numSalidas": 16,
    "idioma": "PT"  
  },

  "dimensiones": {
    "nombre": ["DIAMETER_MAX", "DIAMETER_MIN", "COLOR", "QUALITY"], 
    "domain_min_value": [0,0,0,0],
    "domain_max_value": [9999,9999,255,255]	
  }

}
```
### frutas.json
```bash
{
 

  	"MADARINA": {
  		"colorIndex": "NARANJA",
  		"VARIEDADES": ["TODAS", "CLEMENULES","MARISOL","ORONULES"]
    	},
    	
  	"TOMATE": {
  		"colorIndex": "HUE",
  		"VARIEDADES": ["TODAS", "RAF","PERA","MONTEROSA"]
  	}

  
}
```
### Config/config_pos_salidas.json   

```bash
{
    "positions": [
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        8,
        9,
        10,
        11,
        12,
        13,
        14,
        20
    ]
}
```  

## Configuracion de los puertos   

El puerto de la izquierda sera el puerto de las camaras   
El puerto de la derecha sera el puerto de las tarjetas expulsoras   

![](./doc/dos_puertos.png)


## Entrada del sensor  

![](./doc/conexionOptoacoplador.png)   

![](./doc/calibrador_sensor.png)   


![](./doc/GPIO.png)


## CamaraManager "Thread" (Clase principal) 
1.- CalibrationProgram* _calibrationProgram es el programa activo actual, se puede cambiar desde el GUI (MainWindow)   
2.- OutputBoardManager*  _outputBoardManager
3.- Fotocelula* _fotocelula, Thread que lee del GPIO de OrangePi5 Plus  

![](./doc/Calibrador.png)  

## CalibersTable & SalidasTable (Principales clases del GUI, visualizan un programa de calibracion)   

Estas dos clases son las dos principales del GUI, visualizan un objeto CalibrationProgram   

![](./doc/Salidas&CalibersTable.png) 

## CalibrationProgram (Clase para crear los objetos programa de calibracion)   
 
En CalibrationProgram se define lo que es un  programa de calibracion  

![](./doc/CalibrationProgram.png)      

## CaliberClassifier  (Clase principal)   

La clase CaliberClassifier la utiliza CamaraManager  para dados los parametros de una fruta, descubrir el calibre asociado.   


