// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QScrollBar>
#include <QProcess>

// Primero incluir OutputBoardManager ya que otros módulos dependen de él
#include "OutputBoards/OutputBoardManager.h"

// Luego incluir el resto de headers
#include "CalibrationProgram.h"
#include "CalibersTable.h"
#include "SalidasTable.h"
#include "Params/ParamsGeneral.h"
#include "Params/ParamsFruta.h"
#include "FruitColorIndexs/ColorIndexLabel.h"
#include "Widgets/LedStrip.h"
#include "Widgets/CounterWidget.h"
#include "Widgets/DimensionsWidget.h"
#include "Dialogs/OutputPositionsDialog.h"
#include "Dialogs/TestSalidasDialog.h"
#include "Cameras/CameraManager.h"
#include "Fotocelula.h"

// Definir la constante de versión
#define APP_VERSION "1.0.2"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void ini(ParamsGeneral* paramgeneral);  // Corregir typo en ParamsGenersal
    void loadProgram() {onLoadProgram();};
    void configCalibrador(int numsalidas, int numlineas);

    void setParamsFruta(ParamsFruta* paramsfruta) { _paramsfruta = paramsfruta; }
    LedStrip* getLedsCamara() { return _ledsCamara; }
    LedStrip* getLedsSalidas() { return _ledsSalidas; }
    CounterWidget* getCounterWidget() { return _counterWidget; }
    DimensionsWidget* getDimensionsWidget() { return _dimensionsWidget; }
    void setCameraManager(CameraManager* manager) { _cameraManager = manager; }

    void setPartidaActiva(bool partida_activa); 
    bool getPartidaActiva() { return _partida_activa; }  

private slots:
    void onAddCaliber();
    void onRemoveCaliber();

    void onNewProgram();
    void onDeleteProgram();
    void onCopyProgram();
    void onSaveProgram();
    void onLoadProgram();
    void onTerminalButtonClicked();
    void onShutdownButtonClicked();
    void onNomachineButtonClicked();
    void onPosSalidasButtonClicked();
    void onTestSalidasButtonClicked();
    void onAnydeskButtonClicked();
    void onSortCalibers();
    void onPartidaButtonClicked();  // Añadir esta línea antes del último slot

private slots:
    void handleCameraLEDUpdate(int index, int color);
    void handleOutputLEDUpdate(const std::vector<int>& outputs); // Modified
    void handleDimensionsUpdate(const vector<vector<int>>& dimensiones, const vector<bool>& hayfruta);
    void handleCounterUpdate(const std::vector<int>& calibres);

private:
    void setupUI();
    void createConnections();

    void load_listOfPrograms();

    void enableButtons(bool enable);

    bool checkInDomain();
    bool checkNameOfCalibers();
    std::string getAnyDeskID();

    void enable_bottons();
    void disable_bottons();
    string getFechaHora();


private:

    CalibersTable* _calibersTable;
    SalidasTable* _salidasTable;  
    CalibrationProgram _currentProgram;   
    QComboBox* _listOfPrograms; 
    string _pathPrograms;
    string _pathConfig;
    string _pathPartidas;
    vector<string> _programs; 
    vector<string> _dimensiones_name;

    QLabel* fruitLabel;
    QLabel* varietyLabel;

    ColorIndexLabel* colorIndexLabel;
     
    QPushButton* addButton;
    QPushButton* removeButton;
    QPushButton* saveButton;
    QPushButton* newButton;
    QPushButton* copyButton;
    QPushButton* deleteButton; 
    QPushButton* sortButton;

    // Configuración del Calibrador
    int _numsalidas;
    int _numlineas;

    //Params Fruta
    ParamsFruta* _paramsfruta;

    //Widgets
    LedStrip* _ledsCamara; 
    CounterWidget* _counterWidget;
    DimensionsWidget* _dimensionsWidget;
    LedStrip* _ledsSalidas;
    QLabel* _partidaLabel;  
    
    // Nuevos botones
    QPushButton* posSalidas;  
    QPushButton* appsButton;
    QPushButton* anydeskButton; 
    QPushButton* nomachineButton;
    QPushButton* terminalButton;
    QPushButton* shutdownButton;
    QPushButton* partidaButton;  

    QThread* cameraThread;
    CameraManager* _cameraManager = nullptr;  // Thread CameraManager

    bool _partida_activa = false;
    string _partida_name;
    bool _counter_enabled = false;  
    bool _dimensions_enabled = false;  
    bool _outputs_enabled = false;  

    QProcess* _nomachineProcess;
    Fotocelula* _fotocelula = nullptr;  // Thread Fotocelula
    OutputBoardManager* _outputBoardManager = nullptr;

    bool _debugMode;
    bool _showLogo;
    int _logo_dimX;
    int _logo_dimY;

    // Label para mostrar la versión
    QLabel* _versionLabel;
};

#endif // MAINWINDOW_H