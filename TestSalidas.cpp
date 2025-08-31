// main.cpp
#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QThread>
#include <boost/asio.hpp>
#include <ctime>
#include <random>
#include <chrono>
#include <iostream>

using namespace std;

class UdpThread : public QThread {
    Q_OBJECT
public:
    UdpThread(QObject *parent = nullptr) : QThread(parent), io_context(), socket(io_context) {
        stopping = false;
    }

    void stop() {
        stopping = true;
    }

    void setPort(int newPort) {
        port = newPort;
    }

    void setIp(const QString& newIp) {
        ip = newIp;
    }

    void setSpeed(float newSpeed) {
        speed = newSpeed;
    }

    void setNumChannels(int channels) {
        numChannels = channels;
    }

protected:
    void run() override {
        boost::asio::ip::udp::endpoint local_endpoint(boost::asio::ip::udp::v4(), port);

        cout << "IP: " << ip.toStdString() << endl;
        cout << "Puerto: " << port << endl;
        cout << "Velocidad: " << speed << endl;
        cout << "numChannels: " << numChannels << endl;

        socket.open(local_endpoint.protocol());
        socket.bind(local_endpoint);

        int cnt = 0;
        int op = 0;
        auto cycleExpulsores = std::chrono::steady_clock::now();
        auto cycleOtros = cycleExpulsores;
        auto cycleRecepcion = cycleExpulsores;

        while (!stopping) {
            auto now = std::chrono::steady_clock::now();
            
            float cycle = 1.0f / speed;
            if (std::chrono::duration<float>(now - cycleExpulsores).count() > cycle) {
                
                //cout << "Ciclo: " << cnt << endl;
                cycleExpulsores = now;
                cnt++;
                
                uint16_t valor = 0;
                while (NumeroDeUnos(valor) < numChannels) {
                    valor |= (1 << (rand() % 16));
                }

                //emit updateEnvio(QString("%1 -> %2").arg(cnt).arg(QString::number(valor, 16).rightJustified(4, '0')));
                
                QByteArray data;
                data.append('\xC5').append('\x01');
                data.append((char)(valor >> 8)).append((char)(valor & 0xFF));
                data.append('\0').append('\0').append('\0').append('\x5C');
                boost::asio::ip::udp::endpoint remote_endpoint(boost::asio::ip::address::from_string(ip.toStdString()), port);
                socket.send_to(boost::asio::buffer(data.data(), data.size()), remote_endpoint);
            }

            msleep(10); // Prevent high CPU usage
        }
    }

private:
    bool stopping;
    QString ip;
    int port;
    float speed;
    int numChannels;
    float dacValue;
    float coef;
    boost::asio::io_context io_context;
    boost::asio::ip::udp::socket socket;

    int NumeroDeUnos(uint16_t valor) {
        int cnt = 0;
        uint16_t tes = 1;
        for (int i = 0; i < 16; i++) {
            if (valor & tes) cnt++;
            tes <<= 1;
        }
        return cnt;
    }

signals:
    void updateEnvio(const QString &text);
    void updateAdc(const QString &text);
    void updateIn(const QString &text);
    void updateKg(const QString &text);
    void error(const QString &text);
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setupUi();
        udpThread = new UdpThread(this);
        
        connect(startButton, &QPushButton::clicked, this, &MainWindow::startStop);
        connect(udpThread, &UdpThread::updateEnvio, envioLabel, &QLabel::setText);
        connect(udpThread, &UdpThread::updateAdc, adcLabel, &QLabel::setText);
        connect(udpThread, &UdpThread::updateIn, inLabel, &QLabel::setText);
        connect(udpThread, &UdpThread::updateKg, kgLabel, &QLabel::setText);
        connect(channelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int index) {
                if (udpThread->isRunning()) {
                    udpThread->setNumChannels(index + 1);
                }
            });
    }

    ~MainWindow() {
        if (udpThread->isRunning()) {
            udpThread->stop();
            udpThread->wait();
        }
    }

