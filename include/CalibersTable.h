// CalibersTable.h
#ifndef CALIBERSTABLE_H
#define CALIBERSTABLE_H

#include <QTableWidget>
#include <QSpinBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include "CalibrationProgram.h"
#include "Utils/json.hpp"
#include <QTableWidgetItem>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include "FruitColorIndexs/ColorIndexBar.h"

class GroupHeaderView : public QHeaderView {
    Q_OBJECT
public:
    explicit GroupHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr);
    void setGroupedColumns(const QMap<QString, QPair<int, int>>& groups);

protected:
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;
    QSize sectionSizeFromContents(int logicalIndex) const override;

private:
    QMap<QString, QPair<int, int>> _groups;
};

class CalibersTable : public QTableWidget {
    Q_OBJECT

public:
    explicit CalibersTable(QWidget* parent = nullptr);
    
    void setCalibrationProgram(CalibrationProgram* program);
    void createTable();
    void updateTable();
    void updateHeaders();
    void addCaliber(const Caliber& caliber);  // Change to const reference
    int  removeCaliber();
    QScrollBar* verticalScrollBar() const { return QTableWidget::verticalScrollBar(); }

    // Añadir nuevo método público
    void setColorIndex(const std::string& colorindex);
    void setPartidaActiva(bool partida_activa); 
    bool isPartidaActiva() const { return _partida_activa; }

signals:
    void caliberChanged(int row, int dimensionIndex, int min, int max);
    void caliberNameChanged(int row, const QString& name);
    // Añadir nueva señal
    void rowSelected(int row);

private slots:
    void onCellChanged(int row, int column);
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    void setupTable();
    void swapRows(int row1, int row2);
    void addSpinBoxToCell(int row, int col, int value);
    QColor getColorFromIndex(unsigned char index); // Nueva función auxiliar

    GroupHeaderView* _horizontalHeader;
    CalibrationProgram* _calibrationProgram;

    ColorIndexBar _colorIndexBar; 
    bool _partida_activa = false;
};

class CaliberNameDelegate : public QStyledItemDelegate {
public:
    CaliberNameDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                         const QModelIndex& index) const override {
        QLineEdit* editor = qobject_cast<QLineEdit*>(
            QStyledItemDelegate::createEditor(parent, option, index));
        if (editor) {
            editor->setMaxLength(17);
        }
        return editor;
    }
};

#endif // CALIBERSTABLE_H