#include "mainwindow.h"
#include "mainwindow.moc"

#include <cstdio>
#include <memory>
#include <string>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <QInputDialog>
#include <QTimer>  // Añadir esta línea al principio con los otros includes
#include <QThread>


using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    //this->setWindowTitle(QString("Calibrador Válvulas Proporcionales - ") + APP_VERSION);
    _nomachineProcess = nullptr;
    _partida_activa = false;
    _partida_name = "";
}

MainWindow::~MainWindow()
{
    if (_nomachineProcess) {
        _nomachineProcess->close();
        delete _nomachineProcess;
    }
}

void MainWindow::setPartidaActiva(bool partida_activa)
{
    _partida_activa = partida_activa;
    _counter_enabled = partida_activa;
    _dimensions_enabled = partida_activa;
    _outputs_enabled = partida_activa;
    
    // Notify camera manager about partida status
    if (_cameraManager) {
        _cameraManager->setPartidaActiva(partida_activa);
    }
    
    if (_partida_activa)
    {
        partidaButton->setIcon(QIcon(":/images/partida_activa.png"));
        _partidaLabel->setText(QString::fromStdString(_partida_name));  // Mostrar nombre de la partida
        disable_bottons();
        _listOfPrograms->setEnabled(false);
        _dimensionsWidget->setEnabled(true);
        _ledsSalidas->setEnabled(true);
        _counterWidget->reset();
        _cameraManager->getCaliberHistory()->setName(_partida_name);
        _cameraManager->getCaliberHistory()->setCalibrationProgram(&_currentProgram); 
        _cameraManager->getCaliberHistory()->clear(); 
        _cameraManager->getCaliberHistory()->setIni();
        _salidasTable->setPartidaActiva(true);
        _calibersTable->setPartidaActiva(true);  // Add this line
    }
    else
    {
        partidaButton->setIcon(QIcon(":/images/partida_noactiva.png"));
        _partidaLabel->clear();  // Limpiar el texto
        enable_bottons();
        _listOfPrograms->setEnabled(true);
        _counterWidget->reset();  
        _dimensionsWidget->reset();
        _ledsSalidas->setAllLedsColor(4);
        _dimensionsWidget->setEnabled(false);
        _ledsSalidas->setEnabled(false);
        _cameraManager->getCaliberHistory()->setEnd();
        _salidasTable->setPartidaActiva(false);
        _calibersTable->setPartidaActiva(false);  
    }
}

void MainWindow::ini(ParamsGeneral* paramsgeneral) {
    _pathPrograms = paramsgeneral->get_pathPrograms();
    _pathConfig = paramsgeneral->get_pathConfig();
    _pathPartidas = paramsgeneral->get_pathPartidas();
    _debugMode = paramsgeneral->get_debugMode();
    _showLogo = paramsgeneral->get_showLogo();
    _logo_dimX = paramsgeneral->get_logo_dimX();
    _logo_dimY = paramsgeneral->get_logo_dimY();
    int tiempoCiclo = paramsgeneral->get_debugMode_ciclo_ms();

    // Crear OutputBoardManager antes que CameraManager 
    int timeoutExpulsores = 100;
    _outputBoardManager = new OutputBoardManager(_numlineas, timeoutExpulsores);
    _outputBoardManager->start();

    // Crear Fotocelula antes que CameraManager
    _fotocelula = new Fotocelula();
    
    // Crear CameraManager antes de setupUI
    int timeoutRecepcion = 100; 
    cameraThread = new QThread(this);

    string possalidasfile = _pathConfig + "/config_pos_salidas.json";
    cout << "Asignando Pos Salidas File!: " << possalidasfile << endl; 
    

    _cameraManager = new CameraManager(_numlineas, possalidasfile, timeoutRecepcion);
    _cameraManager->setTiempoCiclo(tiempoCiclo);
    _cameraManager->setDebugMode(_debugMode);
    _cameraManager->setPathPartidas(_pathPartidas);
    _cameraManager->setFotocelula(_fotocelula);  
    _cameraManager->setOutputBoardManager(_outputBoardManager);  
    _cameraManager->moveToThread(cameraThread);
    

    // Iniciar Fotocelula
    _fotocelula->start();

    // Ahora setupUI tiene acceso a _cameraManager
    setupUI();  

    connect(cameraThread, &QThread::started, _cameraManager, &CameraManager::start);
    cameraThread->start();
    createConnections();

    setPartidaActiva(false);
}

void MainWindow::configCalibrador(int numsalidas, int numlineas)
{
    _numsalidas = numsalidas;
    _numlineas = numlineas;
}

//SLOTS

void MainWindow::onAddCaliber()
{
    CalibrationDimension dimension; 
    Caliber newcaliber("");

    int num_dimensions = _currentProgram.getNumDimensions();
    for (int i = 0; i < num_dimensions; i++) {
            string dimName = _currentProgram.getDimension(i);
            int index =CalibrationDimension::getIndexOfName(dimName);  
            if (index == -1) continue;
            dimension.setDimension(index, dimName);
            pair<int,int> domain = CalibrationDimension::_dimension_domain_value[index]; 
            dimension.setDomain(domain);
            dimension.setMinValue(domain.first);
            dimension.setMaxValue(domain.second);
            newcaliber.addDimension(dimension);
    }

    _currentProgram.addCaliber(newcaliber);
    _calibersTable->addCaliber(newcaliber); 
    _calibersTable->viewport()->update();
    _salidasTable->addRow();

    //_currentProgram actualizar _salidas
    std::vector<std::vector<int>> salidas = _salidasTable->getInfo();
    _currentProgram.setSalidas(salidas);
}

void MainWindow::onRemoveCaliber()
{
    int currentRow = _calibersTable->removeCaliber();
    if (currentRow >= 0) {
        _currentProgram.removeCaliber(currentRow);
        _salidasTable->removeRow(currentRow);
    }
}

