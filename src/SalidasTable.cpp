#include "SalidasTable.h"
#include <QPainter>
#include <QPalette>
#include <QHeaderView>

SalidaHeaderView::SalidaHeaderView(Qt::Orientation orientation, QWidget* parent)
    : QHeaderView(orientation, parent)
{
    setSectionsClickable(true);
    setSectionResizeMode(QHeaderView::Fixed);
}

void SalidaHeaderView::setHeaders(const QStringList& headers)
{
    _headers = headers;
    update();
}

void SalidaHeaderView::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
    painter->save();
    
    QRect headerRect = rect;
    
    // Ajustar el rect para la última columna si es necesario
    if (logicalIndex == count() - 1) {
        headerRect.setRight(headerRect.right() - 1); // Restar 1 pixel para el borde
    }
    
    painter->fillRect(headerRect, palette().brush(QPalette::Button));
    painter->setPen(palette().color(QPalette::Dark));
    
    // Dibujar borde incluyendo la línea inferior
    painter->drawRect(headerRect);
    
    // Asegurar que se dibuje la línea inferior
    painter->drawLine(headerRect.bottomLeft(), headerRect.bottomRight());
    
    // Configurar una fuente más pequeña
    QFont headerFont = painter->font();
    headerFont.setPointSizeF(headerFont.pointSizeF() * 0.8   ); // Reducir tamaño en un 20%
    painter->setFont(headerFont);
    
    // Dibujar texto centrado verticalmente en la mitad inferior
    QRect textRect(headerRect.x(), headerRect.height()/2, headerRect.width(), headerRect.height()/2);
    painter->setPen(palette().color(QPalette::ButtonText));
    painter->drawText(textRect, Qt::AlignCenter, _headers[logicalIndex]);
    
    painter->restore();
}

QSize SalidaHeaderView::sectionSizeFromContents(int logicalIndex) const
{
    QSize size = QHeaderView::sectionSizeFromContents(logicalIndex);
    size.setHeight(80); // Misma altura que GroupHeaderView
    return size;
}
SalidasTable::SalidasTable(QWidget *parent)
    : QTableWidget(parent), _calibrationProgram(nullptr), _numSalidas(0)
{
    setupTable();
}

void SalidasTable::setCalibrationProgram(CalibrationProgram *program)
{
    _calibrationProgram = program;
    updateTable();
}

std::vector<std::vector<int>> SalidasTable::getInfo() const
{
    std::vector<std::vector<int>> matrix;
    
    // Redimensionar la matriz al tamaño de la tabla
    matrix.resize(rowCount(), std::vector<int>(columnCount()));
    
    // Llenar la matriz con los valores de los SpinBox
    for (int i = 0; i < rowCount(); i++) {
        for (int j = 0; j < columnCount(); j++) {
            if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(cellWidget(i, j))) {
                matrix[i][j] = spinBox->value();
            }
        }
    }
    
    return matrix;
}

void SalidasTable::setNumSalidas(int num)
{
    _numSalidas = num;
    setColumnCount(num);
    updateHeaders();
}

void SalidasTable::addRow()
{
    int row = rowCount();
    cout << "Numero de filas: " << row << endl;
    insertRow(row);
    cout << "Numero de filas: " << rowCount() << endl;
    
    // Añadir SpinBox a cada columna
    for(int col = 0; col < _numSalidas; col++) {
        addSpinBoxToCell(row, col);
    }
}

void SalidasTable::removeRow(int row)
{
    QTableWidget::removeRow(row);
}

