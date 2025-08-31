// CalibersTable.cpp
#include "CalibersTable.h"
#include <QDebug>
#include <QMessageBox>
#include <iostream>
#include <QFile>
#include <QTextStream>
#include <fstream>
#include <QPainter>


using namespace std;

// Implementación de CalibersTable
CalibersTable::CalibersTable(QWidget *parent)
    : QTableWidget(parent), _calibrationProgram(nullptr)
{
    setupTable();
}

void CalibersTable::setCalibrationProgram(CalibrationProgram *program)
{
    _calibrationProgram = program;
    createTable();
}

void CalibersTable::createTable()
{
    blockSignals(true);
    clearContents();
    setRowCount(0);

    // Actualizar cabeceras
    updateHeaders();
    setColorIndex(_calibrationProgram->getColorIndexFruta());
    const auto &calibers = _calibrationProgram->getCalibers();
    for (const auto& caliber : calibers) 
        addCaliber(caliber);

    blockSignals(false);
}

void CalibersTable::updateTable()
{
    blockSignals(true);
    clearContents();
    setRowCount(0);

    // Actualizar cabeceras
    updateHeaders();
    
    const auto &calibers = _calibrationProgram->getCalibers();

    setColorIndex(_calibrationProgram->getColorIndexFruta());
    for (const auto& caliber : calibers) {

        int row = rowCount();
        insertRow(row);

        // Nombre del calibre
        setItem(row, 0, new QTableWidgetItem(QString::fromStdString(caliber.getName())));
       
        // Rangos de las dimensiones
        const auto& dimensions = caliber.getDimensions();
        for (size_t j = 0; j < dimensions.size(); j++) 
        {
            addSpinBoxToCell(row, j * 2 + 1, dimensions[j].getMinValue());
            addSpinBoxToCell(row, j * 2 + 2, dimensions[j].getMaxValue());
        }
    }
    blockSignals(false);
}

//SLOTS

void CalibersTable::addCaliber(const Caliber& caliber)  // Change to const reference
{
    int row = rowCount();
    insertRow(row);

    // Nombre del calibre
    QTableWidgetItem *nameItem = new QTableWidgetItem(QString::fromStdString(caliber.getName()).left(17));
    nameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    setItem(row, 0, nameItem);

    // Rangos de dimensiones
    const auto &dimensions = caliber.getDimensions();
    for (size_t j = 0; j < dimensions.size(); ++j)
    {
        addSpinBoxToCell(row, j * 2 + 1, dimensions[j].getMinValue());
        addSpinBoxToCell(row, j * 2 + 2, dimensions[j].getMaxValue());
    }
}

int CalibersTable::removeCaliber()
{
    int currentRow = this->currentRow();
    if (currentRow >= 0)
        removeRow(currentRow);
    return currentRow;
}

// PRIVATE SLOTS

void CalibersTable::onCellChanged(int row, int column)
{
    if (row >= 0 && column == 0) {
        // Get the name of the caliber
        QTableWidgetItem* item = this->item(row, column);
        if (item) { 
            QString name = item->text();
            // Limitar a 17 caracteres
            if (name.length() > 17) {
                name = name.left(17);
                item->setText(name);
            }
            _calibrationProgram->getCalibers()[row].setName(name.toStdString());
        }
    }

    if (row >= 0 && column > 0) {
        // Get the spinbox that changed
        QSpinBox* spinBox = qobject_cast<QSpinBox*>(cellWidget(row, column));
        if (spinBox) {
            int newValue = spinBox->value();
            int dimIndex = (column - 1) / 2;  // Calculate dimension index
            bool isMin = (column % 2) == 1;   // Odd column is min value
            
            auto& caliber = _calibrationProgram->getCalibers()[row];
            auto& dim = caliber.getDimensions()[dimIndex];
            
            // Get domain values for this dimension
            int index = dim.getIndex();
            auto domain = CalibrationDimension::_dimension_domain_value[index];
            bool isValid = newValue >= domain.first && newValue <= domain.second;
            
            // Update text color based on validity
            QPalette pal = spinBox->palette();
            pal.setColor(QPalette::Text, isValid ? Qt::black : Qt::red);
            spinBox->setPalette(pal);
            
            if (isMin) {
                dim.setMinValue(newValue);
            } else {
                dim.setMaxValue(newValue);
            }
        }
    }
}

// PRIVATE