void MainWindow::onSaveProgram()
{
    if (!checkInDomain()) 
        return;
    if (!checkNameOfCalibers()) 
        return;


    //Coger la informacion de _salidasTable, para poder grabarla en el programa
    std::vector<std::vector<int>> salidas = _salidasTable->getInfo();

    _currentProgram.setSalidas(salidas);   
    _currentProgram.save(_pathPrograms);

    QMessageBox::information(this, tr("Guardar Programa"), tr("Programa guardado correctamente"));
}

void MainWindow::onLoadProgram()
{
    string programName = _listOfPrograms->currentText().toStdString();
    string name = _pathPrograms + "/" + programName + ".json"; 
    _currentProgram.load(name);

    fruitLabel->setText(QString::fromStdString(_currentProgram.getFruta()));
    varietyLabel->setText(QString::fromStdString(_currentProgram.getVariedad()));  

    string colorindex = _paramsfruta->getColorIndex(_currentProgram.getFruta());
    colorIndexLabel->create(colorindex, 30);
    colorIndexLabel->paintBar();
    
    _calibersTable->setCalibrationProgram(&_currentProgram);
    _salidasTable->setCalibrationProgram(&_currentProgram); 
    _dimensionsWidget->setCalibrationProgram(&_currentProgram);

    if (_cameraManager) {
        cout << "CameraManager setCalibrationProgram" << endl;
        _cameraManager->setCalibrationProgram(&_currentProgram);
    }
}

void MainWindow::onNewProgram()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Nuevo Programa"));
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    // Program name input
    QHBoxLayout* nameLayout = new QHBoxLayout();
    QLabel* nameLabel = new QLabel(tr("Nombre:"), &dialog);
    QLineEdit* nameEdit = new QLineEdit(&dialog);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(nameEdit);
    layout->addLayout(nameLayout);

    // Fruit and Variety selection
    QGroupBox* fruitGroup = new QGroupBox(tr("Fruta y Variedad"), &dialog);
    QGridLayout* fruitLayout = new QGridLayout(fruitGroup);

    QLabel* fruitLabel = new QLabel(tr("Fruta:"), &dialog);
    QComboBox* fruitComboBox = new QComboBox(&dialog);
    QLabel* varietyLabel = new QLabel(tr("Variedad:"), &dialog);
    QComboBox* varietyComboBox = new QComboBox(&dialog);

    // Populate fruit combo
    vector<string> frutas = _paramsfruta->getFrutas();
    for(const auto& fruta : frutas) {
        fruitComboBox->addItem(QString::fromStdString(fruta));
    }

    // Populate variety combo with first fruit varieties
    if (!frutas.empty()) {
        vector<string> variedades = _paramsfruta->getVariedades(frutas[0]);
        for(const auto& var : variedades) {
            varietyComboBox->addItem(QString::fromStdString(var));
        }
    }

    // Connect fruit combo to update varieties
    connect(fruitComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [=](int index) {
            varietyComboBox->clear();
            vector<string> variedades = _paramsfruta->getVariedades(frutas[index]);
            for(const auto& var : variedades) {
                varietyComboBox->addItem(QString::fromStdString(var));
            }
        });

    fruitLayout->addWidget(fruitLabel, 0, 0);
    fruitLayout->addWidget(fruitComboBox, 0, 1);
    fruitLayout->addWidget(varietyLabel, 1, 0);
    fruitLayout->addWidget(varietyComboBox, 1, 1);
    layout->addWidget(fruitGroup);
    
    // Dimension selection
    QGroupBox* dimGroup = new QGroupBox(tr("Dimensiones"), &dialog);
    QGridLayout* dimLayout = new QGridLayout(dimGroup);
    
    QCheckBox* dimChecks[3];
    QComboBox* dimCombos[3];
    
    for(int i = 0; i < 3; i++) {
        dimChecks[i] = new QCheckBox(tr("Dimensión ") + QString::number(i+1), &dialog);
        dimCombos[i] = new QComboBox(&dialog);
        dimCombos[i]->setEnabled(false);
        
        // Add dimension types
        for(const auto& dim : CalibrationDimension::_dimension_names) {
            dimCombos[i]->addItem(QString::fromStdString(dim));
        }
        
        // Connect checkbox to enable/disable combo
        connect(dimChecks[i], &QCheckBox::toggled, dimCombos[i], &QComboBox::setEnabled);

        // Conectar para controlar la habilitación del siguiente checkbox
        if (i > 0) {
            dimChecks[i]->setEnabled(false);
            connect(dimChecks[i-1], &QCheckBox::toggled, dimChecks[i], &QCheckBox::setEnabled);
            
            // Cuando un checkbox se desmarca, desmarcar los siguientes
            connect(dimChecks[i-1], &QCheckBox::toggled, this, [=](bool checked) {
                if (!checked) {
                    for(int j = i; j < 3; j++) {
                        dimChecks[j]->setChecked(false);
                        dimChecks[j]->setEnabled(false);
                    }
                }
            });
        }
        
        dimLayout->addWidget(dimChecks[i], i, 0);
        dimLayout->addWidget(dimCombos[i], i, 1);
    }
    dimChecks[0]->setChecked(true);
    dimChecks[0]->setDisabled(true);
    
    layout->addWidget(dimGroup);
    
    // Dialog buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttonBox);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString name = nameEdit->text();
        if (!name.isEmpty()) {
            _listOfPrograms->blockSignals(true);  // Bloquear señales temporalmente (evita un segment fault)
            _listOfPrograms->addItem(name); 
            _listOfPrograms->setCurrentText(name);
            
            _currentProgram.clear();
            _currentProgram.setName(name.toStdString());
            _listOfPrograms->blockSignals(false);
            // Save selected fruit and variety
            string fruta = fruitComboBox->currentText().toStdString();
            string variedad = varietyComboBox->currentText().toStdString();
            //get the color index of the fruit
            string colorindex = _paramsfruta->getColorIndex(fruta);
            _currentProgram.setFruta(fruta,colorindex);
            _currentProgram.setVariedad(variedad);           
            this->fruitLabel->setText(QString::fromStdString(fruta));
            this->varietyLabel->setText(QString::fromStdString(variedad));            
            colorIndexLabel->create(colorindex, 30);
            colorIndexLabel->paintBar();

            // Set selected dimensions
            for(int i = 0; i < 3; i++) {
                if(dimChecks[i]->isChecked()) {
                    _currentProgram.setDimension(i, 
                        dimCombos[i]->currentText().toStdString());
                }
            }

            enableButtons(true);
            _calibersTable->setCalibrationProgram(&_currentProgram);
            _salidasTable->setCalibrationProgram(&_currentProgram);
            _calibersTable->updateTable();
            _dimensionsWidget->setCalibrationProgram(&_currentProgram);
            _currentProgram.save(_pathPrograms);
        }
    }
}

