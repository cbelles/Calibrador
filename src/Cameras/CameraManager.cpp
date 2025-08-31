#include "Cameras/CameraManager.h"
#include <chrono>
#include <iostream>
#include <fstream>  // Añadir este include para std::ofstream
#include <future>
#include <thread>

using boost::asio::ip::tcp;

CameraManager::CameraManager(int numCameras, const string& possalidasfile, int receiveTimeoutMs, int connectionTimeoutMs)
    : _connectionTimeoutMs(connectionTimeoutMs), _receiveTimeoutMs(receiveTimeoutMs), _running(false),
      _io_context(std::make_unique<boost::asio::io_context>()) // Initialize shared io_context
{
    _possalidasfile = possalidasfile;
    // Inicializar las conexiones de cámara
    for (int i = 0; i < numCameras; ++i)
    {
        CameraConnection camera;
        camera.ip = _baseIP + std::to_string(i + 1);
        camera.connected = false;
        camera.responded = false;
        camera.socket = std::make_unique<tcp::socket>(*_io_context); // Use shared io_context
        camera.lastAttemptTime = std::chrono::steady_clock::now(); // Initialize last attempt time

        _cameras.push_back(std::move(camera));
    }

    _fruta = "";
    _variedad = "";
    _idpulso = 0;
    setPort(3000);
    _calibres.resize(numCameras, -1); // Inicializar el vector de calibres

    // Start the io_context thread
    _io_thread = std::make_unique<std::thread>(
        [this]()
        {
            _io_context->run();
        });

    _expulsionmanager.resize(numCameras);
    for (int i = 0; i < numCameras; i++)
        _expulsionmanager[i].loadPositionsFromFile(_possalidasfile);

    _activaciones_salidas.resize(numCameras); // Inicializar el vector con el número de líneas
}

CameraManager::~CameraManager()
{
    stop();

    // Detener el io_context y su thread
    if (_io_context)
    {
        _io_context->stop();
    }
    if (_io_thread && _io_thread->joinable())
    {
        _io_thread->join();
    }
}

void CameraManager::start()
{
    if (!_running)
    {
        _running = true;
        _thread = std::make_unique<std::thread>(&CameraManager::run, this);
    }
}

void CameraManager::stop()
{
    if (_running)
    {
        _running = false;
        if (_thread && _thread->joinable())
        {
            _thread->join();
        }

        // Cerrar todos los sockets
        for (auto &camera : _cameras)
        {
            if (camera.socket->is_open())
            {
                boost::system::error_code ec;
                camera.socket->close(ec);
            }
        }
    }

    // Stop the io_context
    if (_io_context)
    {
        _io_context->stop();
    }
}

void CameraManager::run()
{
    for (int i = 0; i < _cameras.size(); i++)
    {
        auto &camera = _cameras[i];
        camera.connected = false;
    }

    while (_running)
    {
        if (_debugMode)
        {            
            // Procesar en modo debug
            auto start = std::chrono::steady_clock::now();
            cout << "======================================" << endl;
            sendCaptureSignal();
            _idpulso++;
            
            auto end = std::chrono::steady_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            cout << "Tiempo total de proceso (debug): " << ms << " ms" << endl;

            // Esperar el tiempo restante para completar el ciclo
            if (ms < _tiempoCiclo) {
                std::this_thread::sleep_for(std::chrono::milliseconds(_tiempoCiclo - ms));
            }
        }
        else
        {
            if (!_fotocelula->waitForDetection(SENSOR_TIMEOUT_MS)) {
                continue;
            }

            // Procesar inmediatamente la detección
            auto start = std::chrono::steady_clock::now();
            
            // Bloquear nuevas detecciones mientras procesamos esta
            _fotocelula->stop();
            
            // Enviar señales y procesar
            cout << "=====================================" << endl;
            sendCaptureSignal();
            
            // Incrementar ID de pulso
            _idpulso++;
            
            // Reiniciar detección para el siguiente ciclo
            _fotocelula->start();
            
            auto end = std::chrono::steady_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            cout << "Tiempo total de proceso: " << ms << " ms" << endl;
        }
    }
}

void CameraManager::setDebugMode(bool debugMode)
{
    _debugMode = debugMode;
}

void CameraManager::notifySensorActivation()
{
    std::unique_lock<std::mutex> lock(_sensorMutex);
    _sensorActivated = true;
    _sensorCv.notify_all();
}