void CalibersTable::setupTable()
{
    // Configuración básica de la tabla
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setShowGrid(true);
    setAlternatingRowColors(true);

    // Configurar cabeceras
    verticalHeader()->setVisible(false);
    verticalHeader()->setDefaultSectionSize(30);

    horizontalHeader()->setVisible(true);
    horizontalHeader()->setHighlightSections(true);
    horizontalHeader()->setStretchLastSection(true);

    // Conectar señales
    connect(this, &QTableWidget::cellChanged,
            this, &CalibersTable::onCellChanged);

    // Reemplazar el header horizontal estándar con nuestro header personalizado
    _horizontalHeader = new GroupHeaderView(Qt::Horizontal, this);
    setHorizontalHeader(_horizontalHeader);
    _horizontalHeader->setMinimumHeight(80); // Altura para acomodar dos filas
    _horizontalHeader->setDefaultAlignment(Qt::AlignCenter);
    _horizontalHeader->setSectionResizeMode(QHeaderView::Fixed);

    // Configurar el color de fondo de la cabecera
    QPalette headerPal = _horizontalHeader->palette();
    headerPal.setColor(QPalette::Window, Qt::white);
    headerPal.setColor(QPalette::Button, Qt::white);
    _horizontalHeader->setPalette(headerPal);
    _horizontalHeader->setAutoFillBackground(true);

    QFont boldFont = font();
    boldFont.setBold(true);
    setFont(boldFont);
    
    // Configurar colores alternados
    QPalette pal = palette();
    pal.setColor(QPalette::Base, Qt::white);  // Color base blanco
    pal.setColor(QPalette::Button, Qt::white); // Color de cabecera blanco
    pal.setColor(QPalette::AlternateBase, QColor(240, 248, 255)); // Mantener azul claro para filas alternas
    setPalette(pal);

    // Limitar la edición de texto en la columna de nombres
    setItemDelegate(new CaliberNameDelegate(this));

    // Añadir conexión para detectar cambios en la selección
    connect(selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &CalibersTable::onSelectionChanged);
}

void CalibersTable::setPartidaActiva(bool partida_activa) 
{
    _partida_activa = partida_activa;
    
    // Disable/enable cell editing based on partida status
    for(int row = 0; row < rowCount(); ++row) {
        // Disable/enable name column
        if (QTableWidgetItem* item = this->item(row, 0)) {
            item->setFlags(partida_activa ? 
                (item->flags() & ~Qt::ItemIsEditable) : 
                (item->flags() | Qt::ItemIsEditable));
        }
        
        // Disable/enable spinboxes
        for(int col = 1; col < columnCount(); ++col) {
            if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(cellWidget(row, col))) {
                spinBox->setReadOnly(partida_activa);
                spinBox->setEnabled(!partida_activa);
            }
        }
    }
}

// Añadir este nuevo método
void CalibersTable::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    // Restaurar color normal en las filas deseleccionadas
    for (const QModelIndex &index : deselected.indexes()) {
        int row = index.row();
        for (int col = 0; col < columnCount(); ++col) {
            if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(cellWidget(row, col))) {
                QPalette pal = spinBox->palette();
                if (row % 2 != 0) {
                    pal.setColor(QPalette::Base, palette().color(QPalette::AlternateBase));
                } else {
                    pal.setColor(QPalette::Base, palette().color(QPalette::Base));
                }
                // Mantener el color de fondo para la columna "COLOR"
                int dimIndex = (col - 1) / 2;
                if (dimIndex < _calibrationProgram->getNumDimensions()) {
                    string dimName = _calibrationProgram->getDimension(dimIndex);
                    if (dimName == "COLOR") {
                        pal.setColor(QPalette::Base, getColorFromIndex(spinBox->value()));
                    }
                }
                pal.setColor(QPalette::Text, Qt::black);
                spinBox->setPalette(pal);
            }
        }
    }

    // Aplicar color de selección en las filas seleccionadas
    for (const QModelIndex &index : selected.indexes()) {
        int row = index.row();
        for (int col = 0; col < columnCount(); ++col) {
            if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(cellWidget(row, col))) {
                QPalette pal = spinBox->palette();
                pal.setColor(QPalette::Base, palette().color(QPalette::Highlight));
                pal.setColor(QPalette::Text, palette().color(QPalette::HighlightedText));
                // Mantener el color de fondo para la columna "COLOR"
                int dimIndex = (col - 1) / 2;
                if (dimIndex < _calibrationProgram->getNumDimensions()) {
                    string dimName = _calibrationProgram->getDimension(dimIndex);
                    if (dimName == "COLOR") {
                        pal.setColor(QPalette::Base, getColorFromIndex(spinBox->value()));
                    }
                }
                spinBox->setPalette(pal);
            }
        }
    }

    // Emitir señal con la fila seleccionada
    if (!selected.indexes().isEmpty()) {
        emit rowSelected(selected.indexes().first().row());
    }
}