void SalidasTable::setupTable()
{
    // Configuración básica de la tabla
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setShowGrid(true);
    setAlternatingRowColors(true);

    // Configurar cabeceras
    verticalHeader()->setVisible(false);
    verticalHeader()->setDefaultSectionSize(30);

    // Reemplazar el header horizontal con nuestro header personalizado
    _horizontalHeader = new SalidaHeaderView(Qt::Horizontal, this);
    setHorizontalHeader(_horizontalHeader);
    _horizontalHeader->setMinimumHeight(80);  // Cambiar a 80 para coincidir con GroupHeaderView
    _horizontalHeader->setDefaultAlignment(Qt::AlignCenter);
    _horizontalHeader->setSectionResizeMode(QHeaderView::Fixed);
    _horizontalHeader->setStretchLastSection(false);  // Añadir esta línea

    // Configurar el color de fondo de la cabecera
    QPalette headerPal = _horizontalHeader->palette();
    headerPal.setColor(QPalette::Window, Qt::white);
    headerPal.setColor(QPalette::Button, Qt::white);
    _horizontalHeader->setPalette(headerPal);
    _horizontalHeader->setAutoFillBackground(true);

    // Configurar la fuente
    QFont boldFont = font();
    boldFont.setBold(true);
    setFont(boldFont);
    
    // Configurar colores alternados
    QPalette pal = palette();
    pal.setColor(QPalette::Base, Qt::white);
    pal.setColor(QPalette::Button, Qt::white);
    pal.setColor(QPalette::AlternateBase, QColor(240, 248, 255));
    setPalette(pal);

    // Conectar señales
    connect(this, &QTableWidget::cellChanged,
            this, &SalidasTable::onCellChanged);

    // Asegurar que la última columna no se estire
    _horizontalHeader->setStretchLastSection(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Evitar barra de desplazamiento

    // Establecer ancho fijo para la tabla basado en el máximo número de columnas
    setFixedWidth(MAX_COLUMNS * COLUMN_WIDTH + 16); // +2 para el borde
    setMaximumWidth(MAX_COLUMNS * COLUMN_WIDTH + 16);
}

void SalidasTable::updateHeaders()
{
    QStringList headers;
    
    // Crear cabeceras para cada salida
    for(int i = 0; i < _numSalidas; i++) {
        headers << tr("Salida %1").arg(i + 1);
    }

    setHorizontalHeaderLabels(headers);
    _horizontalHeader->setHeaders(headers);  // Nuevo método

    // Establecer el ancho de las columnas
    for(int i = 0; i < headers.size(); i++) {
        setColumnWidth(i, COLUMN_WIDTH);
    }
    
    // Ya no necesitamos establecer el ancho aquí porque es fijo
    // setFixedWidth(_numSalidas * COLUMN_WIDTH + 2);  // Eliminar esta línea
    // setMaximumWidth(_numSalidas * COLUMN_WIDTH + 2);  // Eliminar esta línea
}

void SalidasTable::addSpinBoxToCell(int row, int col, int value)
{
    QSpinBox *spinBox = new QSpinBox(this);
    spinBox->setMinimum(0);
    spinBox->setMaximum(100);
    spinBox->setValue(value);
    
    // Aplicar el color de fondo alternado
    QPalette pal = spinBox->palette();
    if (row % 2 != 0) {
        pal.setColor(QPalette::Base, palette().color(QPalette::AlternateBase));
    } else {
        pal.setColor(QPalette::Base, palette().color(QPalette::Base));
    }
    spinBox->setPalette(pal);
    
    // Usar valueChanged para respuesta inmediata en lugar de editingFinished
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [=](int) { onCellChanged(row, col); });
    
    setCellWidget(row, col, spinBox);
}

void SalidasTable::onCellChanged(int row, int column)
{
    QSpinBox* spinBox = qobject_cast<QSpinBox*>(cellWidget(row, column));
    if (spinBox && _calibrationProgram) {
        _calibrationProgram->setSalida(row, column, spinBox->value());
        
        // Change text color to red if partida is active
        if (_partida_activa) {
            QPalette pal = spinBox->palette();
            pal.setColor(QPalette::Text, Qt::red);
            spinBox->setPalette(pal);
        }

        if (_cameraManager) {
            _cameraManager->getOutputManager()->reset();
        }
    }
}

void SalidasTable::updateTable()
{
    if (!_calibrationProgram) return;
    
    // Limpiar tabla actual
    clearContents();
    setRowCount(0);
    
    // A  ñadir filas según el número de calibres en el programa
    const auto& calibers = _calibrationProgram->getCalibers();
    std::vector<std::vector<int>> salidas = _calibrationProgram->getSalidas();
    
    for (size_t i = 0; i < calibers.size(); ++i) {
        addRow();
        // Actualizar valores de las salidas si existen
        if (i < salidas.size()) {
            for (size_t j = 0; j < salidas[i].size() && j < _numSalidas; ++j) {
                if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(cellWidget(i, j))) {
                    spinBox->setValue(salidas[i][j]);
                    // Reset text color to default black
                    QPalette pal = spinBox->palette();
                    pal.setColor(QPalette::Text, Qt::black);
                    spinBox->setPalette(pal);
                }
            }
        }
    }
}

void SalidasTable::highlightRow(int row)
{
    // Restaurar colores normales en todas las filas
    for (int r = 0; r < rowCount(); ++r) {
        for (int col = 0; col < columnCount(); ++col) {
            if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(cellWidget(r, col))) {
                QPalette pal = spinBox->palette();
                if (r % 2 != 0) {
                    pal.setColor(QPalette::Base, palette().color(QPalette::AlternateBase));
                } else {
                    pal.setColor(QPalette::Base, palette().color(QPalette::Base));
                }
                pal.setColor(QPalette::Text, Qt::black);
                spinBox->setPalette(pal);
            }
        }
    }

    // Aplicar color de selección a la fila indicada
    if (row >= 0 && row < rowCount()) {
        for (int col = 0; col < columnCount(); ++col) {
            if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(cellWidget(row, col))) {
                QPalette pal = spinBox->palette();
                pal.setColor(QPalette::Base, palette().color(QPalette::Highlight));
                pal.setColor(QPalette::Text, palette().color(QPalette::HighlightedText));
                spinBox->setPalette(pal);
            }
        }
    }
}