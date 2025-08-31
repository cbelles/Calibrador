#include "CaliberHistory.h"
#include <ctime>
#include <chrono>  // Añadir este include para std::chrono
#include <fstream>
#include <iostream>
#include "Utils/json.hpp"

using json = nlohmann::json;

CaliberHistory::CaliberHistory() {}

CaliberHistory::~CaliberHistory() {}


void CaliberHistory::setCalibrationProgram(CalibrationProgram* program)
{
    _calibrationProgram = program;
}

void CaliberHistory::setIni() {
    _startTime = std::time(nullptr);
}

void CaliberHistory::setEnd() {
    _endTime = std::time(nullptr);
}

void CaliberHistory::addCaliberEntry(const std::vector<int>& calibers) {
    std::string timestamp = getCurrentTimestamp();
    _history[timestamp] = calibers;
}

void CaliberHistory::clear() {
    _history.clear();
}

bool CaliberHistory::saveToFile(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return false;
    }

    // Guardar el número de entradas
    size_t numEntries = _history.size();
    file.write(reinterpret_cast<const char*>(&numEntries), sizeof(numEntries));

    // Guardar cada entrada
    for (const auto& entry : _history) {
        // Guardar el timestamp
        size_t timestampLength = entry.first.length();
        file.write(reinterpret_cast<const char*>(&timestampLength), sizeof(timestampLength));
        file.write(entry.first.c_str(), timestampLength);

        // Guardar el vector de calibres
        size_t vectorSize = entry.second.size();
        file.write(reinterpret_cast<const char*>(&vectorSize), sizeof(vectorSize));
        file.write(reinterpret_cast<const char*>(entry.second.data()), vectorSize * sizeof(int));
    }

    return true;
}

bool CaliberHistory::loadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return false;
    }

    _history.clear();

    // Leer el número de entradas
    size_t numEntries;
    file.read(reinterpret_cast<char*>(&numEntries), sizeof(numEntries));

    // Leer cada entrada
    for (size_t i = 0; i < numEntries; i++) {
        // Leer el timestamp
        size_t timestampLength;
        file.read(reinterpret_cast<char*>(&timestampLength), sizeof(timestampLength));
        std::string timestamp(timestampLength, '\0');
        file.read(&timestamp[0], timestampLength);

        // Leer el vector de calibres
        size_t vectorSize;
        file.read(reinterpret_cast<char*>(&vectorSize), sizeof(vectorSize));
        std::vector<int> calibers(vectorSize);
        file.read(reinterpret_cast<char*>(calibers.data()), vectorSize * sizeof(int));

        // Almacenar en el mapa
        _history[timestamp] = calibers;
    }

    return true;
}

std::string CaliberHistory::toJSON() const {
    json j;
    
    // Metadatos
    j["partida"] = _partida;
    j["fruit"] = _calibrationProgram->getFruta();    
    j["variety"] = _calibrationProgram->getVariedad();  
    j["start_time"] = getFormattedDateTime(_startTime);
    j["end_time"] = getFormattedDateTime(_endTime);

    // Estadísticas
    json stats;
    std::map<int, int> caliber_counts;
    int total_pieces = 0;

    // Contar piezas por calibre
    for (const auto& entry : _history) {
        for (int caliber : entry.second) {
            caliber_counts[caliber]++;
            if (caliber>=0) total_pieces++;
        }
    }

    stats["total_pieces"] = total_pieces;
    stats["caliber_counts"] = caliber_counts;
    stats["num_entries"] = _history.size();

    j["statistics"] = stats;

    return j.dump(4);  // 4 espacios de indentación
}

std::string CaliberHistory::calibersToJSON() const {
    json j;
    
    // Dimension names
    json dimensions = json::array();
    for (int i = 0; i < _calibrationProgram->getNumDimensions(); i++) {
        dimensions.push_back(_calibrationProgram->getDimension(i));
    }
    j["dimensions"] = dimensions;
    
    // Calibers
    json calibers = json::array();
    for (const auto& caliber : _calibrationProgram->getCalibers()) {
        json caliberJson;
        caliberJson["name"] = caliber.getName();
        
        json dimensionsData = json::array();
        for (const auto& dim : caliber.getDimensions()) {
            json dimJson;
            dimJson["min"] = dim.getMinValue();
            dimJson["max"] = dim.getMaxValue();
            dimensionsData.push_back(dimJson);
        }
        caliberJson["dimensions"] = dimensionsData;
        calibers.push_back(caliberJson);
    }
    j["calibers"] = calibers;
    
    return j.dump(4);  // 4 spaces indentation
}