void CalibersTable::updateHeaders()
{
    QStringList headers;
    QMap<QString, QPair<int, int>> groups;
    
    headers << tr("Nombre Calibres");  // Cambiado de "Nombre" a "Nombre de los Calibres"
    int col = 1;
    
    if (_calibrationProgram->getNumDimensions() > 0)
    {
        for (int i = 0; i < 3; i++) {
            string dimName = _calibrationProgram->getDimension(i);
            if (!dimName.empty()) {
                headers << tr("Min") << tr("Max");
                groups[QString::fromStdString(dimName)] = qMakePair(col, col + 1);
                col += 2;
            }
        }
    }

    setColumnCount(headers.size());
    setHorizontalHeaderLabels(headers);
    
    // Configurar los grupos en el header
    _horizontalHeader->setGroupedColumns(groups);

    // Set column widths
    setColumnWidth(0, 150);  // Name column width
    
    // Set width for dimension columns
    for (int i = 1; i < headers.size(); ++i) {
        setColumnWidth(i, 58);  // Min y Max columns width = 60 pixels
    }
    
    // Optional: Set a fixed size for the table widget
    int totalWidth = 150; // Name column
    totalWidth += _calibrationProgram->getNumDimensions() * 120; // 60 * 2 for each dimension
    setMinimumWidth(totalWidth);
}

void CalibersTable::swapRows(int row1, int row2)
{
    if (_calibrationProgram)
    {
        auto &calibers = _calibrationProgram->getCalibers();
        if (row1 >= 0 && row2 >= 0 &&
            row1 < static_cast<int>(calibers.size()) &&
            row2 < static_cast<int>(calibers.size()))
        {
            std::swap(calibers[row1], calibers[row2]);
        }
    }

    // Intercambiar elementos visuales
    for (int col = 0; col < columnCount(); ++col)
    {
        QTableWidgetItem *item1 = takeItem(row1, col);
        QTableWidgetItem *item2 = takeItem(row2, col);
        QWidget *widget1 = cellWidget(row1, col);
        QWidget *widget2 = cellWidget(row2, col);

        if (item1 && item2)
        {
            setItem(row2, col, item1);
            setItem(row1, col, item2);
        }

        if (widget1 && widget2)
        {
            setCellWidget(row2, col, widget1);
            setCellWidget(row1, col, widget2);
        }
    }
}

QColor CalibersTable::getColorFromIndex(unsigned char index) {
    std::vector<unsigned char> color(3);
    _colorIndexBar.getColorValue(color, index);
    return QColor(color[0], color[1], color[2]);
}

void CalibersTable::addSpinBoxToCell(int row, int col, int value)
{
    QSpinBox *spinBox = new QSpinBox(this);
    spinBox->setMinimum(0);
    spinBox->setMaximum(9999);
    spinBox->setValue(value);
    
    // Obtener la dimensión correspondiente
    int dimIndex = (col - 1) / 2;
    if (dimIndex < _calibrationProgram->getNumDimensions()) {
        string dimName = _calibrationProgram->getDimension(dimIndex);
        if (dimName == "COLOR") {
            // Establecer el color de fondo basado en el valor
            QPalette pal = spinBox->palette();
            pal.setColor(QPalette::Base, getColorFromIndex(value));
            spinBox->setPalette(pal);
        } else {
            // Aplicar el color de fondo alternado normal
            QPalette pal = spinBox->palette();
            if (row % 2 != 0) {
                pal.setColor(QPalette::Base, palette().color(QPalette::AlternateBase));
            } else {
                pal.setColor(QPalette::Base, palette().color(QPalette::Base));
            }
            spinBox->setPalette(pal);
        }
    }

    //AQUI
   
    // Conectar el cambio de valor para actualizar el color si es necesario
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            [=](int newValue) {
        if (dimIndex < _calibrationProgram->getNumDimensions()) {
            string dimName = _calibrationProgram->getDimension(dimIndex);
            if (dimName == "COLOR") {
                QPalette pal = spinBox->palette();
                pal.setColor(QPalette::Base, getColorFromIndex(newValue));
                spinBox->setPalette(pal);
            }
        }
        onCellChanged(row, col);
    });
   
    // Set readonly state based on current partida status
    spinBox->setReadOnly(_partida_activa);
    spinBox->setEnabled(!_partida_activa);
    
    setCellWidget(row, col, spinBox);
}

