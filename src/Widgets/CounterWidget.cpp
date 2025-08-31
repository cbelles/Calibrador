// counterwidget.cpp
#include "Widgets/CounterWidget.h"
#include <QFont>
#include <chrono>
#include <deque>

CounterWidget::CounterWidget(QWidget *parent)
    : QWidget(parent), _numLines(1)  // Default to 1 line
{
    setupDisplays();
}

void CounterWidget::reset()
{
    _numpieces = 0;
    piecesCounter->display(_numpieces);
    _callHistory.clear();
    setCupletesPerSecond(0);
    setFruitsPerSecond(0);
}

void CounterWidget::incNumPieces(const vector<int>& calibres)
{
    auto now = std::chrono::steady_clock::now();
    
    // Count valid pieces (calibres >= 0)
    int validPieces = 0;
    for (int calibre : calibres) {
        if (calibre >= 0) {
            validPieces++;
        }
    }
    
    // Add to history
    _callHistory.push_back({now, validPieces});
    
    // Remove old entries outside the window
    auto windowStart = now - std::chrono::milliseconds(WINDOW_SIZE_MS);
    while (!_callHistory.empty() && _callHistory.front().timestamp < windowStart) {
        _callHistory.pop_front();
    }
    
    // Limit history size
    while (_callHistory.size() > MAX_HISTORY) {
        _callHistory.pop_front();
    }
    
    // Update counters
    _numpieces += validPieces;
}

void CounterWidget::updatePieces()
{
    piecesCounter->display(_numpieces);
}

void CounterWidget::updateCupletsPerSecond()
{
    setCupletesPerSecond(calculateCupletsPerSecond());
}

void CounterWidget::updateFruitsPerSecond()
{
    setFruitsPerSecond(calculateFruitsPerSecond());
}

//PRIVATE

void CounterWidget::setupDisplays()
{
    // Crear el layout principal como horizontal
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10); // Reducir espaciado entre elementos
    
    // Crear un widget contenedor para los contadores
    QWidget* countersWidget = new QWidget(this);
    QHBoxLayout* countersLayout = new QHBoxLayout(countersWidget);
    countersLayout->setSpacing(60); // Aumentar espaciado entre contadores
    
    // Crear los tres grupos de contador+etiqueta
    QVBoxLayout* piecesLayout = new QVBoxLayout();
    piecesLayout->setSpacing(5); // Reducir espaciado entre etiqueta y contador
    QLabel* piecesLabel = createLabel("Piezas");
    piecesCounter = createCounter(7);
    piecesLayout->addWidget(piecesLabel);
    piecesLayout->addWidget(piecesCounter);
    
    QVBoxLayout* cupletsLayout = new QVBoxLayout();
    cupletsLayout->setSpacing(5); // Reducir espaciado entre etiqueta y contador
    QLabel* cupletsLabel = createLabel("Cazoletas/s");
    cupletsCounter = createCounter(2,1);
    cupletsLayout->addWidget(cupletsLabel);
    cupletsLayout->addWidget(cupletsCounter);
    
    QVBoxLayout* fruitsLayout = new QVBoxLayout();
    fruitsLayout->setSpacing(5); // Reducir espaciado entre etiqueta y contador
    QLabel* fruitsLabel = createLabel("Frutas/s");
    fruitsCounter = createCounter(2,1);
    fruitsLayout->addWidget(fruitsLabel);
    fruitsLayout->addWidget(fruitsCounter);

    // Añadir los tres grupos al layout horizontal con margen extra
    countersLayout->addLayout(piecesLayout);
    countersLayout->addLayout(cupletsLayout);
    countersLayout->addLayout(fruitsLayout);
    
    // Añadir el widget contenedor al layout principal
    mainLayout->addWidget(countersWidget);
    mainLayout->addStretch();

    // Ajustar el tamaño del widget
    setMinimumSize(450, 100); // Aumentar el ancho mínimo
    setFixedHeight(100); // Fijar altura
}

QLCDNumber* CounterWidget::createCounter(int numDigits, int decimals)
{
    QLCDNumber *counter = new QLCDNumber(this);
    counter->setDigitCount(numDigits + (decimals > 0 ? 1 : 0));
    counter->setSegmentStyle(QLCDNumber::Filled);
    counter->setSmallDecimalPoint(true);
    
    counter->setStyleSheet("background-color: #CCE5E1; color: #000000; border: none;");
    
    counter->setMinimumHeight(50);
    counter->setMinimumWidth(25 * (numDigits + (decimals > 0 ? 1 : 0))); // Reducir ancho por dígito
    counter->setFixedWidth(25 * (numDigits + (decimals > 0 ? 1 : 0))); // Reducir ancho por dígito
    counter->display(0.0);
    return counter;
}

QLabel* CounterWidget::createLabel(const QString &text)
{
    QLabel *label = new QLabel(text, this);
    QFont font = label->font();
    font.setBold(true);
    label->setFont(font);
    label->setAlignment(Qt::AlignCenter);
    return label;
}

double CounterWidget::calculateCupletsPerSecond()
{
    if (_callHistory.size() < 2) {
        return 0.0;
    }
    
    // Calculate rate based on number of calls
    auto windowSeconds= std::chrono::duration_cast<std::chrono::milliseconds>(
        _callHistory.back().timestamp - _callHistory.front().timestamp).count()  / 1000.0;;
    
    if (windowSeconds == 0) {
        return 0.0;
    }
    
    return _callHistory.size() / windowSeconds;
}

double CounterWidget::calculateFruitsPerSecond()
{
    if (_callHistory.size() < 2) {
        return 0.0;
    }
    
    // Sum all valid pieces in the window
    int totalValidPieces = 0;
    for (const auto& data : _callHistory) {
        totalValidPieces += data.validPieces;
    }
    
    // Calculate time window in seconds
    double windowSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        _callHistory.back().timestamp - _callHistory.front().timestamp).count() / 1000.0;
    
    if (windowSeconds == 0) {
        return 0.0;
    }
    
    // Calculate fruits per second per line
    return totalValidPieces / windowSeconds;
}

void CounterWidget::setCupletesPerSecond(double count)
{
    cupletsCounter->display(QString::number(count, 'f', 1)); // Forzar 1 decimal
}

void CounterWidget::setFruitsPerSecond(double count)
{
    fruitsCounter->display(QString::number(count, 'f', 1)); // Forzar 1 decimal
}
