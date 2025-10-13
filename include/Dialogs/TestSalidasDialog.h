#ifndef TESTSALIDASDIALOG_H
#define TESTSALIDASDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <QLabel>
#include <QCloseEvent>
#include <vector>
#include "OutputBoards/OutputBoardManager.h"

class TestSalidasDialog : public QDialog {
    Q_OBJECT

public:
    explicit TestSalidasDialog(int numOutputs, int numLines, OutputBoardManager* outputBoardManager, 
                              QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onTestAllOutputs();
    void onTimerTick();
    void onCloseRequested();

private:
    void setupUI(int numOutputs, int numLines);
    void activateOutput(int lineIndex, int outputIndex);
    void deactivateOutput(int lineIndex, int outputIndex);
    void activateOutputInAllLines(int outputIndex);
    void deactivateOutputInAllLines(int outputIndex);

private:
    QTableWidget* _table;
    QPushButton* _testAllButton;
    QLabel* _statusLabel;
    
    OutputBoardManager* _outputBoardManager;
    int _numOutputs;
    int _numLines;
    
    QTimer* _testTimer;
    bool _testRunning;
    bool _currentState; // true = todas ON, false = todas OFF
};

#endif // TESTSALIDASDIALOG_H