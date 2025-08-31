#include "Widgets/DimensionsWidget.h"
#include "FruitColorIndexs/ColorIndexBar.h"
#include <QFont>

DimensionsWidget::DimensionsWidget(QWidget *parent)
    : QWidget(parent)
    , xMaxDomain(1000)  // Valores por defecto
    , yMaxDomain(1000)
    , zMaxDomain(1000)
    , currentLine(0)
    , _calibrationProgram(nullptr)  // Asegurarse de inicializar todos los punteros
{
    setupDisplays();
    QMetaObject::invokeMethod(this, [this]() {
        if (lineSelector) {
            lineSelector->clear();
            lineSelector->addItem("L1");
        }
    }, Qt::QueuedConnection);
}

void DimensionsWidget::reset()
{
    setXDimension(0);
    setYDimension(0);
    setZDimension(0);
}

void DimensionsWidget::setCalibrationProgram(CalibrationProgram *program)
{
    _calibrationProgram = program;
    update();
}

void DimensionsWidget::updateCounterColor(QLCDNumber* counter, const std::string& dimension, int value) {
    if (dimension == "COLOR") {
        ColorIndexBar colorBar;
        colorBar.setColorIndex(_calibrationProgram->getColorIndexFruta());
        std::vector<unsigned char> color;
        colorBar.getColorValue(color, value);
        QString stylesheet = QString("background-color: rgb(%1,%2,%3); color: #000000; border: none;")
            .arg(color[0]).arg(color[1]).arg(color[2]);
        counter->setStyleSheet(stylesheet);
    }
}

void DimensionsWidget::setXDimension(int value)
{
    int validValue = validateValue(static_cast<int>(value), xMaxDomain);
    xCounter->display(validValue);   
    if (_calibrationProgram)
        updateCounterColor(xCounter, _calibrationProgram->getDimension(0), validValue);
}

void DimensionsWidget::setYDimension(int value)
{
    int validValue = validateValue(static_cast<int>(value), yMaxDomain);
    yCounter->display(validValue);   
    if (_calibrationProgram && _calibrationProgram->getNumDimensions() > 1)
        updateCounterColor(yCounter, _calibrationProgram->getDimension(1), validValue);
}

void DimensionsWidget::setZDimension(int value)
{
    int validValue = validateValue(static_cast<int>(value), zMaxDomain);
    zCounter->display(validValue);    
    if (_calibrationProgram && _calibrationProgram->getNumDimensions() > 2)
        updateCounterColor(zCounter, _calibrationProgram->getDimension(2), validValue);
}

void DimensionsWidget::setXDomain(int maxValue)
{
    if (maxValue > 0) {
        xMaxDomain = maxValue;
        int currentValue = static_cast<int>(xCounter->value());
        setXDimension(currentValue);
    }
}

void DimensionsWidget::setYDomain(int maxValue)
{
    if (maxValue > 0) {
        yMaxDomain = maxValue;
        int currentValue = static_cast<int>(yCounter->value());
        setYDimension(currentValue);
    }
}

void DimensionsWidget::setZDomain(int maxValue)
{
    if (maxValue > 0) {
        zMaxDomain = maxValue;
        int currentValue = static_cast<int>(zCounter->value());
        setZDimension(currentValue);
    }
}

void DimensionsWidget::setXName(const std::string& name)
{
    xLabel->setText(QString::fromStdString(name) );
}

void DimensionsWidget::setYName(const std::string& name)
{
    yLabel->setText(QString::fromStdString(name) );
}

void DimensionsWidget::setZName(const std::string& name)
{
    zLabel->setText(QString::fromStdString(name) );
}

void DimensionsWidget::update() {
    // Ocultar todos los contadores y etiquetas
    xCounter->setVisible(false);
    yCounter->setVisible(false);
    zCounter->setVisible(false);
    xLabel->setVisible(false);
    yLabel->setVisible(false);
    zLabel->setVisible(false);

    if (!_calibrationProgram) return;

    // Array de punteros a los controles para cada dimensión
    struct DimensionControls {
        QLCDNumber* counter;
        QLabel* label;
        void (DimensionsWidget::*setName)(const std::string&);
        void (DimensionsWidget::*setDomain)(int);
    };

    DimensionControls controls[] = {
        {xCounter, xLabel, &DimensionsWidget::setXName, &DimensionsWidget::setXDomain},
        {yCounter, yLabel, &DimensionsWidget::setYName, &DimensionsWidget::setYDomain},
        {zCounter, zLabel, &DimensionsWidget::setZName, &DimensionsWidget::setZDomain}
    };

    // Procesar cada dimensión
    for(int i = 0; i < _calibrationProgram->getNumDimensions() && i < 3; i++) {
        auto& control = controls[i];
        control.counter->setVisible(true);
        control.label->setVisible(true);

        string dimName = _calibrationProgram->getDimension(i);
        (this->*control.setName)(dimName);

        // Cambiar color de fondo si es dimensión COLOR
        if (dimName == "COLOR") {
            ColorIndexBar colorBar;
            colorBar.setColorIndex(_calibrationProgram->getColorIndexFruta());
            std::vector<unsigned char> color;
            colorBar.getColorValue(color, control.counter->intValue());
            QString stylesheet = QString("background-color: rgb(%1,%2,%3); color: #000000; border: none;")
                .arg(color[0]).arg(color[1]).arg(color[2]);
            control.counter->setStyleSheet(stylesheet);
        } else {
            control.counter->setStyleSheet("background-color: #CCE5E1; color: #000000; border: none;");
        }

        int index = CalibrationDimension::getIndexOfName(dimName);
        if (index != -1) {
            pair<int,int> domain = CalibrationDimension::_dimension_domain_value[index];
            (this->*control.setDomain)(domain.second);
        }
    }

    // Resetear valores
    setXDimension(0);
    setYDimension(0);
    setZDimension(0);
}