bool CameraManager::connectCamera(CameraConnection &camera)
{
    // Skip connection attempt if we tried recently
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(
            now - camera.lastAttemptTime).count() < 1000) {
        return false;
    }
    
    camera.lastAttemptTime = now;
    
    try {
        // Close existing socket if open
        if (camera.socket->is_open()) {
            boost::system::error_code ec;
            camera.socket->close(ec);
        }

        // Create new socket
        camera.socket = std::make_unique<tcp::socket>(*_io_context);
        boost::system::error_code ec;

        // Configure socket
        camera.socket->open(tcp::v4(), ec);
        if (ec) return false;

        // Set non-blocking mode
        camera.socket->non_blocking(true, ec);
        if (ec) return false;

        // Set socket options for quick timeout
        boost::asio::socket_base::linger linger(true, 0);
        camera.socket->set_option(linger, ec);
        camera.socket->set_option(boost::asio::ip::tcp::no_delay(true), ec);
        
        // Start connection
        tcp::endpoint endpoint(boost::asio::ip::address::from_string(camera.ip), _port);
        camera.socket->connect(endpoint, ec);  //OJO: ES AQUI DONDE ESTA 2000ms si NO HAY PUERTO   

        if (ec && ec != boost::asio::error::would_block) {
            return false;
        }

        // Quick connection check with minimal timeout
        auto start = std::chrono::steady_clock::now();
        auto timeout = std::chrono::milliseconds(200); // Very aggressive timeout

        while (std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start) < timeout) {
            boost::system::error_code check_ec;
            camera.socket->remote_endpoint(check_ec);

            if (!check_ec) {
                camera.socket->non_blocking(false);
                camera.fullyConnected = true;
                return true;
            }
        }

        // Cleanup on timeout
        camera.socket->close(ec);
        camera.fullyConnected = false;
        return false;
    }
    catch (...) {
        camera.fullyConnected = false;
        return false;
    }
}

//=========================================================================
bool CameraManager::sendCaptureSignal()
{
    _dimensiones.resize(_cameras.size());
    _hayfruta.resize(_cameras.size(), false); // Inicializar el vector que indica si hay fruta
    for (int i=0 ; i < _cameras.size(); i++)
        _dimensiones[i].resize(3,0);

    std::vector<std::future<bool>> futures;
    bool allSuccess = true;

    // Iniciar capturas en paralelo
    for (int i = 0; i < _cameras.size(); i++)
    {
        futures.push_back(std::async(std::launch::async,
                                     &CameraManager::handleSingleCamera, this, i));
    }

    // Esperar y recoger resultados
    for (auto &fut : futures)
    {
        try
        {
            bool success = fut.get(); // Usar get() en lugar de wait()
            allSuccess &= success;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error en captura: " << e.what() << std::endl;
            allSuccess = false;
        }
    }

    // _ledsCamaras  -> Emitir señal
    for (int i = 0; i < _cameras.size(); i++)
        emit updateCameraLEDsRequested(i, _cameras[i].connected ? 2 : 1);  //RED = 1, GREEN = 2
    
    // _ledsSalidas -> Emitir señal
    std::vector<int> outputs = _outputmanager.processCalibres(_calibres);  //Calibre actual en cada linea
    emit updateOutputLEDsRequested(outputs);


    //print the outputs
    cout << "Outputs activados: ";
    for (const auto &output : outputs)
    {
        cout << output << " ";
    }
    cout << endl;

    // _counterWidget->incNumPieces(...) -> Emitir señal
    emit updateCounterRequested(_calibres);

    // _dimensionsWidget  -> Emitir señal
    emit updateDimensionsRequested(_dimensiones, _hayfruta);

    //Guardar informacion sobre el calibrado en el historial de la partida _caliberhistory
    _caliberhistory.addCaliberEntry(_calibres);

    //Ejecucion de la cola de expulsiones
    _activaciones_salidas.clear(); 
    for (int i = 0; i < _cameras.size(); i++)
    {
        _expulsionmanager[i].tick();
        _expulsionmanager[i].scheduleExpulsion(outputs[i]);

        vector<int> active = _expulsionmanager[i].getActiveOutputs();
        _activaciones_salidas.push_back(active);
    }
    //Actuar sobre las tarjetas de salidas llamando a la funcion setMultipleOutputs
    for (int i = 0; i < _cameras.size(); i++)
    {
        if (_outputBoardManager->isBoardConnected(i))
        {
            cout << "Activando salidas en linea " << i + 1 << ": ";
            for (auto out : _activaciones_salidas[i])
                cout << out << " ";
            cout << endl;
            
            // Only activate outputs if there's an active session
            if (_partida_activa)
            {
                _outputBoardManager->setMultipleOutputs(i, _activaciones_salidas[i]);
            }
        }
        else
        {
            cout << "Tarjeta de salidas no conectada en linea " << i + 1 << endl;
        }
    }

    return allSuccess;
}