void MainWindow::onDeleteProgram()
{
    string programName = _listOfPrograms->currentText().toStdString();
    
    // Show confirmation dialog
    QMessageBox::StandardButton reply = QMessageBox::question(nullptr, 
        tr("Eliminar Programa"),
        tr("¿Está seguro que desea eliminar el programa '%1'?").arg(QString::fromStdString(programName)),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        string name = _pathPrograms + "/" + programName + ".json";
        
        // Remove program file
        if (std::filesystem::remove(name)) {
            // Remove program from combo box
            _listOfPrograms->removeItem(_listOfPrograms->currentIndex());
            
            // Clear program and update UI
            _currentProgram.clear();
            
            //Si hay mas programas en la lista, cargar el primero
            if (_listOfPrograms->count() > 0) {
                onLoadProgram();
            }
            else
            {
                _calibersTable->setCalibrationProgram(&_currentProgram);
                _salidasTable->setCalibrationProgram(&_currentProgram); // Añadir esta línea
                // Disable buttons when no programs exist
                enableButtons(false);
            }
        }
    }
}

void MainWindow::onCopyProgram()
{


    string programName = _listOfPrograms->currentText().toStdString();
    
    // Show dialog to input new program name
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Copiar Programa"),
        tr("Nombre del nuevo programa:"), QLineEdit::Normal, QString::fromStdString(programName), &ok);
    
    if (ok && !newName.isEmpty()) {
        // Bloquear señales temporalmente
        _listOfPrograms->blockSignals(true);
        
        // Add to combo box
        _listOfPrograms->addItem(newName);
        _listOfPrograms->setCurrentText(newName);
        
        // Restaurar señales
        _listOfPrograms->blockSignals(false);
        
        // Update current program name
        _currentProgram.setName(newName.toStdString());

        // Save the new program
        _currentProgram.save(_pathPrograms);
    }
}

void MainWindow::onSortCalibers()
{
    std::vector<int> newOrder = _currentProgram.sortCalibers();
    //_currentProgram.printCalibers();
    _calibersTable->updateTable();
    _salidasTable->updateTable(); // Cambiar updateRows por updateTable
}

//PRIVATE

