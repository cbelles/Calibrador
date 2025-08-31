#pragma once

#include <vector>
#include <memory>
#include <atomic>
#include <string>
#include <boost/asio.hpp>
#include <QObject>

class OutputBoardManager : public QObject {
    Q_OBJECT

public:
    struct BoardConnection {
        std::unique_ptr<boost::asio::ip::udp::socket> socket;
        boost::asio::ip::udp::endpoint endpoint;
        std::string ip;
        int port;
        bool connected;
    };

    explicit OutputBoardManager(int numLines, int timeoutMs = 100);
    ~OutputBoardManager();

    void start();
    void stop();

    // Métodos principales
    bool activateOutput(int lineIndex, int outputNumber);
    bool deactivateOutput(int lineIndex, int outputNumber);
    bool setMultipleOutputs(int lineIndex, const std::vector<int>& outputs);
    
    // Configuración
    bool isBoardConnected(int lineIndex) const;

private:
    bool initializeSocket(BoardConnection& board);
    bool sendCommand(int lineIndex, uint16_t outputs);
    void run();

private:
    std::vector<BoardConnection> _boards;
    std::unique_ptr<std::thread> _thread;
    std::atomic<bool> _running;
    int _timeoutMs;
    std::unique_ptr<boost::asio::io_context> _io_context;
    std::unique_ptr<std::thread> _io_thread;
};