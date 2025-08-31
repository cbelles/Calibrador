#ifndef LEDSTRIP_H
#define LEDSTRIP_H

#include <QWidget>
#include <QColor>
#include <vector>

class LedStrip : public QWidget
{
    Q_OBJECT

public:
    //enum LedColor {
    //    OFF,    // Gris
    //    RED,    // Rojo
    //    GREEN,  // Verde
    //    YELLOW, // Amarillo
    //    WHITE   // Blanco
    //};

    explicit LedStrip(QWidget *parent = nullptr, int numLeds = 8);
    
    void setLedColor(int index, int color);
    void setAllLedsColor(int color);
    int getLedColor(int index) const;
    int getNumLeds() const { return _numLeds; }
    void setLedSize(int size) { _ledSize = size; update(); }

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;

private:
    int _numLeds;
    int _ledSize = LED_SIZE;  // Nueva variable para tamaño personalizado
    static const int LED_SIZE = 20;
    static const int LED_SPACING = 5;
    std::vector<int> _ledStates;

    QColor getColorFromState(int state) const;
};

#endif // LEDSTRIP_H