void CameraManager::savePartidatoJson(const string& partidaname)
{
    // Crear un nuevo thread para el guardado
    std::thread saveThread(&CameraManager::saveCaliberHistoryWorker, this, partidaname);
    saveThread.detach();  // Desacoplar el thread para que se ejecute independientemente
}

void CameraManager::savePartidatoPDF(const string& partidaname)
{
    // Crear un nuevo thread para el guardado
    std::thread saveThread(&CameraManager::saveCaliberHistoryPDFWorker, this, partidaname);
    saveThread.detach();  // Desacoplar el thread para que se ejecute independientemente
}

bool CameraManager::handleSingleCamera(int index)
{
    if (index >= _cameras.size())
        return false;

    auto &camera = _cameras[index];
    _hayfruta[index] = false; // Por defecto, no hay fruta

    try
    {
        // Add timeout for the entire camera handling process
        auto start = std::chrono::steady_clock::now();
        auto timeout = std::chrono::milliseconds(100); // Total timeout for this camera

        if (!camera.connected || !camera.fullyConnected)
        {
            camera.connected = connectCamera(camera);
            if (!camera.connected || !camera.fullyConnected)
            {
                //tiempo desde start
                auto end = std::chrono::steady_clock::now();
                auto diff = end - start;
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();

                cout << "Camera " << index << " INTENTO DE CONEXION FALLA CLARO " <<  ms << endl;
                return false;
            }
        }

        // Check if we've exceeded our time budget
        if (std::chrono::steady_clock::now() - start > timeout) {
            return false;
        }

        // Limpiar el buffer antes de enviar
        clearSocketBuffer(camera);

        // Construir y enviar el mensaje de captura
        string packet = build_pulso(_fruta, _variedad, _idpulso);
        auto sendTime = std::chrono::steady_clock::now();
        
        if (!sendToCamera(camera, packet))
        {
            camera.connected = false;
            camera.lastAttemptTime = std::chrono::steady_clock::now(); // Update last attempt time
            return false;
        }

        // Recibir y procesar la respuesta
        string response;
        if (!receiveFromCamera(camera, response))
        {
            camera.connected = false;
            camera.lastAttemptTime = std::chrono::steady_clock::now(); // Update last attempt time
            return false;
        }

        auto receiveTime = std::chrono::steady_clock::now();
        auto processingTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            receiveTime - sendTime).count();
            
        cout << "Camera " << index << " processing time: " << processingTime << "ms" << endl;

        // Procesar la respuesta
        try
        {
            vector<int> dimensiones;
            Mensaje msg;
            msg.parse(response);

            cout << "SINCRONISMO PULSO " << _idpulso << " = " << msg.getTimeStampIn() <<endl;

            int msg_calibre =-1;
            if (msg.getPieza()) {
                msg_calibre = _classifier.classify(msg);
                _hayfruta[index] = true; // Marcar que hay fruta en esta cámara
            }
            _calibres[index] = msg_calibre;
            
            cout << "En linea : " << index + 1 << " calibre =  " << msg_calibre << endl;

            dimensiones =  getDimensionsFromMessage(msg);
            //print dimensions
            //cout << "En linea : " << index + 1 << " dimensions= ";
            //for (auto dim : dimensiones)
            //    cout << dim << " ";
            //cout << endl;

            _dimensiones[index] = dimensiones;
 
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error procesando mensaje: " << e.what() << std::endl;
            return false;
        }

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error en cámara " << index << ": " << e.what() << std::endl;
        camera.connected = false;
        camera.fullyConnected = false;
        camera.lastAttemptTime = std::chrono::steady_clock::now(); // Update last attempt time
        return false;
    }
}
//=========================================================================

bool CameraManager::sendToCamera(CameraConnection &camera, const string &packet)
{
    try
    {
        // cout << "Sending to camera at " << camera.ip << ": " << packet << std::endl;
        boost::asio::write(*camera.socket, boost::asio::buffer(packet.data(), packet.size()));
        return true;
    }
    catch (const boost::system::system_error &e)
    {
        std::cerr << "Error sending to camera at " << camera.ip
                  << ": " << e.what() << std::endl;
        camera.connected = false;
        return false;
    }
}

