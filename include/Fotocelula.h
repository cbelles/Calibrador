#ifndef _FOTOCELULA_H
#define _FOTOCELULA_H

#include <wiringPi.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <iostream>

using namespace std;

/**
 * @brief Class to handle a photoelectric sensor using GPIO
 * 
 * This class implements a thread-safe GPIO reader that can detect
 * stable high signals from a photoelectric sensor. It uses filtering
 * to avoid false triggers and provides synchronization mechanisms.
 */
class Fotocelula {
public:
    /**
     * @brief Construct a new Fotocelula object
     * @param pin GPIO pin number to monitor
     * @throw std::runtime_error if GPIO initialization fails
     */
    Fotocelula(int pin = 0) : _pin(pin), _running(false), _detected(false) { // Default to wPi 0 (pos 3 fisica)
        if (wiringPiSetup() == -1) {
            throw std::runtime_error("Error initializing WiringPi");
        }
        
        pinMode(_pin, INPUT);
        pullUpDnControl(_pin, PUD_UP);
    }

    ~Fotocelula() {
        stop();
    }

    /**
     * @brief Start the GPIO monitoring thread
     */
    void start() {
        _running = true;
        _thread = std::thread(&Fotocelula::operator(), this);
    }

    /**
     * @brief Stop the GPIO monitoring thread
     */
    void stop() {
        _running = false;
        if (_thread.joinable()) {
            _thread.join();
        }
    }

    /**
     * @brief Check if a signal was detected
     * @return true if a valid signal was detected
     * @thread-safe
     */
    bool isDetected() {
        std::unique_lock<std::mutex> lock(_mutex);
        return _detected;
    }

    /**
     * @brief Get the current detection state
     * @return Current value of _detected flag
     */
    bool getDetected() const {
        return _detected.load();
    }

    /**
     * @brief Wait for the next valid signal detection
     * 
     * @param timeout_ms Timeout in milliseconds (default 5000ms)
     * @return bool true if detection occurred, false if timeout
     * @thread-safe
     */
    bool waitForDetection(int timeout_ms = 5000) {
        std::unique_lock<std::mutex> lock(_mutex);
        //cout << "Waiting for detection (timeout: " << timeout_ms << "ms)..." << endl;
        if (_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this] { return getDetected(); })) {
            cout << "Detection received!" << endl;
            _detected.store(false);
            return true;
        } else {
            cout << "Timeout waiting for detection!" << endl;
            return false;
        }
    }

    /**
     * @brief Thread function that monitors the GPIO
     * 
     * Implements signal filtering by requiring multiple consecutive
     * high readings before triggering a detection. Samples the GPIO
     * at regular intervals defined by SAMPLE_PERIOD.
     */
    void operator()() {
        int consecutive_ones = 0;
        const int REQUIRED_ONES = 3; // Reducido para pruebas
        const auto SAMPLE_PERIOD = std::chrono::milliseconds(10);

        while (_running) {
            try {
                int value = digitalRead(_pin);
                
                if (value == 1) {
                    consecutive_ones++;
                    if (consecutive_ones >= REQUIRED_ONES) {
                        if (!_previous_state) {
                            std::unique_lock<std::mutex> lock(_mutex);
                            _detected.store(true);
                            _previous_state = true;
                            cout << "Rising edge detected - Notifying..." << endl;
                            _cv.notify_all(); // Cambiado a notify_all
                            //emit sensorActivated(); // Emit signal
                        }
                    }
                } else {
                    if (_previous_state) {
                        cout << "Signal went low" << endl;
                    }
                    consecutive_ones = 0;
                    _previous_state = false;
                }

                std::this_thread::sleep_for(SAMPLE_PERIOD);
            } catch (const std::exception& e) {
                std::cerr << "Error in Fotocelula thread: " << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }

    bool isThreadRunning() const {
        return _running && _thread.joinable();
    }

signals:
    //void sensorActivated();

private:
    const int _pin;                    ///< WiringPi pin number
    std::atomic<bool> _running;        ///< Thread control flag
    std::atomic<bool> _detected;       ///< Signal detection flag
    std::thread _thread;              ///< Monitoring thread
    std::mutex _mutex;                ///< Synchronization mutex
    std::condition_variable _cv;      ///< Synchronization condition variable
    bool _previous_state = false;     ///< Previous state for edge detection
};

#endif



