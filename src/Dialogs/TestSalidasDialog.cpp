#include "../../include/Dialogs/TestSalidasDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QMessageBox>
#include <QCloseEvent>

TestSalidasDialog::TestSalidasDialog(int numOutputs, int numLines, OutputBoardManager* outputBoardManager, QWidget* parent)
    : QDialog(parent), _outputBoardManager(outputBoardManager), _numOutputs(numOutputs), _numLines(numLines),
      _testRunning(false), _currentState(false)
{
    setWindowTitle(tr("Test de Salidas"));
    setModal(true);
    
    // Crear timer para el test continuo
    _testTimer = new QTimer(this);
    connect(_testTimer, &QTimer::timeout, this, &TestSalidasDialog::onTimerTick);
    
    setupUI(numOutputs, numLines);
    
    // Calcular ancho necesario dinámicamente
    int columnWidth = 60;
    int verticalHeaderWidth = 40; // Ancho aproximado del header vertical
    int marginsAndSpacing = 60; // Márgenes del layout + espaciado
    int minWidth = (numOutputs * columnWidth) + verticalHeaderWidth + marginsAndSpacing;
    
    // Asegurar un ancho mínimo y añadir buffer adicional
    int dialogWidth = qMax(minWidth, 800); // Mínimo 800px, más 100px de buffer
    
    resize(dialogWidth, 200);
}

void TestSalidasDialog::setupUI(int numOutputs, int numLines)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10); // Reducir márgenes del layout principal
    
    // Información sobre el test
    QLabel* infoLabel = new QLabel(tr("Test de Salidas - Líneas: %1, Salidas por línea: %2\nMarque las salidas a testear y luego active el test")
                                   .arg(numLines).arg(numOutputs), this);
    infoLabel->setStyleSheet("font-weight: bold; font-size: 12pt; color: #2E8B57;");
    mainLayout->addWidget(infoLabel);
    
    // Crear tabla para mostrar estado de salidas - Solo una fila para las salidas
    _table = new QTableWidget(1, numOutputs, this);
    _table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    _table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed); // Cambiar a Fixed para control exacto
    _table->verticalHeader()->setVisible(false); // Ocultar header vertical ya que solo hay una fila
    
    // Configurar headers
    QStringList horizontalHeaders;
    for(int i = 0; i < numOutputs; ++i) {
        horizontalHeaders << QString("S%1").arg(i + 1);
        _table->setColumnWidth(i, 60); // Tamaño correcto de columnas
    }
    _table->setHorizontalHeaderLabels(horizontalHeaders);
    
    // Configurar altura de la única fila
    _table->setRowHeight(0, 40);
    
    // Llenar tabla con botones de test individuales - Solo una fila
    for(int output = 0; output < numOutputs; ++output) {
        QPushButton* testButton = new QPushButton("OFF", _table);
        testButton->setCheckable(true);
        testButton->setStyleSheet(
            "QPushButton { background-color: #FF6B6B; color: white; font-weight: bold; }"
            "QPushButton:checked { background-color: #4ECDC4; }"
        );
        
        // Conectar cada botón individualmente con protección contra test continuo
        connect(testButton, &QPushButton::toggled, this, [=](bool checked) {
            // Durante el test continuo, deshabilitar cambios en los botones
            if (_testRunning) {
                testButton->blockSignals(true);
                testButton->setChecked(!checked); // Revertir el cambio
                testButton->blockSignals(false);
                return;
            }
            
            // En modo preparación: solo cambiar el estado visual del botón
            // No activar hardware, solo marcar qué salidas se van a testear
            testButton->setText(checked ? "ON" : "OFF");
        });
        
        _table->setCellWidget(0, output, testButton);
    }
    
    mainLayout->addWidget(_table);
    
    // Grupo de botones de control
    QGroupBox* controlGroup = new QGroupBox(tr("Controles de Test"), this);
    QGridLayout* controlLayout = new QGridLayout(controlGroup);
    
    // Botones de control
    _testAllButton = new QPushButton(tr("Activar Test"), this);
    _testAllButton->setCheckable(true); // Hacer el botón checkable para que sea interruptor
    _testAllButton->setStyleSheet(
        "QPushButton { background-color: #4ECDC4; color: white; font-weight: bold; padding: 8px; }"
        "QPushButton:checked { background-color: #FF6B6B; }"
    );
    connect(_testAllButton, &QPushButton::clicked, this, &TestSalidasDialog::onTestAllOutputs);
    
    controlLayout->addWidget(_testAllButton, 0, 0);
    
    mainLayout->addWidget(controlGroup);
    
    // Etiqueta de estado
    _statusLabel = new QLabel(tr("Estado: Marque las salidas a testear"), this);
    _statusLabel->setStyleSheet("font-style: italic; color: #666;");
    mainLayout->addWidget(_statusLabel);
    
    // Botones del diálogo
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TestSalidasDialog::onCloseRequested);
    mainLayout->addWidget(buttonBox);
}


