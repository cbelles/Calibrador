#ifndef CALIBERHISTORY_H
#define CALIBERHISTORY_H

#include <vector>
#include <string>
#include <map>
#include <ctime>
#include "CalibrationProgram.h"
#include <QPrinter>
#include <QPainter>

using namespace std;

class CaliberHistory {
public:
    CaliberHistory();
    ~CaliberHistory();

    void setName(const string& partida) { _partida = partida; }
    void setCalibrationProgram(CalibrationProgram* program);

    void setIni();
    void setEnd();

    void addCaliberEntry(const std::vector<int>& calibers);
    void clear();

    // Nuevos métodos para serialización
    bool saveToFile(const std::string& filename);
    bool loadFromFile(const std::string& filename);

    // Nuevo método para exportar a JSON
    std::string toJSON() const;
    std::string calibersToJSON() const;  // Add this line

    bool exportToPDF(const std::string& filename) const;

private:

    std::string getCurrentTimestamp();
    string getFormattedDateTime(time_t time) const;

private:
    std::map<std::string, std::vector<int>> _history;  // timestamp -> vector calibres

    CalibrationProgram* _calibrationProgram;

    string _partida;
    string _fruit;
    string _variety;
    time_t _startTime;
    time_t _endTime;

};

#endif // CALIBERHISTORY_H