bool CaliberHistory::exportToPDF(const std::string& filename) const {
    if (!_calibrationProgram) {
        std::cerr << "Error: No calibration program set" << std::endl;
        return false;
    }

    // Configurar QPrinter
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(QString::fromStdString(filename));
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Portrait);

    QPainter painter;
    if (!painter.begin(&printer)) {
        std::cerr << "Error: Cannot start PDF generation" << std::endl;
        return false;
    }

    try {
        // Configurar márgenes y fuentes
        const int margin = 50;
        int y = margin;
        QFont font("Helvetica", 12);
        QFont titleFont("Helvetica", 16, QFont::Bold);
 
        // Título
        painter.setFont(titleFont);
        painter.drawText(margin, y, "Informe de Calibración");
        y += 40;

        // Información del encabezado
        painter.setFont(font);
        const std::string partida = _partida.empty() ? "N/A" : _partida;
        const std::string fruta = _calibrationProgram->getFruta().empty() ? "N/A" : _calibrationProgram->getFruta();
        const std::string variedad = _calibrationProgram->getVariedad().empty() ? "N/A" : _calibrationProgram->getVariedad();
        const std::string inicio = getFormattedDateTime(_startTime);
        const std::string fin = getFormattedDateTime(_endTime);

        std::vector<std::string> headerLines = {
            "Partida: " + partida,
            "Fruta: " + fruta,
            "Variedad: " + variedad,
            "Inicio: " + inicio,
            "Fin: " + fin
        };

        for (const auto& line : headerLines) {
            painter.drawText(margin, y, QString::fromStdString(line));
            y += 20;
        }
  
        // Añadir tabla de estadísticas
        y += 40;
        painter.setFont(QFont("Helvetica", 14, QFont::Bold));
        painter.drawText(margin, y, "Estadísticas por Calibre");
        y += 30;

        // Configurar la tabla
        const int colWidth = 150;
        const int rowHeight = 25;
        painter.setFont(font);

        // Dibujar encabezados de columna
        const int pageWidth = printer.pageRect().width();
        const int tableWidth = pageWidth - (margin * 2);  // Ancho total disponible
        const int colWidth1 = tableWidth * 0.5;    // 50% para el nombre/info del calibre
        const int colWidth2 = tableWidth * 0.25;   // 25% para piezas
        const int colWidth3 = tableWidth * 0.25;   // 25% para porcentaje

        // Dibujar encabezados de columna
        QRect headerCol1(margin, y-20, colWidth1, rowHeight);
        QRect headerCol2(margin + colWidth1, y-20, colWidth2, rowHeight);
        QRect headerCol3(margin + colWidth1 + colWidth2, y-20, colWidth3, rowHeight);

        painter.drawText(headerCol1, Qt::AlignLeft | Qt::AlignVCenter, "Calibre");
        painter.drawText(headerCol2, Qt::AlignRight | Qt::AlignVCenter, "Piezas");
        painter.drawText(headerCol3, Qt::AlignRight | Qt::AlignVCenter, "Porcentaje");

        // Dibujar línea separadora
        painter.drawLine(margin, y, margin + tableWidth, y);
        y += 5;

        // Calcular estadísticas
        int total_pieces = 0;
        std::map<int, int> caliber_counts;
        caliber_counts.clear();
        
        // Primero contar el total de piezas excluyendo descarte
        for (const auto& entry : _history) {
            for (int caliber : entry.second) {
                if (caliber >= 0) { // Solo contar piezas válidas (no descartes)
                    caliber_counts[caliber]++;
                    total_pieces++; 
                }
            }
        }

        // Dibujar filas de la tabla - Calibres configurados
        for (int i = 0; i < _calibrationProgram->getNumCalibers(); i++) {
            if (y > printer.pageRect().height() - margin*2) {
                printer.newPage();
                y = margin;
            }

            const auto& cal = _calibrationProgram->getCalibers()[i];
            std::string caliber_info = cal.getName() + " (";
            
            // Añadir información de dimensiones
            for (size_t j = 0; j < cal.getDimensions().size(); ++j) {
                const auto& dim = cal.getDimensions()[j];
                if (j > 0) caliber_info += ", ";
                caliber_info += std::to_string(dim.getMinValue()) + "-" + 
                               std::to_string(dim.getMaxValue());
            }
            caliber_info += ")";

            // Obtener conteo y calcular porcentaje respecto al total (incluyendo descarte)
            int count = caliber_counts.count(i) ? caliber_counts[i] : 0;
            double percentage = (total_pieces > 0) ? (count * 100.0 / total_pieces) : 0.0;
            
            QRect col1(margin, y, colWidth1, rowHeight);
            QRect col2(margin + colWidth1, y, colWidth2, rowHeight);
            QRect col3(margin + colWidth1 + colWidth2, y, colWidth3, rowHeight);

            painter.drawText(col1, Qt::AlignLeft | Qt::AlignVCenter, 
                           QString::fromStdString(caliber_info));
            painter.drawText(col2, Qt::AlignRight | Qt::AlignVCenter, 
                           QString::number(count));
            painter.drawText(col3, Qt::AlignRight | Qt::AlignVCenter, 
                           QString::number(percentage, 'f', 1) + "%");

            y += rowHeight;
            painter.drawLine(margin, y, margin + tableWidth, y);
        }

        // Añadir fila de descarte si hay piezas descartadas
        if (caliber_counts.count(-1) > 0) {
            y += rowHeight/2;
            int count = caliber_counts[-1];
            double percentage = (total_pieces > 0) ? (count * 100.0 / total_pieces) : 0.0;

            QRect col1(margin, y, colWidth1, rowHeight);
            QRect col2(margin + colWidth1, y, colWidth2, rowHeight);
            QRect col3(margin + colWidth1 + colWidth2, y, colWidth3, rowHeight);

            painter.drawText(col1, Qt::AlignLeft | Qt::AlignVCenter, "Descarte");
            painter.drawText(col2, Qt::AlignRight | Qt::AlignVCenter, 
                           QString::number(count));
            painter.drawText(col3, Qt::AlignRight | Qt::AlignVCenter, 
                           QString::number(percentage, 'f', 1) + "%");

            y += rowHeight;
            painter.drawLine(margin, y, margin + tableWidth, y);
        }

        // Total
        y += rowHeight/2;
        QRect totalRow1(margin, y, colWidth1, rowHeight);
        QRect totalRow2(margin + colWidth1, y, colWidth2, rowHeight);
        QRect totalRow3(margin + colWidth1 + colWidth2, y, colWidth3, rowHeight);

        painter.setFont(QFont("Helvetica", 12, QFont::Bold));
        painter.drawText(totalRow1, Qt::AlignLeft | Qt::AlignVCenter, "Total");
        painter.drawText(totalRow2, Qt::AlignRight | Qt::AlignVCenter, 
                        QString::number(total_pieces));
        painter.drawText(totalRow3, Qt::AlignRight | Qt::AlignVCenter, "100.0%");

        painter.end();
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error generando PDF: " << e.what() << std::endl;
        painter.end();
        return false;
    }
}

//PRIVATE

std::string CaliberHistory::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count() % 1000;
    
    tm *ltm = localtime(&now_c);
    
    char buffer[18];  // 17 caracteres para yyyymmddhhmmsscc + 1 para null terminator
    snprintf(buffer, sizeof(buffer), "%04d%02d%02d%02d%02d%02d%02d",
        1900 + ltm->tm_year,  // año
        1 + ltm->tm_mon,      // mes
        ltm->tm_mday,         // día
        ltm->tm_hour,         // hora
        ltm->tm_min,          // minutos
        ltm->tm_sec,          // segundos
        (int)(ms/10));        // centésimas de segundo
    
    return std::string(buffer);
}

std::string CaliberHistory::getFormattedDateTime(time_t time) const {
    char buffer[20];
    tm* ltm = localtime(&time);
    strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M", ltm);
    return std::string(buffer);
}

/*
Nombre Partida:
Programa:
Fruta:
Variedad: 
Inicio partida:  2025/01/26 10:20
Find de partida: 
*/