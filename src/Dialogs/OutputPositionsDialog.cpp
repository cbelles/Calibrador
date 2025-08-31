#include "../../include/Dialogs/OutputPositionsDialog.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>

OutputPositionsDialog::OutputPositionsDialog(int numOutputs, const std::vector<int>& positions, const QString& configPath, QWidget* parent)
    : QDialog(parent), _configPath(configPath)
{
    setWindowTitle(tr("Configurar Posiciones de Salidas"));
    // Intentar cargar posiciones del archivo, si no existe usar las proporcionadas
    std::vector<int> loadedPositions = loadPositions(numOutputs);
    if (!loadedPositions.empty()) {
        setupUI(numOutputs, loadedPositions);
    } else {
        setupUI(numOutputs, positions);
    }
}

void OutputPositionsDialog::setupUI(int numOutputs, const std::vector<int>& positions)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Crear la tabla
    _table = new QTableWidget(1, numOutputs, this);
    _table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    _table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    _table->verticalHeader()->setVisible(false);
    
    // Configurar headers
    QStringList headers;
    for(int i = 0; i < numOutputs; ++i) {
        headers << QString("Salida %1").arg(i + 1);
        _table->setColumnWidth(i, 70); // Reduced column width
    }
    _table->setHorizontalHeaderLabels(headers);
    
    // Añadir SpinBoxes
    for(int col = 0; col < numOutputs; ++col) {
        QSpinBox* spinBox = new QSpinBox(_table);
        spinBox->setRange(0, 999);
        spinBox->setValue(col < positions.size() ? positions[col] : col);
        _table->setCellWidget(0, col, spinBox);
        // Conectar cambios del spinBox
        connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
                this, &OutputPositionsDialog::updateSaveButtonState);
    }
    
    mainLayout->addWidget(new QLabel(tr("Configurar posición física de cada salida:"), this));
    mainLayout->addWidget(_table);
    
    // Botones OK/Cancel
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    _saveButton = buttonBox->button(QDialogButtonBox::Ok);
    _saveButton->setText(tr("Guardar"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (validatePositions()) {
            savePositions();
            accept();
        }
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
    
    // Set a reasonable initial size for the dialog
    resize(std::min(1000, numOutputs * 75), 150);
}

std::vector<int> OutputPositionsDialog::getPositions() const
{
    std::vector<int> positions;
    for(int col = 0; col < _table->columnCount(); ++col) {
        QSpinBox* spinBox = qobject_cast<QSpinBox*>(_table->cellWidget(0, col));
        if(spinBox) {
            positions.push_back(spinBox->value());
        }
    }
    return positions;
}

void OutputPositionsDialog::savePositions()
{
    QJsonObject configObj;
    QJsonArray posArray;
    
    auto positions = getPositions();
    for(int pos : positions) {
        posArray.append(pos);
    }
    
    configObj["positions"] = posArray;
    
    QJsonDocument doc(configObj);
    QFile file(_configPath + "/config_pos_salidas.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

std::vector<int> OutputPositionsDialog::loadPositions(int numOutputs) const
{
    std::vector<int> positions;
    QFile file(_configPath + "/config_pos_salidas.json");
    
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("positions")) {
                QJsonArray posArray = obj["positions"].toArray();
                
                // Solo cargar si el número de posiciones coincide
                if (posArray.size() == numOutputs) {
                    for (const QJsonValue &value : posArray) {
                        positions.push_back(value.toInt());
                    }
                }
            }
        }
        file.close();
    }
    
    return positions;
}

bool OutputPositionsDialog::validatePositions() const
{
    auto positions = getPositions();
    for(size_t i = 1; i < positions.size(); ++i) {
        if(positions[i] <= positions[i-1]) {
            return false;
        }
    }
    return true;
}

void OutputPositionsDialog::updateSaveButtonState()
{
    bool isValid = validatePositions();
    QString styleSheet = isValid ? 
        "QPushButton { background-color: #CCE5E1; }" :  // Verde claro
        "QPushButton { background-color: #FF6B6B; }";   // Rojo claro
    
    _saveButton->setStyleSheet(styleSheet);
    _saveButton->setToolTip(isValid ? 
        "" : tr("Los valores deben estar en orden creciente"));
}