void MainWindow::setupUI()
{
    // Mover la inicialización de _listOfPrograms al principio de setupUI
    _listOfPrograms = new QComboBox(this);

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Layout principal
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(10); // Espacio entre layouts
    
    // Panel izquierdo (tabla de calibres)
    QWidget* leftWidget = new QWidget(this);
    leftWidget->setFixedWidth(520); // Ancho fijo para el panel izquierdo
    QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    
    // Crear tabla de calibres especializada
    _calibersTable = new CalibersTable(this);
    leftLayout->addWidget(_calibersTable);
    
    // Botones debajo de la tabla
    QHBoxLayout* tableButtonsLayout = new QHBoxLayout();
    QSize buttonSize(32, 32);  // Tamaño del icono
    int buttonHeight = 40;     // Altura del botón
    
    addButton = new QPushButton(tr("Añadir"), this);
    addButton->setIcon(QIcon(":/images/mas.png"));
    addButton->setMinimumHeight(buttonHeight);
    QPalette addPal = addButton->palette();
    addPal.setColor(QPalette::Button, QColor(204, 229, 225));  // Azul pastel verdoso
    addButton->setAutoFillBackground(true);
    addButton->setPalette(addPal);
    addButton->setStyleSheet("background-color: #CCE5E1;"); // Alternativa usando CSS
    
    removeButton = new QPushButton(tr("Eliminar"), this);
    removeButton->setIcon(QIcon(":/images/menos.png"));
    removeButton->setMinimumHeight(buttonHeight);
    removeButton->setAutoFillBackground(true);
    removeButton->setPalette(addPal);
    removeButton->setStyleSheet("background-color: #CCE5E1;"); // Mismo color que Añadir
    
    tableButtonsLayout->addWidget(addButton);
    tableButtonsLayout->addWidget(removeButton);
    
    leftLayout->addLayout(tableButtonsLayout);
    
    // Panel central (tabla de salidas)
    QWidget* centerWidget = new QWidget(this);
    _salidasTable = new SalidasTable(this);
    _salidasTable->setNumSalidas(_numsalidas);
    _salidasTable->setCameraManager(_cameraManager);
    
    // Establecer el ancho fijo del centerWidget para que coincida con la tabla
    centerWidget->setFixedWidth(_salidasTable->width() + 20); // Añadir un margen de 20px para evitar que se corte el contenido
    
    QVBoxLayout* centerLayout = new QVBoxLayout(centerWidget);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->addWidget(_salidasTable);
    
    // Sincronizar scrollbars verticales
    connect(_calibersTable->verticalScrollBar(), &QScrollBar::valueChanged,
            _salidasTable->verticalScrollBar(), &QScrollBar::setValue);
    connect(_salidasTable->verticalScrollBar(), &QScrollBar::valueChanged,
            _calibersTable->verticalScrollBar(), &QScrollBar::setValue);
    
    // Añadir botones al layout central, después de _salidasTable
    QVBoxLayout* centralButtonsLayout = new QVBoxLayout();
    QHBoxLayout* centralButtonsRow1 = new QHBoxLayout();
    QHBoxLayout* centralButtonsRow2 = new QHBoxLayout();
    
    // Estilo común para los botones
    QString buttonStyle = "background-color: #CCE5E1;"; // Azul pastel verdoso
    QPalette buttonPal;
    buttonPal.setColor(QPalette::Button, QColor(204, 229, 225));
  
    appsButton = new QPushButton(this);
    appsButton->setIcon(QIcon(":/images/apps.png"));
    appsButton->setText(tr("Aplicaciones"));
    appsButton->setIconSize(QSize(32, 32));
    appsButton->setMinimumHeight(buttonHeight);
    appsButton->setAutoFillBackground(true);
    appsButton->setPalette(buttonPal);
    appsButton->setStyleSheet(buttonStyle);
    
    anydeskButton = new QPushButton(this);
    anydeskButton->setIcon(QIcon(":/images/anydesk.png"));
    anydeskButton->setText(tr("Anydesk"));
    anydeskButton->setIconSize(QSize(32, 32));
    anydeskButton->setMinimumHeight(buttonHeight);
    anydeskButton->setAutoFillBackground(true);
    anydeskButton->setPalette(buttonPal);
    anydeskButton->setStyleSheet(buttonStyle);
    
    nomachineButton = new QPushButton(this);
    nomachineButton->setIcon(QIcon(":/images/nomachine.png"));
    nomachineButton->setText(tr("Nomachine"));
    nomachineButton->setIconSize(QSize(32, 32));
    nomachineButton->setMinimumHeight(buttonHeight);
    nomachineButton->setAutoFillBackground(true);
    nomachineButton->setPalette(buttonPal);
    nomachineButton->setStyleSheet(buttonStyle);

    // Añadir el botón Partida después de nomachineButton
    partidaButton = new QPushButton(this);
    partidaButton->setIcon(QIcon(":/images/partida_noactiva.png"));  
    partidaButton->setText(tr("Partida"));
    partidaButton->setIconSize(QSize(32, 32));
    partidaButton->setMinimumHeight(buttonHeight);
    partidaButton->setAutoFillBackground(true);
    partidaButton->setPalette(buttonPal);
    partidaButton->setStyleSheet(buttonStyle);

    // Cambiar la creación del botón
    posSalidas = new QPushButton(this);
    posSalidas->setIcon(QIcon(":/images/config.png"));
    posSalidas->setText(tr("Pos. Salidas"));  
    posSalidas->setIconSize(QSize(32, 32));
    posSalidas->setMinimumHeight(buttonHeight);
    posSalidas->setAutoFillBackground(true);
    posSalidas->setPalette(buttonPal);
    posSalidas->setStyleSheet(buttonStyle);

    // Primera fila de botones
    //centralButtonsRow1->addWidget(configButton);
    centralButtonsRow1->addWidget(posSalidas);
    centralButtonsRow1->addWidget(appsButton);
    centralButtonsRow1->addWidget(nomachineButton);
    centralButtonsRow1->addWidget(partidaButton);
    centralButtonsRow1->addStretch();   
   
    terminalButton = new QPushButton(this);
    terminalButton->setIcon(QIcon(":/images/terminal.png"));
    terminalButton->setText(tr("Terminal"));
    terminalButton->setIconSize(QSize(32, 32));
    terminalButton->setMinimumHeight(buttonHeight);
    terminalButton->setAutoFillBackground(true);
    terminalButton->setPalette(buttonPal);
    terminalButton->setStyleSheet(buttonStyle);
    
    shutdownButton = new QPushButton(this);
    shutdownButton->setIcon(QIcon(":/images/shutdown.png"));
    shutdownButton->setText(tr("Apagar"));
    shutdownButton->setIconSize(QSize(32, 32));
    shutdownButton->setMinimumHeight(buttonHeight);
    shutdownButton->setAutoFillBackground(true);
    shutdownButton->setPalette(buttonPal);
    shutdownButton->setStyleSheet(buttonStyle);

    // Cambiar el orden de los widgets en el layout
    centralButtonsRow2->addWidget(anydeskButton);  // Mover AnyDesk antes de Terminal
    centralButtonsRow2->addWidget(terminalButton);
    centralButtonsRow2->addWidget(shutdownButton);
    centralButtonsRow2->addStretch(); // Añadir espacio flexible al final

    // Añadir las dos filas de botones al layout central
    centralButtonsLayout->addLayout(centralButtonsRow1);
    centralButtonsLayout->addLayout(centralButtonsRow2);
    centerLayout->addLayout(centralButtonsLayout);

    // Panel derecho (configuración)
    QVBoxLayout* rightLayout = new QVBoxLayout();
    
    // Crear un nuevo QGroupBox para el selector de programa
    QGroupBox* programGroup = new QGroupBox(tr("SELECCIONAR PROGRAMA"), this);
    
    // Configurar la fuente del título y estilo
    QFont titleFont;
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    programGroup->setFont(titleFont);
    
    // Configurar el color de fondo del grupo
    QPalette programPal = programGroup->palette();
    programPal.setColor(QPalette::Window, QColor(230, 240, 255));  // Azul muy claro
    programGroup->setAutoFillBackground(true);
    programGroup->setPalette(programPal);

    // Crear layout para el grupo y añadir el combo box
    QVBoxLayout* programLayout = new QVBoxLayout(programGroup);
    programLayout->addWidget(_listOfPrograms);

    // Crear un nuevo QGroupBox para los detalles del programa
    QGroupBox* detailsGroup = new QGroupBox(tr(""), this);
    detailsGroup->setFont(titleFont);
    QVBoxLayout* detailsLayout = new QVBoxLayout(detailsGroup);

    // Fruta y Variedad
    QGridLayout* fruitLayout = new QGridLayout();
    fruitLabel = new QLabel(this);
    varietyLabel = new QLabel(this);
    fruitLayout->addWidget(new QLabel(tr("FRUTA:"), this), 0, 0);
    fruitLayout->addWidget(fruitLabel, 0, 1);
    fruitLayout->addWidget(new QLabel(tr("VARIEDAD:"), this), 1, 0);
    fruitLayout->addWidget(varietyLabel, 1, 1);

    // Color Fruit Index
    QGridLayout* colorfruitindexLayout = new QGridLayout();
    colorfruitindexLayout->addWidget(new QLabel(tr("INDICE COLOR:"), this), 0, 0);
    colorIndexLabel = new ColorIndexLabel(this);
    colorfruitindexLayout->addWidget(colorIndexLabel, 0, 1);

    // Añadir los layouts al grupo de detalles
    detailsLayout->addLayout(fruitLayout);
    detailsLayout->addLayout(colorfruitindexLayout);

    // Configurar el estilo del grupo de detalles
    QPalette detailsPal = detailsGroup->palette();
    detailsPal.setColor(QPalette::Window, QColor(230, 240, 255));  // Azul muy claro
    detailsGroup->setAutoFillBackground(true);
    detailsGroup->setPalette(detailsPal);

    // Añadir los grupos al layout derecho
    rightLayout->addWidget(programGroup);
    rightLayout->addWidget(detailsGroup);

    //Widget de los leds de la camara
    QGridLayout* ledscamaraLayout = new QGridLayout();
    ledscamaraLayout->addWidget(new QLabel(tr("CAMARAS:"), this), 0, 0);
    _ledsCamara = new LedStrip(this,_numlineas);
    _ledsCamara->setAllLedsColor(1);  //RED
    ledscamaraLayout->addWidget(_ledsCamara, 0, 1);

    //Widget contador velocidad
    QGridLayout* countervelocidadLayout = new QGridLayout();
    _counterWidget = new CounterWidget(this);
    _counterWidget->reset();
    countervelocidadLayout->addWidget(_counterWidget, 0, 0);

    // Crear un grupo para dimensiones y salidas (sin título)
    QGroupBox* dimensionsSalidasGroup = new QGroupBox(this);  // Eliminado el título
    dimensionsSalidasGroup->setFont(titleFont);
    QVBoxLayout* dimensionsSalidasLayout = new QVBoxLayout(dimensionsSalidasGroup);
    
    // Widget dimensiones
    QGridLayout* dimensionsLayout = new QGridLayout();
    _dimensionsWidget = new DimensionsWidget(this);
    QTimer::singleShot(0, this, [this]() {
        _dimensionsWidget->setNumLines(_numlineas);
    });
    dimensionsLayout->addWidget(_dimensionsWidget, 0, 0);

    // Widget de los leds de salidas
    QGridLayout* ledsSalidasLayout = new QGridLayout();
    _ledsSalidas = new LedStrip(this, _numsalidas);
    _ledsSalidas->setLedSize(10);
    _ledsSalidas->setAllLedsColor(4);  //WHITE
    
    // Opción 1: Usar addStretch en ambos lados del layout
    ledsSalidasLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 0);
    ledsSalidasLayout->addWidget(_ledsSalidas, 0, 1);
    ledsSalidasLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 2);

    // Añadir los layouts al grupo
    dimensionsSalidasLayout->addLayout(dimensionsLayout);
    dimensionsSalidasLayout->addLayout(ledsSalidasLayout);
    
    // Ajustar el alto del grupo para que se ajuste a su contenido
    dimensionsSalidasGroup->setMaximumHeight(dimensionsSalidasGroup->sizeHint().height());

    // Configurar el estilo del grupo
    QPalette groupPal = dimensionsSalidasGroup->palette();
    groupPal.setColor(QPalette::Window, QColor(230, 240, 255));  // Azul muy claro
    dimensionsSalidasGroup->setAutoFillBackground(true);
    dimensionsSalidasGroup->setPalette(groupPal);

    // Crear un layout separado para el label de partida
    QHBoxLayout* partidaLayout = new QHBoxLayout();
    QLabel* partidaTitleLabel = new QLabel(tr("PARTIDA:"), this);
    _partidaLabel = new QLabel(this);
    QFont partidaFont;
    partidaFont.setPointSize(12);
    partidaFont.setBold(true);
    partidaTitleLabel->setFont(partidaFont);
    _partidaLabel->setFont(partidaFont);
    _partidaLabel->setAlignment(Qt::AlignLeft);
    partidaLayout->addWidget(partidaTitleLabel);
    partidaLayout->addWidget(_partidaLabel);
    partidaLayout->addStretch();

    // Añadir todos los elementos al layout derecho  
    rightLayout->addLayout(ledscamaraLayout);
    rightLayout->addLayout(countervelocidadLayout);
    rightLayout->addWidget(dimensionsSalidasGroup);
    rightLayout->addLayout(partidaLayout);  // Añadir partidaLayout después del grupo
    rightLayout->addStretch();

    // Añadir el logo del cliente
    QLabel* logoLabel = new QLabel(this);
    QString logoPath = QString::fromStdString(_pathConfig + "/client_logo.png");
    if (_showLogo) {
        QPixmap logoPixmap(logoPath);
        logoLabel->setPixmap(logoPixmap.scaled(_logo_dimX, _logo_dimY, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    rightLayout->addWidget(logoLabel, 0, Qt::AlignCenter);
    
    //=========================================================
    // Crear y configurar el label de versión
    _versionLabel = new QLabel(QString("v") + APP_VERSION, this);
    _versionLabel->setStyleSheet(
        "font-size: 8pt; "
        "color: #404040; " // Gris oscuro para el texto
        "background-color: rgba(240, 240, 240, 150); " // Fondo semitransparente
        "padding: 2px 4px; "
        "border-radius: 3px;"
    );
    _versionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _versionLabel->adjustSize();
    _versionLabel->raise(); // Asegurar que esté por encima de otros widgets
    
    // Posicionar debajo del logo (en la parte inferior)
    QTimer::singleShot(100, this, [this]() {
        int x = this->width() - _versionLabel->width(); // Alineado a la derecha;
        int y = this->height() - 5 - _versionLabel->height(); // En la parte inferior de la ventana
        _versionLabel->move(x, y);
    });
    //=========================================================

    // Añadir layouts al layout principal
    mainLayout->addWidget(leftWidget);
    mainLayout->addWidget(centerWidget);
    mainLayout->addLayout(rightLayout, 1);

    // Barra de botones inferior
    QHBoxLayout* bottomButtonsLayout = new QHBoxLayout();
    
    // Usar el mismo estilo definido anteriormente para los botones inferiores
    saveButton = new QPushButton(this);
    saveButton->setIcon(QIcon(":/images/save.png"));
    saveButton->setText(tr("Guardar"));
    saveButton->setIconSize(buttonSize);
    saveButton->setMinimumHeight(buttonHeight);
    saveButton->setAutoFillBackground(true);
    saveButton->setPalette(buttonPal);
    saveButton->setStyleSheet(buttonStyle);
    
    newButton = new QPushButton(this);
    newButton->setIcon(QIcon(":/images/new.png"));
    newButton->setText(tr("Nuevo"));
    newButton->setIconSize(buttonSize);
    newButton->setMinimumHeight(buttonHeight);
    newButton->setAutoFillBackground(true);
    newButton->setPalette(buttonPal);
    newButton->setStyleSheet(buttonStyle);
    
    copyButton = new QPushButton(this);
    copyButton->setIcon(QIcon(":/images/copy.png"));
    copyButton->setText(tr("Copiar"));
    copyButton->setIconSize(buttonSize);
    copyButton->setMinimumHeight(buttonHeight);
    copyButton->setAutoFillBackground(true);
    copyButton->setPalette(buttonPal);
    copyButton->setStyleSheet(buttonStyle);

    // Añadir el nuevo botón "Ordenar"
    sortButton = new QPushButton(this);
    sortButton->setIcon(QIcon(":/images/ordenar.png"));
    sortButton->setText(tr("Ordenar"));
    sortButton->setIconSize(buttonSize);
    sortButton->setMinimumHeight(buttonHeight);
    sortButton->setAutoFillBackground(true);
    sortButton->setPalette(buttonPal);
    sortButton->setStyleSheet(buttonStyle);
    
    deleteButton = new QPushButton(this);
    deleteButton->setIcon(QIcon(":/images/delete.png"));
    deleteButton->setText(tr("Eliminar"));
    deleteButton->setIconSize(buttonSize);
    deleteButton->setMinimumHeight(buttonHeight);
    deleteButton->setAutoFillBackground(true);
    deleteButton->setPalette(buttonPal);
    deleteButton->setStyleSheet(buttonStyle);
    
    bottomButtonsLayout->addWidget(saveButton);
    bottomButtonsLayout->addWidget(newButton);
    bottomButtonsLayout->addWidget(copyButton);
    bottomButtonsLayout->addWidget(sortButton);  // Añadir el botón "Ordenar"
    bottomButtonsLayout->addWidget(deleteButton);
    
    leftLayout->addLayout(bottomButtonsLayout);

    //Load the programs in the QComboBox _listOfPrograms
    load_listOfPrograms();
}

void MainWindow::createConnections()
{
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddCaliber);
    connect(removeButton, &QPushButton::clicked, this, &MainWindow::onRemoveCaliber);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::onSaveProgram);
    connect(newButton, &QPushButton::clicked, this, &MainWindow::onNewProgram);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteProgram);
    connect(copyButton, &QPushButton::clicked, this, &MainWindow::onCopyProgram);
    connect(sortButton, &QPushButton::clicked, this, &MainWindow::onSortCalibers);
    connect(_listOfPrograms, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLoadProgram);
    // Conectar la señal de selección entre las tablas
    connect(_calibersTable, &CalibersTable::rowSelected,
            _salidasTable, &SalidasTable::highlightRow);
    connect(terminalButton, &QPushButton::clicked, this, &MainWindow::onTerminalButtonClicked);
    connect(shutdownButton, &QPushButton::clicked, this, &MainWindow::onShutdownButtonClicked);
    connect(nomachineButton, &QPushButton::clicked, this, &MainWindow::onNomachineButtonClicked);
    connect(appsButton, &QPushButton::clicked, this, &MainWindow::onAppsButtonClicked);
    connect(anydeskButton, &QPushButton::clicked, this, &MainWindow::onAnydeskButtonClicked);
    connect(partidaButton, &QPushButton::clicked, this, &MainWindow::onPartidaButtonClicked);  
    connect(posSalidas, &QPushButton::clicked, this, &MainWindow::onPosSalidasButtonClicked);

    // Conectar señales de CameraManager a slots
    connect(_cameraManager, &CameraManager::updateCameraLEDsRequested,
        this, &MainWindow::handleCameraLEDUpdate, Qt::QueuedConnection);
    connect(_cameraManager, &CameraManager::updateOutputLEDsRequested,
        this, &MainWindow::handleOutputLEDUpdate, Qt::QueuedConnection);
    connect(_cameraManager, &CameraManager::updateCounterRequested,
        this, &MainWindow::handleCounterUpdate, Qt::QueuedConnection);
    connect(_cameraManager, &CameraManager::updateDimensionsRequested,
        this, &MainWindow::handleDimensionsUpdate, Qt::QueuedConnection);
}

