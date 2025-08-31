#ifndef OUTPUTPOSITIONSDIALOG_H
#define OUTPUTPOSITIONSDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <vector>
#include <QString>

class OutputPositionsDialog : public QDialog {
    Q_OBJECT

public:
    explicit OutputPositionsDialog(int numOutputs, const std::vector<int>& positions, 
                                   const QString& configPath, QWidget* parent = nullptr);
    std::vector<int> getPositions() const;

private:
    void setupUI(int numOutputs, const std::vector<int>& positions);
    std::vector<int> loadPositions(int numOutputs) const;
    bool validatePositions() const;
    void updateSaveButtonState();
    QTableWidget* _table;
    QString _configPath;
    QPushButton* _saveButton;
    void savePositions();
};

#endif // OUTPUTPOSITIONSDIALOG_H