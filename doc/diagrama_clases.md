classDiagram
    QMainWindow <|-- MainWindow
    MainWindow *-- CalibersTable
    MainWindow *-- SalidasTable
    MainWindow *-- CalibrationProgram
    MainWindow *-- LedStrip
    MainWindow *-- CounterWidget
    MainWindow *-- DimensionsWidget
    MainWindow *-- ColorIndexLabel
    MainWindow *-- CameraManager
    MainWindow *-- Fotocelula
    MainWindow *-- OutputBoardManager
    MainWindow --> ParamsGeneral
    MainWindow --> ParamsFruta
    MainWindow *-- QProcess
    
    class MainWindow {
        -CalibersTable* _calibersTable
        -SalidasTable* _salidasTable
        -CalibrationProgram _currentProgram
        -ParamsFruta* _paramsfruta
        -LedStrip* _ledsCamara
        -LedStrip* _ledsSalidas
        -CounterWidget* _counterWidget
        -DimensionsWidget* _dimensionsWidget
        -CameraManager* _cameraManager
        -Fotocelula* _fotocelula
        -OutputBoardManager* _outputBoardManager
        -bool _partida_activa
        +ini(ParamsGeneral* paramgeneral)
        +loadProgram()
        +configCalibrador(int numsalidas, int numlineas)
        +setPartidaActiva(bool partida_activa)
    }
    
    class CameraManager {
        -int _numlineas
        -Fotocelula* _fotocelula
        -OutputBoardManager* _outputBoardManager
        -CalibrationProgram* _calibrationProgram
        +setDebugMode(bool debugMode)
        +setPathPartidas(string path)
        +setFotocelula(Fotocelula* fotocelula)
        +setOutputBoardManager(OutputBoardManager* manager)
        +setCalibrationProgram(CalibrationProgram* program)
        +getCaliberHistory()
    }
    
    class CalibrationProgram {
        -string _name
        -string _fruta
        -string _variedad
        -vector~Caliber~ _calibers
        -vector~vector~int~~ _salidas
        +addCaliber(Caliber caliber)
        +removeCaliber(int index)
        +sortCalibers()
        +setDimension(int index, string name)
        +save(string path)
        +load(string name)
    }
    
    class CalibersTable {
        +setCalibrationProgram(CalibrationProgram* program)
        +addCaliber(Caliber caliber)
        +removeCaliber()
        +updateTable()
        +setPartidaActiva(bool active)
    }
    
    class SalidasTable {
        +setCalibrationProgram(CalibrationProgram* program)
        +setCameraManager(CameraManager* manager)
        +setNumSalidas(int num)
        +addRow()
        +removeRow(int row)
        +getInfo()
        +setPartidaActiva(bool active)
    }
    
    class OutputBoardManager {
        +start()
    }
    
    class Fotocelula {
        +start()
    }
