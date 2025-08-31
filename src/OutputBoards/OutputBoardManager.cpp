#include "OutputBoards/OutputBoardManager.h"
#include <iostream>

OutputBoardManager::OutputBoardManager(int numLines, int timeoutMs) 
    : _timeoutMs(30), _running(false) {  // 30ms = ~66.67ms/2 , 15 frutas/s   actua cada 66.67ms como maximo
    _boards.resize(numLines);
    _io_context = std::make_unique<boost::asio::io_context>();
    
    // Inicializar valores por defecto para cada placa
    for (size_t i = 0; i < numLines; ++i) {
        _boards[i].ip = "10.0.0." + std::to_string(150 + i);  // IPs: 10.0.0.150, 10.0.0.151, etc.
        _boards[i].port = 5000;  // Puerto por defecto
        _boards[i].connected = false;
    }
}

OutputBoardManager::~OutputBoardManager() {
    stop();
}

void OutputBoardManager::start() {
    if (_running) return;
    _running = true;
    
    // Inicializar sockets
    for (size_t i = 0; i < _boards.size(); ++i) {
        initializeSocket(_boards[i]);
    }

    // Iniciar thread de IO
    _io_thread = std::make_unique<std::thread>([this]() {
        while (_running) {
            _io_context->run_for(std::chrono::milliseconds(_timeoutMs));
            _io_context->restart();
        }
    });
}

void OutputBoardManager::stop() {
    _running = false;
    if (_io_thread && _io_thread->joinable()) {
        _io_thread->join();
    }
    for (auto& board : _boards) {
        if (board.socket) {
            board.socket->close();
        }
    }
}

bool OutputBoardManager::activateOutput(int lineIndex, int outputNumber) {
    if (outputNumber < 0 || outputNumber > 15) return false;
    uint16_t mask = 1 << outputNumber;
    return sendCommand(lineIndex, mask);
}

bool OutputBoardManager::deactivateOutput(int lineIndex, int outputNumber) {
    if (outputNumber < 0 || outputNumber > 15) return false;
    return sendCommand(lineIndex, 0);  // Send all outputs off (0)
}

bool OutputBoardManager::setMultipleOutputs(int lineIndex, const std::vector<int>& outputs) {
    uint16_t mask = 0;
    for (int output : outputs) {
        if (output >= 0 && output < 16) {
            mask |= (1 << output);
        }
    }
    return sendCommand(lineIndex, mask);
}


bool OutputBoardManager::isBoardConnected(int lineIndex) const {
    if (lineIndex >= 0 && lineIndex < _boards.size()) {
        return _boards[lineIndex].connected;
    }
    return false;
}

//PRIVATE

bool OutputBoardManager::initializeSocket(BoardConnection& board) {
    try {
        board.socket = std::make_unique<boost::asio::ip::udp::socket>(*_io_context);
        board.socket->open(boost::asio::ip::udp::v4());
        board.endpoint = boost::asio::ip::udp::endpoint(
            boost::asio::ip::address::from_string(board.ip), 
            board.port
        );
        board.connected = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing socket: " << e.what() << std::endl;
        board.connected = false;
        return false;
    }
}

bool OutputBoardManager::sendCommand(int lineIndex, uint16_t outputs) {
    if (lineIndex >= _boards.size() || !_boards[lineIndex].connected) {
        return false;
    }

    try {
        auto& board = _boards[lineIndex];
        
        // Create the 8-byte command packet
        uint8_t packet[8] = {
            0xC5,           // Start byte
            0x01,           // Command byte
            static_cast<uint8_t>(outputs >> 8),    // MSB
            static_cast<uint8_t>(outputs & 0xFF),  // LSB
            0x00, 0x00, 0x00,  // Padding
            0x5C            // End byte
        };

        board.socket->send_to(
            boost::asio::buffer(packet, sizeof(packet)), 
            board.endpoint
        );
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error sending command: " << e.what() << std::endl;
        return false;
    }
}