void MainWindow::onTerminalButtonClicked()
{
    // Ejecutar el terminal
    system("exo-open --launch TerminalEmulator");
}

void MainWindow::onShutdownButtonClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, 
        tr("Apagar sistema"),
        tr("¿Está seguro que desea apagar el sistema?"),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        system("/sbin/shutdown -h now");  // Usar ruta completa del comando
    }
}

void MainWindow::onNomachineButtonClicked()
{
    if (!_nomachineProcess) {
        _nomachineProcess = new QProcess(this);
        _nomachineProcess->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    }
    
    if (_nomachineProcess->state() == QProcess::NotRunning) {
        _nomachineProcess->start("/usr/NX/bin/nxplayer");
    }
}

void MainWindow::onPosSalidasButtonClicked()   
{
    // Crear vector con las posiciones por defecto
    std::vector<int> currentPositions(_numsalidas);

    bool ok;
    QString qpassword = QInputDialog::getText(this, tr("Autenticación Requerida"),
                                             tr("Contraseña:"), QLineEdit::Password,
                                             "", &ok);
    std::string password = qpassword.toStdString();
    string config_password = "insimtec";

    if (ok && !password.empty()) {

        if (password == config_password) {
            OutputPositionsDialog dialog(_numsalidas, currentPositions, QString::fromStdString(_pathConfig), this);
            if (dialog.exec() == QDialog::Accepted) {
                // Recargar las posiciones en el CameraManager en caliente
                if (_cameraManager) {
                    _cameraManager->reloadOutputPositions();
                    cout << "MainWindow: Posiciones de salida actualizadas en caliente" << endl;
                } else {
                    cout << "MainWindow: Advertencia - CameraManager no disponible para recarga" << endl;
                }
            }       
        } else {
            QMessageBox::warning(this, tr("Error de Autenticación"), tr("Contraseña incorrecta."));
        }
    } 
    else {
        // El usuario canceló el diálogo o no introdujo nada.
        // Puedes mostrar un mensaje o simplemente no hacer nada.
    }
 
}