private:
    void setupUi() {
        QWidget *centralWidget = new QWidget;
        setCentralWidget(centralWidget);
        
        QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
        
        // Left column
        QVBoxLayout *leftColumn = new QVBoxLayout;
        
        // IP input
        QLabel *ipLabel = new QLabel("IP:");
        ipInput = new QLineEdit("10.0.0.150");
        leftColumn->addWidget(ipLabel);
        leftColumn->addWidget(ipInput);
        
        // Port input
        QLabel *portLabel = new QLabel("Puerto:");
        portInput = new QLineEdit("5000");
        leftColumn->addWidget(portLabel);
        leftColumn->addWidget(portInput);
        
        // Start/Stop button
        startButton = new QPushButton("Start");
        leftColumn->addWidget(startButton);
        
        // Speed slider
        QLabel *speedLabel = new QLabel("Velocidad Pz/Seg:");
        speedSlider = new QSlider(Qt::Horizontal);
        speedSlider->setRange(5, 50); // 0.5 to 5.0
        speedSlider->setValue(20);     // 2.0
        speedIndicator = new QLabel("2.0");
        connect(speedSlider, &QSlider::valueChanged, this, &MainWindow::updateSpeedIndicator);
        leftColumn->addWidget(speedLabel);
        leftColumn->addWidget(speedSlider);
        leftColumn->addWidget(speedIndicator);
        
        // Channel selector
        QLabel *channelLabel = new QLabel("Canales simultaneos activos:");
        channelCombo = new QComboBox;
        for (int i = 1; i <= 8; i++) {
            channelCombo->addItem(QString::number(i));
        }
        leftColumn->addWidget(channelLabel);
        leftColumn->addWidget(channelCombo);
        
        // Output label
        QLabel *outputLabel = new QLabel("Salida:");
        envioLabel = new QLabel("---");
        leftColumn->addWidget(outputLabel);
        leftColumn->addWidget(envioLabel);
        
        // Right column
        QVBoxLayout *rightColumn = new QVBoxLayout;
        
        // DAC slider
        QLabel *dacLabel = new QLabel("Cero offset (DAC):");
        dacSlider = new QSlider(Qt::Horizontal);
        dacSlider->setRange(0, 330); // 0.0 to 3.3
        dacSlider->setValue(155);    // 1.55
        rightColumn->addWidget(dacLabel);
        rightColumn->addWidget(dacSlider);
        
        // ADC display
        QLabel *adcTxLabel = new QLabel("ADC:");
        adcLabel = new QLabel("---");
        rightColumn->addWidget(adcTxLabel);
        rightColumn->addWidget(adcLabel);
        
        // Input display
        QLabel *inTxLabel = new QLabel("Entradas:");
        inLabel = new QLabel("---");
        rightColumn->addWidget(inTxLabel);
        rightColumn->addWidget(inLabel);
        
        // Coefficient input
        QLabel *coefLabel = new QLabel("Coef Uc/Gr:");
        coefInput = new QLineEdit("0.0038");
        kgLabel = new QLabel("Kg:");
        rightColumn->addWidget(coefLabel);
        rightColumn->addWidget(coefInput);
        rightColumn->addWidget(kgLabel);
        
        mainLayout->addLayout(leftColumn);
        mainLayout->addLayout(rightColumn);
    }

private slots:
    void startStop() {
        if (udpThread->isRunning()) {
            udpThread->stop();
            udpThread->wait();
            startButton->setText("Start");
        } else {
            bool ok;
            int port = portInput->text().toInt(&ok);
            if (!ok) {
                // Manejar error de conversión
                return;
            }
            float speed = speedSlider->value() / 10.0f;
            udpThread->setPort(port);
            udpThread->setIp(ipInput->text());
            udpThread->setSpeed(speed);
            udpThread->setNumChannels(channelCombo->currentIndex() + 1);
            startButton->setText("Stop");
            udpThread->start();
        }
    }
    
    void updateSpeedIndicator(int value) {
        float speed = value / 10.0f;
        speedIndicator->setText(QString::number(speed, 'f', 1));
        if (udpThread->isRunning()) {
            udpThread->setSpeed(speed);
        }
    }

private:
    UdpThread *udpThread;
    QLineEdit *ipInput;
    QLineEdit *portInput;
    QPushButton *startButton;
    QSlider *speedSlider;
    QLabel *speedIndicator;
    QComboBox *channelCombo;
    QLabel *envioLabel;
    QSlider *dacSlider;
    QLabel *adcLabel;
    QLabel *inLabel;
    QLineEdit *coefInput;
    QLabel *kgLabel;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}

#include "TestSalidas.moc"