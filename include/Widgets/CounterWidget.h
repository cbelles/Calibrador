// counterwidget.h
#ifndef COUNTERWIDGET_H
#define COUNTERWIDGET_H


#include <vector>
#include <QWidget>
#include <QLCDNumber>
#include <QVBoxLayout>
#include <QLabel>
#include <chrono>
#include <deque>

using namespace std;

class CounterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CounterWidget(QWidget *parent = nullptr);
    void setNumLines(size_t lines) { _numLines = lines; }

    void reset();
    void incNumPieces(const vector<int>& calibres);
    
    void updatePieces();   
    void updateCupletsPerSecond();
    void updateFruitsPerSecond();

private: 

    void setCupletesPerSecond(double count);
    void setFruitsPerSecond(double count);
    double calculateCupletsPerSecond();
    double calculateFruitsPerSecond();

    // Configuración de los displays
    void setupDisplays();
    QLCDNumber* createCounter(int numDigits, int decimals = 0);
    QLabel* createLabel(const QString &text);

private:
    QLCDNumber *piecesCounter;
    QLCDNumber *cupletsCounter;
    QLCDNumber *fruitsCounter;
    
    // Layout principal
    QVBoxLayout *mainLayout;

    int _numpieces;

    // Add timing related members
    struct CallData {
        std::chrono::steady_clock::time_point timestamp;
        int validPieces;
    };

    std::deque<CallData> _callHistory;
    const int WINDOW_SIZE_MS = 4000; // 4 second window
    const size_t MAX_HISTORY = 100;  // Prevent unbounded growth
    size_t _numLines;  // Number of calibrator lines

};

#endif // COUNTERWIDGET_H
