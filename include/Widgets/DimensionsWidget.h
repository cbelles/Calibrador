#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLCDNumber>
#include <QLabel>
#include <QComboBox>
#include "CalibrationProgram.h"
//#include "Cameras/Mensaje.h"

class DimensionsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DimensionsWidget(QWidget *parent = nullptr);

    void reset();
    
    void setCalibrationProgram(CalibrationProgram* program);

    // Setters para los valores
    void setXDimension(int value);
    void setYDimension(int value);
    void setZDimension(int value);

    // Setters para los dominios
    void setXDomain(int maxValue);
    void setYDomain(int maxValue);
    void setZDomain(int maxValue);

    // Getters para los dominios
    int getXMaxDomain() const { return xMaxDomain; }
    int getYMaxDomain() const { return yMaxDomain; }
    int getZMaxDomain() const { return zMaxDomain; }

    // Setters para los nombres de dimensiones
    void setXName(const std::string& name);
    void setYName(const std::string& name);
    void setZName(const std::string& name);

    void update();

    // Estructura para mapear dimensiones a campos del mensaje
    struct DimensionMapping {
        std::string programDimension;  // Nombre en el programa (ej: "COLOR", "AREA", etc)
        std::string messageField;      // Nombre en el mensaje JSON
    };

    // Nuevo método para actualizar desde mensaje
    //void updateFromMessage(const Mensaje& msg);
    void updateDimensions(vector<int> dimensions);

    void setNumLines(int numLines);
    int getCurrentLine() const { return currentLine; }

    void clear();  // Añadir esta declaración

private slots:
    void onLineChanged(int index);

private:
    void setupDisplays();
    QLCDNumber* createCounter(int numDigits, int decimals);
    QLabel* createLabel(const QString &text);
    int validateValue(int value, int maxDomain) const;

    // Mapa de dimensiones a campos del mensaje
    static const std::vector<DimensionMapping> _dimensionMappings;
    
    // Helper para obtener el campo del mensaje correspondiente a una dimensión
    std::string getMessageField(const std::string& dimension) const;
    
    // Helper para actualizar una dimensión específica
    //void updateDimension(const std::string& dim, const Mensaje& msg, void (DimensionsWidget::*setDimension)(int));
    
    // Helper para actualizar el color del contador si es dimensión COLOR
    void updateCounterColor(QLCDNumber* counter, const std::string& dimension, int value);

private:

    CalibrationProgram* _calibrationProgram;

    QVBoxLayout* mainLayout;
    QLCDNumber* xCounter;
    QLCDNumber* yCounter;
    QLCDNumber* zCounter;

    // Dominios para cada dimensión
    int xMaxDomain;
    int yMaxDomain;
    int zMaxDomain;

    // Labels para los nombres de las dimensiones
    QLabel* xLabel;
    QLabel* yLabel;
    QLabel* zLabel;

    QComboBox* lineSelector;
    int currentLine;
};