void CalibersTable::setColorIndex(const std::string& colorindex)
{
    _colorIndexBar.setColorIndex(colorindex);
    // Actualizar las celdas de color si existen
    if (_calibrationProgram) {
        for (int row = 0; row < rowCount(); row++) {
            for (int col = 1; col < columnCount(); col++) {
                int dimIndex = (col - 1) / 2;
                if (dimIndex < _calibrationProgram->getNumDimensions()) {
                    string dimName = _calibrationProgram->getDimension(dimIndex);
                    if (dimName == "COLOR") {
                        QSpinBox* spinBox = qobject_cast<QSpinBox*>(cellWidget(row, col));
                        if (spinBox) {
                            QPalette pal = spinBox->palette();
                            pal.setColor(QPalette::Base, getColorFromIndex(spinBox->value()));
                            spinBox->setPalette(pal);
                        }
                    }
                }
            }
        }
    }
}

GroupHeaderView::GroupHeaderView(Qt::Orientation orientation, QWidget* parent)
    : QHeaderView(orientation, parent)
{
    setSectionsClickable(true);
    setSectionResizeMode(QHeaderView::Interactive);
}

void GroupHeaderView::setGroupedColumns(const QMap<QString, QPair<int, int>>& groups)
{
    _groups = groups;
    update();
}

void GroupHeaderView::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
    painter->save();
    
    // Si es la columna del nombre, pintar solo en la parte inferior
    if (logicalIndex == 0) {
        QRect bottomRect(rect.x(), rect.height()/2, rect.width(), rect.height()/2);
        painter->fillRect(bottomRect, palette().brush(QPalette::Button));
        painter->setPen(palette().color(QPalette::Dark));
        painter->drawLine(bottomRect.topLeft(), bottomRect.topRight());
        painter->drawLine(bottomRect.topRight(), bottomRect.bottomRight());
        painter->drawLine(bottomRect.bottomRight(), bottomRect.bottomLeft());
        painter->drawLine(bottomRect.bottomLeft(), bottomRect.topLeft());
        
        painter->setPen(palette().color(QPalette::ButtonText));
        painter->drawText(bottomRect, Qt::AlignLeft | Qt::AlignVCenter, " " + model()->headerData(logicalIndex, orientation()).toString());
        painter->restore();
        return;
    }

    // Para las demás columnas
    // 1. Pintar la parte superior (nombre de la dimensión)
    for (auto it = _groups.constBegin(); it != _groups.constEnd(); ++it) {
        if (logicalIndex >= it.value().first && logicalIndex <= it.value().second) {
            if (logicalIndex == it.value().first) {  // Solo pintar el grupo una vez
                int startX = sectionPosition(it.value().first);
                int width = sectionPosition(it.value().second) - startX + sectionSize(it.value().second);
                
                QRect groupRect(startX, 0, width, rect.height()/2);
                
                painter->fillRect(groupRect, palette().brush(QPalette::Button));
                painter->setPen(palette().color(QPalette::Dark));
                
                // Dibujar las líneas del borde manualmente
                painter->drawLine(groupRect.topLeft(), groupRect.topRight());
                painter->drawLine(groupRect.topRight(), groupRect.bottomRight());
                painter->drawLine(groupRect.bottomRight(), groupRect.bottomLeft());
                painter->drawLine(groupRect.bottomLeft(), groupRect.topLeft());
                
                painter->setPen(palette().color(QPalette::ButtonText));
                painter->drawText(groupRect, Qt::AlignCenter, it.key());
            }
        }
    }

    // 2. Pintar la parte inferior (Min/Max)
    QRect bottomRect(rect.x(), rect.height()/2, rect.width(), rect.height()/2);
    QString text = (logicalIndex % 2 == 1) ? "Min" : "Max";
    
    painter->fillRect(bottomRect, palette().brush(QPalette::Button));
    painter->setPen(palette().color(QPalette::Dark));
    
    // Dibujar las líneas del borde manualmente
    painter->drawLine(bottomRect.topLeft(), bottomRect.topRight());
    painter->drawLine(bottomRect.topRight(), bottomRect.bottomRight());
    painter->drawLine(bottomRect.bottomRight(), bottomRect.bottomLeft());
    painter->drawLine(bottomRect.bottomLeft(), bottomRect.topLeft());
    
    painter->setPen(palette().color(QPalette::ButtonText));
    painter->drawText(bottomRect, Qt::AlignCenter, text);

    painter->restore();
}

QSize GroupHeaderView::sectionSizeFromContents(int logicalIndex) const
{
    QSize size = QHeaderView::sectionSizeFromContents(logicalIndex);
    size.setHeight(height() / 2);
    return size;
}