void MainWindow::onAppsButtonClicked()
{
    // TODO: Implementar la apertura de aplicaciones
}

void MainWindow::onAnydeskButtonClicked()
{
    std::string anydeskId = getAnyDeskID();
    if (!anydeskId.empty()) {
        QMessageBox::information(this, tr("AnyDesk ID"), 
            tr("Su ID de AnyDesk es: %1").arg(QString::fromStdString(anydeskId)));
    }
}

void MainWindow::load_listOfPrograms()
{
    _programs.clear();
    _listOfPrograms->clear();
    
    // Iterate through directory
    for (const auto& entry : std::filesystem::directory_iterator(_pathPrograms)) {
        if (entry.path().extension() == ".json") {
            std::string filename = entry.path().filename().string();
            // Remove the .json extension
            filename = filename.substr(0, filename.length() - 5);
            _programs.push_back(filename);
        }
    }
    
    // Sort programs alphabetically
    std::sort(_programs.begin(), _programs.end());

    // Add sorted programs to the combo box
    for (const auto& program : _programs) {
        _listOfPrograms->addItem(QString::fromStdString(program));
    }   

    // Select first program if available
    if (_listOfPrograms->count() > 0) {
        _listOfPrograms->setCurrentIndex(0);
        // Load the selected program
        onLoadProgram();
        
        // Enable buttons when programs exist
        enableButtons(true);
    }
    else
    {
        // Disable buttons when no programs exist
        enableButtons(false);
    }
}

