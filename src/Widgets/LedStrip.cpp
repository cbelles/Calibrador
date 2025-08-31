#include "Widgets/LedStrip.h"
#include <QPainter>

LedStrip::LedStrip(QWidget *parent, int numLeds) 
    : QWidget(parent)
    , _numLeds(numLeds)
    , _ledStates(numLeds, 0)   //OFF = 0
{
    setMinimumSize(sizeHint());
}

void LedStrip::setLedColor(int index, int color)
{
    if (index >= 0 && index < _numLeds) {
        _ledStates[index] = color;
        update();
    }
}

void LedStrip::setAllLedsColor(int color)
{
    for (int i = 0; i < _numLeds; ++i) {
        setLedColor(i, color);
    }
}

int LedStrip::getLedColor(int index) const
{
    if (index >= 0 && index < _numLeds) {
        return _ledStates[index];
    }
    return 0;  //OFF = 0
}

QColor LedStrip::getColorFromState(int state) const
{
    switch (state) {
        case 1:     return QColor(255, 0, 0);
        case 2:     return QColor(0, 255, 0);
        case 3:     return QColor(255, 255, 0);
        case 4:     return QColor(255, 255, 255);
        default:    return QColor(128, 128, 128);
    }
}

void LedStrip::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (int i = 0; i < _numLeds; ++i) {
        int x = i * (_ledSize + LED_SPACING) + LED_SPACING;
        int y = LED_SPACING;

        // Dibujar el borde del LED
        painter.setPen(Qt::black);
        painter.setBrush(getColorFromState(_ledStates[i]));
        painter.drawEllipse(x, y, _ledSize, _ledSize);

        // Efecto de brillo
        if (_ledStates[i] != 0) {   //OFF = 0
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(255, 255, 255, 70));
            painter.drawEllipse(x + _ledSize/4, y + _ledSize/4, 
                              _ledSize/4, _ledSize/4);
        }
    }
}

QSize LedStrip::sizeHint() const
{
    return QSize(_numLeds * (_ledSize + LED_SPACING) + LED_SPACING,
                 _ledSize + 2 * LED_SPACING);
}