void TestSalidasDialog::onTestAllOutputs()
{
    if (!_outputBoardManager) return;
    
    if (_testRunning) {
        // Parar el test
        _testRunning = false;
        _testTimer->stop();
        _testAllButton->setText(tr("Activar Test"));
        _statusLabel->setText(tr("Estado: Marque las salidas a testear"));
        
        // Desactivar todas las salidas
        for(int line = 0; line < _numLines; ++line) {
            if (_outputBoardManager->isBoardConnected(line)) {
                std::vector<int> emptyOutputs;
                _outputBoardManager->setMultipleOutputs(line, emptyOutputs);
            }
        }
        
        // NO cambiar el estado visual de los botones - mantener selección
    } else {
        // Verificar que hay al menos una salida seleccionada
        bool hasSelectedOutputs = false;
        for(int output = 0; output < _numOutputs; ++output) {
            QPushButton* button = qobject_cast<QPushButton*>(_table->cellWidget(0, output));
            if (button && button->isChecked()) {
                hasSelectedOutputs = true;
                break;
            }
        }
        
        if (!hasSelectedOutputs) {
            _statusLabel->setText(tr("Error: Debe marcar al menos una salida"));
            return;
        }
        
        // Iniciar el test
        _testRunning = true;
        _currentState = false; // Empezar con todas OFF
        _testAllButton->setText(tr("Parar Test"));
        _statusLabel->setText(tr("Estado: Test en curso - ciclando salidas seleccionadas"));
        
        // Iniciar el timer - cambiar estado cada 1000ms (1 segundo)
        _testTimer->start(1000);
        
        // Ejecutar el primer tick inmediatamente
        onTimerTick();
    }
}

void TestSalidasDialog::activateOutput(int lineIndex, int outputIndex)
{
    if (!_outputBoardManager || !_outputBoardManager->isBoardConnected(lineIndex)) return;
    
    _outputBoardManager->activateOutput(lineIndex, outputIndex);
}

void TestSalidasDialog::deactivateOutput(int lineIndex, int outputIndex)
{
    if (!_outputBoardManager || !_outputBoardManager->isBoardConnected(lineIndex)) return;
    
    _outputBoardManager->deactivateOutput(lineIndex, outputIndex);
}

void TestSalidasDialog::onTimerTick()
{
    if (!_outputBoardManager || !_testRunning) return;
    
    // Alternar estado
    _currentState = !_currentState;
    
    for(int line = 0; line < _numLines; ++line) {
        if (_outputBoardManager->isBoardConnected(line)) {
            std::vector<int> outputs;
            
            if (_currentState) {
                // Activar solo las salidas que están marcadas como ON
                for(int output = 0; output < _numOutputs; ++output) {
                    QPushButton* button = qobject_cast<QPushButton*>(_table->cellWidget(0, output));
                    if (button && button->isChecked()) {
                        outputs.push_back(output);
                    }
                }
            }
            // Si _currentState es false, el vector queda vacío (todas desactivadas)
            
            _outputBoardManager->setMultipleOutputs(line, outputs);
        }
    }
    
    // NO actualizar estado visual de los botones durante el test
    // Los botones mantienen su estado de selección (marcados/no marcados)
}

void TestSalidasDialog::activateOutputInAllLines(int outputIndex)
{
    // Activar esta salida en todas las líneas conectadas
    for(int line = 0; line < _numLines; ++line) {
        if (_outputBoardManager && _outputBoardManager->isBoardConnected(line)) {
            _outputBoardManager->activateOutput(line, outputIndex);
        }
    }
}

void TestSalidasDialog::deactivateOutputInAllLines(int outputIndex)
{
    // Desactivar esta salida en todas las líneas conectadas
    for(int line = 0; line < _numLines; ++line) {
        if (_outputBoardManager && _outputBoardManager->isBoardConnected(line)) {
            _outputBoardManager->deactivateOutput(line, outputIndex);
        }
    }
}

void TestSalidasDialog::onCloseRequested()
{
    if (_testRunning) {
        _statusLabel->setText(tr("Error: Detenga el test antes de cerrar"));
        return;
    }
    
    // Si no hay test en curso, permitir cerrar
    reject();
}

void TestSalidasDialog::closeEvent(QCloseEvent* event)
{
    if (_testRunning) {
        _statusLabel->setText(tr("Error: Detenga el test antes de cerrar"));
        event->ignore(); // Impedir el cierre
        return;
    }
    
    // Si no hay test en curso, permitir cerrar
    event->accept();
}