void MainWindow::enableButtons(bool enable)
{
    addButton->setEnabled(enable);
    removeButton->setEnabled(enable);
    saveButton->setEnabled(enable);
    copyButton->setEnabled(enable);
    deleteButton->setEnabled(enable);
}

bool MainWindow::checkInDomain() {

    // Recorremos cada celda que contiene un QSpinBox
    for (int row = 0; row < _calibersTable->rowCount(); row++) {
        for (int col = 1; col < _calibersTable->columnCount(); col++) {
            QSpinBox* spinBox = qobject_cast<QSpinBox*>(_calibersTable->cellWidget(row, col));
            if (spinBox) {
                // Verificar si el color del texto es rojo
                if (spinBox->palette().color(QPalette::Text) == Qt::red) {
                    string name = _currentProgram.getCalibers()[row].getName(); 
                    QString msg = QString("Error en calibre '%1':\nValor fuera de dominio en la columna %2")
                        .arg(QString::fromStdString(name))
                        .arg(col/2);
                    QMessageBox::critical(nullptr, "Error de Dominio", msg);
                    return false;
                }
            }
        }
    }
    return true;
}

bool MainWindow::checkNameOfCalibers() {

    // Recorremos cada celda que contiene un QSpinBox
    for (int row = 0; row < _calibersTable->rowCount(); row++) {
        for (int col = 0; col < 1; col++) {
            QTableWidgetItem* item = _calibersTable->item(row, col);
            // Verificar si el nombre está vacío
            if (item) {
                if (item->text().isEmpty()) {
                    string name = _currentProgram.getCalibers()[row].getName(); 
                    QString msg = QString("Error en calibre '%1':\nNombre vacio")
                        .arg(QString::fromStdString(name));
                    QMessageBox::critical(nullptr, "Error de Nombre", msg);
                    return false;
                }
            }
        }
    }
    //Verificar que no haya nombres repetidos
    vector<string> names;
    for (int row = 0; row < _calibersTable->rowCount(); row++) {
        QTableWidgetItem* item = _calibersTable->item(row, 0);
        if (item) {
            string name = item->text().toStdString();
            if (std::find(names.begin(), names.end(), name) != names.end()) {
                QString msg = QString("Error en calibre '%1':\nNombre repetido")
                    .arg(QString::fromStdString(name));
                QMessageBox::critical(nullptr, "Error de Nombre", msg);
                return false;
            }
            names.push_back(name);
        }
    }
    return true;
}