bool CameraManager::receiveFromCamera(CameraConnection &camera, string &response)
{
    try
    {
        camera.socket->non_blocking(true);
        boost::asio::streambuf buf;
        boost::system::error_code ec;

        // Timer para el timeout
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(_receiveTimeoutMs);

        size_t n = 0;
        while (n == 0)
        {
            if (std::chrono::steady_clock::now() > deadline)
            {
                camera.socket->cancel();
                std::cerr << "Timeout receiving from camera at " << camera.ip << std::endl;
                camera.socket->non_blocking(false);
                return false;
            }

            n = boost::asio::read_until(*camera.socket, buf, '#', ec);

            if (ec == boost::asio::error::would_block)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                ec.clear();
                continue;
            }
            else if (ec)
            {
                camera.socket->non_blocking(false);
                throw boost::system::system_error(ec);
            }
        }

        // Extraer solo el primer mensaje completo (hasta el primer '#')
        std::istream is(&buf);
        std::getline(is, response, '#');

        // Limpiar cualquier dato residual del socket
        camera.socket->non_blocking(false);

        // Vaciar el buffer del socket
        try
        {
            boost::asio::socket_base::bytes_readable command(true);
            camera.socket->io_control(command);
            std::size_t bytes_readable = command.get();
            if (bytes_readable > 0)
            {
                std::vector<char> temp_buffer(bytes_readable);
                camera.socket->read_some(boost::asio::buffer(temp_buffer));
            }
        }
        catch (...)
        {
            // Ignorar errores al limpiar el buffer
        }

        return true;
    }
    catch (const boost::system::system_error &e)
    {
        std::cerr << "Error receiving from camera at " << camera.ip
                  << ": " << e.what() << std::endl;
        return false;
    }
}

bool CameraManager::isCameraConnected(int cameraIndex) const
{
    if (cameraIndex >= 0 && cameraIndex < _cameras.size())
    {
        return _cameras[cameraIndex].connected;
    }
    return false;
}

bool CameraManager::hasCameraResponded(int cameraIndex) const
{
    if (cameraIndex >= 0 && cameraIndex < _cameras.size())
    {
        return _cameras[cameraIndex].responded;
    }
    return false;
}

const string CameraManager::build_pulso(const string &fruta, const string &variedad, long long idpulso) const
{
    // Eliminar el & para devolver un nuevo string en lugar de una referencia
    string uni = "";
    if (!variedad.empty())
        uni = "&";
    string quefruta = fruta + uni + variedad;
    if (!quefruta.empty())
        quefruta = quefruta + "_";

    return "PULSO_" + quefruta + to_string(idpulso) + "#";
}

std::vector<int> CameraManager::getDimensionsFromMessage(const Mensaje& msg) const {
    std::vector<int> dimensions(3, 0); // Initialize vector with 3 zeros
    int value;
    if (_calibrationProgram) {
        for (int i = 0; i < 3; ++i) {
            value =0;
            std::string dimensionName = _calibrationProgram->getDimension(i);
            if (dimensionName == "COLOR") 
                value = msg.getColor();
            if (dimensionName == "AREA") 
                value = msg.getArea(); 
            if (dimensionName == "DIAMETER_MIN") 
                value = msg.getDiMenor();
            if (dimensionName == "DIAMETER_MAX") 
                value = msg.getDiMayor();
            dimensions[i] = value;
        }
    }

    return dimensions;
}

//PRIVATE

void CameraManager::clearSocketBuffer(CameraConnection &camera) {
    try {
        camera.socket->non_blocking(true);
        std::vector<char> temp_buffer(1024);
        boost::system::error_code ec;
        
        while (true) {
            size_t bytes = camera.socket->read_some(boost::asio::buffer(temp_buffer), ec);
            if (ec == boost::asio::error::would_block || bytes == 0) {
                break;
            }
        }
        
        camera.socket->non_blocking(false);
    } catch (...) {
        // Ignorar errores al limpiar el buffer
    }
}

void CameraManager::saveCaliberHistoryWorker(const string& partidaname) {
    _caliberhistory.setName(partidaname);
    _caliberhistory.setCalibrationProgram(_calibrationProgram);

    // Generar el nombre del archivo
    string filename = _pathPartidas + "/" + partidaname + ".json";
    
    // Convertir a JSON y guardar
    
    //string jsonData = _caliberhistory.toJSON();
    string jsonData = _caliberhistory.calibersToJSON();  // Use calibersToJSON instead of toJSON
    
    std::ofstream file(filename);
    file << jsonData;
    
    // Limpiar el historial después de guardar
    _caliberhistory.clear();
}

void CameraManager::saveCaliberHistoryPDFWorker(const string& partidaname)
{
    // Generar el nombre del archivo
    string filename = _pathPartidas + "/" + partidaname + ".pdf";
    
    // Guardar en PDF
    if (!_caliberhistory.exportToPDF(filename))
    {
        cout << "Error al guardar el historial en PDF" << std::endl;
        exit(0);
    }
}

