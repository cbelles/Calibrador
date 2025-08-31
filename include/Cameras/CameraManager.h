#pragma once

#include <thread>
#include <vector>
#include <memory>
#include <atomic>
#include <string>
#include <boost/asio.hpp>
#include "Mensaje.h"
#include "CaliberClassifier.h"
#include "CaliberHistory.h"
#include "OutputManager.h"
#include "ExpulsionManager.h"
#include "Widgets/LedStrip.h"
#include "Widgets/DimensionsWidget.h"
#include "Widgets/CounterWidget.h"
#include <future>
#include <chrono> 
#include <QObject>  
#include "Fotocelula.h"  
#include "OutputBoards/OutputBoardManager.h"

using namespace std;

class LedStrip;  // Forward declaration

class CameraManager : public QObject {  // Add QObject inheritance
    Q_OBJECT  // Add Q_OBJECT macro

public:
    struct CameraConnection {
        std::unique_ptr<boost::asio::ip::tcp::socket> socket;
        std::string ip;
        bool connected;
        bool responded;
        bool fullyConnected; // New flag to track complete connection state
        std::chrono::steady_clock::time_point lastAttemptTime; // Add last attempt time
    };

    // Modificar el constructor para aceptar dos timeouts diferentes
    explicit CameraManager(int numCameras, const string& possalidasfile, int receiveTimeoutMs = 200, int connectionTimeoutMs = 50);
    ~CameraManager();

    void setTiempoCiclo(int tiempoCiclo) { _tiempoCiclo = tiempoCiclo; }  

    // Iniciar/detener el thread
    void start();
    void stop();

    // Enviar señal de captura a las cámaras
    bool sendCaptureSignal();

    // Obtener estado de las cámaras
    bool isCameraConnected(int cameraIndex) const;
    bool hasCameraResponded(int cameraIndex) const;

    void setPosSalidaFile(const string& possalidasfile) { _possalidasfile = possalidasfile; }

    void setFruta(const string& fruta) { _fruta = fruta; }
    void setVariedad(const string& variedad) { _variedad = variedad; }
    const string& getFruta() const { return _fruta; }
    const string& getVariedad() const { return _variedad; }

    // Añadir configuración de puerto
    void setPort(int port) { _port = port; }

    // Añadir método para establecer el programa
    void setCalibrationProgram(CalibrationProgram* program) { 
        _calibrationProgram = program;
        _classifier.setCalibrationProgram(program);
        _outputmanager.setCalibrationProgram(program);
    }

    std::vector<int> getDimensionsFromMessage(const Mensaje& msg) const;

    OutputManager* getOutputManager() { return &_outputmanager; }
    CaliberHistory* getCaliberHistory() { return &_caliberhistory; }

    void savePartidatoJson(const string& partidaname); 
    void savePartidatoPDF(const string& partidaname);

    void setPathPartidas(const string& path) { _pathPartidas = path; } 

    void setDebugMode(bool debugMode);
    void notifySensorActivation();

    void setFotocelula(Fotocelula* fotocelula) { _fotocelula = fotocelula; }
    void setOutputBoardManager(OutputBoardManager* outputBoardManager) { _outputBoardManager = outputBoardManager; }

    void setPartidaActiva(bool activa) { _partida_activa = activa; }
    bool isPartidaActiva() const { return _partida_activa; }

signals:  // Add signals declaration
    void updateCameraLEDsRequested(int index, int color);
    void updateOutputLEDsRequested(const std::vector<int>& outputs); // Modified
    void updateCounterRequested(const std::vector<int>& calibres);
    void updateDimensionsRequested(const vector<vector<int>>& dimensiones, const vector<bool>& hayfruta);

private:
    // Thread principal
    void run();
    
    bool connectCamera(CameraConnection& camera);
    
    // Métodos de comunicación
    bool sendToCamera(CameraConnection& camera, const string& packet);
    bool receiveFromCamera(CameraConnection& camera, string& response);  // Cambiar tipo

    const string build_pulso(const string& fruta, const string& variedad, long long idpulso) const;

    bool pingCamera(const std::string& ip) const;

    // Añadir método helper para manejar una cámara individual
    bool handleSingleCamera(int index);

    void clearSocketBuffer(CameraConnection &camera);

    void saveCaliberHistoryWorker(const string& partidaname);
    void saveCaliberHistoryPDFWorker(const string& partidaname);

private:
    std::vector<CameraConnection> _cameras;
    std::unique_ptr<std::thread> _thread;
    std::atomic<bool> _running;
    int _connectionTimeoutMs;  // Timeout para establecer conexión
    int _receiveTimeoutMs;    // Timeout para recibir datos
    const std::string _baseIP = "169.254.1.";
    int _port; 

    string _fruta;
    string _variedad;
    long long _idpulso;  
     
    CalibrationProgram* _calibrationProgram = nullptr;
    CaliberClassifier _classifier;
    OutputManager _outputmanager;
    CaliberHistory _caliberhistory;
    vector<ExpulsionManager> _expulsionmanager;

    std::vector<int> _calibres;
    vector<vector<int>> _dimensiones;
    vector<bool> _hayfruta;  

    vector<vector<int>> _activaciones_salidas; // Vector para almacenar las activaciones de salidas
     
    std::unique_ptr<boost::asio::io_context> _io_context; // Shared io_context
    std::unique_ptr<std::thread> _io_thread; // Thread for shared io_context

    string _pathPartidas;
    string _possalidasfile;

    std::atomic<bool> _debugMode{true}; // Default to debug mode
    std::atomic<bool> _sensorActivated{false};
    std::mutex _sensorMutex;
    std::condition_variable _sensorCv;

    static constexpr int SENSOR_CYCLE_MS = 100;           // Tiempo normal entre detecciones
    static constexpr int SENSOR_TIMEOUT_MS = 500;         // 5x el ciclo normal

    Fotocelula* _fotocelula = nullptr;  
    OutputBoardManager* _outputBoardManager = nullptr;
    
    int _tiempoCiclo = 200;  // Tiempo de ciclo en modo debug, por defecto 200 ms

    std::atomic<bool> _partida_activa{false}; // Control if outputs should be activated

};