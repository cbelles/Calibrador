#ifndef SALIDASTABLE_H
#define SALIDASTABLE_H

#include <QTableWidget>
#include <QSpinBox>
#include "CalibrationProgram.h"
#include <QHeaderView>
#include <QMap>
#include <QString>
#include <QPair>
#include "Cameras/CameraManager.h"

class SalidaHeaderView : public QHeaderView {
    Q_OBJECT

public:
    explicit SalidaHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr);
    void setHeaders(const QStringList& headers);

protected:
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;
    QSize sectionSizeFromContents(int logicalIndex) const override;

private:
    QStringList _headers;
};

class SalidasTable : public QTableWidget {
    Q_OBJECT

public:
    explicit SalidasTable(QWidget *parent = nullptr);
    void setCalibrationProgram(CalibrationProgram *program);
    void setNumSalidas(int num);
    void addRow();
    void removeRow(int row);
    void updateTable(); 
    std::vector<std::vector<int>> getInfo() const;  // Modificar esta línea
    QScrollBar* verticalScrollBar() const { return QTableWidget::verticalScrollBar(); }
    void setCameraManager(CameraManager* manager) { _cameraManager = manager; }
    void setPartidaActiva(bool partida_activa) { _partida_activa = partida_activa; }
    bool isPartidaActiva() const { return _partida_activa; }

public slots:
    void highlightRow(int row);  

private:
    void setupTable();
    void updateHeaders();
    void addSpinBoxToCell(int row, int col, int value = 0);

private slots:
    void onCellChanged(int row, int column);

private:
    CalibrationProgram* _calibrationProgram;
    SalidaHeaderView* _horizontalHeader;  
    int _numSalidas;
    CameraManager* _cameraManager = nullptr;

    static const int COLUMN_WIDTH = 55; 
    static const int MAX_COLUMNS = 16;  

    bool _partida_activa = false;
};

#endif // SALIDASTABLE_H