void DimensionsWidget::updateDimensions(vector<int> dimensions)
{
  for (int i=0; i<dimensions.size(); i++)
  {
    if (i==0)
      setXDimension(dimensions[i]);
    else if (i==1)
      setYDimension(dimensions[i]);
    else if (i==2)
      setZDimension(dimensions[i]);
  }
}


// Definir el mapa de dimensiones
const std::vector<DimensionsWidget::DimensionMapping> DimensionsWidget::_dimensionMappings = {
    {"COLOR", "color"},
    {"AREA", "area"},
    {"DIAMETER_MIN", "di menor"},
    {"DIAMETER_MAX", "di mayor"}
    // Añadir más mappings según sea necesario
};

//PRIVATE

void DimensionsWidget::setupDisplays()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    
    QWidget* dimensionsWidget = new QWidget(this);
    QHBoxLayout* dimensionsLayout = new QHBoxLayout(dimensionsWidget);
    dimensionsLayout->setSpacing(20);  // Cambiar de 60 a 20 para reducir el espacio
    
    // Añadir selector de línea
    lineSelector = new QComboBox(this);
    lineSelector->setFixedWidth(50);
    connect(lineSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DimensionsWidget::onLineChanged);
    dimensionsLayout->addWidget(lineSelector);

    // X dimension
    QVBoxLayout* xLayout = new QVBoxLayout();
    xLayout->setSpacing(5);
    xLabel = createLabel("X");
    xCounter = createCounter(4, 0);  // 4 dígitos, sin decimales
    xLayout->addWidget(xLabel);
    xLayout->addWidget(xCounter);
    
    // Y dimension
    QVBoxLayout* yLayout = new QVBoxLayout();
    yLayout->setSpacing(5);
    yLabel = createLabel("Y");
    yCounter = createCounter(4, 0);  // 4 dígitos, sin decimales
    yLayout->addWidget(yLabel);
    yLayout->addWidget(yCounter);
    
    // Z dimension
    QVBoxLayout* zLayout = new QVBoxLayout();
    zLayout->setSpacing(5);
    zLabel = createLabel("Z");
    zCounter = createCounter(4, 0);  // 4 dígitos, sin decimales
    zLayout->addWidget(zLabel);
    zLayout->addWidget(zCounter);

    dimensionsLayout->addLayout(xLayout);
    dimensionsLayout->addLayout(yLayout);
    dimensionsLayout->addLayout(zLayout);
    
    mainLayout->addWidget(dimensionsWidget);
    mainLayout->addStretch();

    setMinimumSize(450, 100);
    setFixedHeight(100);
}

QLCDNumber* DimensionsWidget::createCounter(int numDigits, int decimals)
{
    QLCDNumber *counter = new QLCDNumber(this);
    counter->setDigitCount(numDigits + (decimals > 0 ? 1 : 0));
    counter->setSegmentStyle(QLCDNumber::Filled);
    counter->setSmallDecimalPoint(true);
    counter->setStyleSheet("background-color: #CCE5E1; color: #000000; border: none;");
    counter->setMinimumHeight(50);
    counter->setMinimumWidth(25 * (numDigits + (decimals > 0 ? 1 : 0)));
    counter->setFixedWidth(25 * (numDigits + (decimals > 0 ? 1 : 0)));
    counter->display(0.0);
    return counter;
}

QLabel* DimensionsWidget::createLabel(const QString &text)
{
    QLabel *label = new QLabel(text, this);
    QFont font = label->font();
    font.setBold(true);
    font.setPointSize(8);  // Reducir el tamaño de la fuente a 8 puntos
    label->setFont(font);
    label->setAlignment(Qt::AlignCenter);
    return label;
}

int DimensionsWidget::validateValue(int value, int maxDomain) const
{
    if (value < 0) return 0;
    if (value > maxDomain) return maxDomain;
    return value;
}

std::string DimensionsWidget::getMessageField(const std::string& dimension) const {
    for (const auto& mapping : _dimensionMappings) {
        if (mapping.programDimension == dimension) {
            return mapping.messageField;
        }
    }
    return "";
}

void DimensionsWidget::setNumLines(int numLines)
{
    lineSelector->clear();
    for(int i = 0; i < numLines; ++i) {
        lineSelector->addItem(QString("L%1").arg(i + 1));
    }
}

void DimensionsWidget::onLineChanged(int index)
{
    currentLine = index;
    clear();
}

void DimensionsWidget::clear()
{
    xCounter->display(0);
    yCounter->display(0);
    zCounter->display(0);
}