std::string MainWindow::getAnyDeskID() {
    std::string result;
    char buffer[128];
    
    // Ejecutar el comando y capturar la salida
    std::unique_ptr<FILE, decltype(&pclose)> pipe(
        popen("anydesk --get-id", "r"), pclose);
        
    if (!pipe) { 
        return "";
    }
    
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }
    
    // Limpiar caracteres de nueva línea
    if (!result.empty() && result[result.length()-1] == '\n') {
        result.erase(result.length()-1);
    }
    
    return result;
}

void MainWindow::handleCameraLEDUpdate(int index, int color) {
    if (_ledsCamara && index < _ledsCamara->getNumLeds()) {
        _ledsCamara->setLedColor(index, color);
    }
}

void MainWindow::handleOutputLEDUpdate(const std::vector<int>& outputs) {
    if (_ledsSalidas && _outputs_enabled) {
        int viewlinea = _dimensionsWidget->getCurrentLine();
        _ledsSalidas->setAllLedsColor(4); // WHITE
        if (viewlinea >= 0 && viewlinea < outputs.size()) {
            _ledsSalidas->setLedColor(outputs[viewlinea], 2); // GREEN
        }
    }
}

void MainWindow::handleCounterUpdate(const std::vector<int>& calibres) {
    _counterWidget->incNumPieces(calibres);
    if (_counterWidget && _counter_enabled) { 
        _counterWidget->updatePieces(); 
        _counterWidget->updateCupletsPerSecond();
        _counterWidget->updateFruitsPerSecond();
    }
    else
    {
        _counterWidget->updateCupletsPerSecond();
    }
}

void MainWindow::handleDimensionsUpdate(const vector<vector<int>>& dimensiones, const vector<bool>& hayfruta) {
    if (_dimensionsWidget && _dimensions_enabled) {  // Solo actualizar si está habilitado
        int viewline = _dimensionsWidget->getCurrentLine();
        
        // Actualizar el widget de dimensiones solo si hay fruta en la línea actual
        if ( (viewline >=0) && (viewline < _numlineas) && (hayfruta[viewline])) {
            _dimensionsWidget->updateDimensions(dimensiones[viewline]);   
        }       
    }
}

 void MainWindow::enable_bottons()
 {
    addButton->setEnabled(true);
    removeButton->setEnabled(true);
    saveButton->setEnabled(true);
    newButton->setEnabled(true);
    copyButton->setEnabled(true);
    sortButton->setEnabled(true);
    deleteButton->setEnabled(true);
    appsButton->setEnabled(true);
    anydeskButton->setEnabled(true);
    nomachineButton->setEnabled(true);
    partidaButton->setEnabled(true);
    posSalidas->setEnabled(true);
    terminalButton->setEnabled(true);
    shutdownButton->setEnabled(true);
 }

void MainWindow::disable_bottons()
{
    //Desabilitar botones 
    addButton->setEnabled(false);
    removeButton->setEnabled(false);
    saveButton->setEnabled(false);
    newButton->setEnabled(false);
    copyButton->setEnabled(false);
    sortButton->setEnabled(false);
    deleteButton->setEnabled(false);
    appsButton->setEnabled(true);
    anydeskButton->setEnabled(true);
    nomachineButton->setEnabled(true);
    partidaButton->setEnabled(true);
    posSalidas->setEnabled(false);
    terminalButton->setEnabled(true);
    shutdownButton->setEnabled(true);
    
    //Cambiar el icono del boton de partida
    //partidaButton->setIcon(QIcon(":/images/partida.png"));

}

void MainWindow::onPartidaButtonClicked()
{
    //TERMINAR LA PARTIDA ACTUAL
    if (_partida_activa) 
    {
        //Mostar un mensagebox para confirmar la desactivación de la partida
        QMessageBox::StandardButton reply = QMessageBox::question(this, 
            tr("Desactivar Partida"),
            tr("¿Está seguro que desea desactivar la partida?"),
            QMessageBox::Yes | QMessageBox::No);
        //Si la respuesta es afirmativa
        if (reply == QMessageBox::Yes) {
            //Desactivar la partida
            setPartidaActiva(false);
            
            _currentProgram.printCalibers();
            //_cameraManager->savePartidatoJson(_partida_name);
            _cameraManager->savePartidatoPDF(_partida_name);
        }    
    }
    //INICIAR UNA NUEVA PARTIDA 
    else
    {
        // Show dialog to input new partida name
        bool ok;
        string name_new_partida = "PAR"+ getFechaHora();
        QString newName = QInputDialog::getText(this, 
            tr("Nueva Partida"),
            tr("Nombre de la nueva partida:"), 
            QLineEdit::Normal, 
            QString::fromStdString(name_new_partida), 
            &ok);
        if (ok && !newName.isEmpty()) {
            //Activar la partida
            _partida_name = newName.toStdString();
            setPartidaActiva (true);
            _partidaLabel->setText(newName);
        }
    }
}

string MainWindow::getFechaHora()
{
    //Generar una cadena con el formato yyyymmddhhmmss
    time_t now = time(0);
    tm *ltm = localtime(&now);
    
    char buffer[15];  // 14 caracteres para yyyymmddhhmmss + 1 para el null terminator
    snprintf(buffer, sizeof(buffer), "%04d%02d%02d%02d%02d%02d",
        1900 + ltm->tm_year,  // año
        1 + ltm->tm_mon,      // mes
        ltm->tm_mday,         // día
        ltm->tm_hour,         // hora
        ltm->tm_min,          // minutos
        ltm->tm_sec);         // segundos
    
    return string(buffer);
}

