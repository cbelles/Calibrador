// main.cpp
#include <string>
#include <iostream>
#include <QApplication>
#include <QMainWindow>
#include <QTranslator>
#include <QStyle>
#include <QDesktopWidget>
#include "mainwindow.h"

#include "license.h"

using namespace std;

int main(int argc, char *argv[])
{
    string pathconfig = "/home/pi/CalibradorParams";

    QApplication app(argc, argv);
    
    #include "Params/load_params.h"  //ParamsGeneral paramsgeneral

    // Crear y configurar la ventana principal
    MainWindow mainWindow;

    vector<string> dimension_names;    
    paramsgeneral.get_dimensiones_name(dimension_names);
    CalibrationDimension::setDimensionNames(dimension_names);

    vector<pair<int,int>> dimensiones_domain_value;
    paramsgeneral.get_dimensiones_domain_value(dimensiones_domain_value);
    CalibrationDimension::setDimensionDomains(dimensiones_domain_value);

    //Configuracion del Calibrador
    int numSalidas = paramsgeneral.get_num_salidas();
    int numLineas  = paramsgeneral.get_num_lineas();
    cout << "numSalidas: " << numSalidas << endl;
    cout << "numLineas: " << numLineas << endl;
    mainWindow.configCalibrador(numSalidas, numLineas);
    mainWindow.setParamsFruta(&paramsfruta);

    //Cargar el idioma
    string idioma = paramsgeneral.get_idioma();
    cout << "Idioma: " << idioma << endl; 
    QTranslator translator; 
    if (idioma=="EN") {
        if (!translator.load(":/translations/EN.qm")) {
            cout << "Error loading EN translation" << endl;
        }
    }
    if (idioma=="PT") {
        if (!translator.load(":/translations/PT.qm")) {
            cout << "Error loading PT translation" << endl;
        }
    }
    app.installTranslator(&translator);

	//=======================================
	#include "license_mac.h"   //Comprueba la licencia de MAC, si no es correcta SALE del programa! 
	//=======================================

    //La asignacion en CalibrationDimension tiene que ser antes de ini 
    mainWindow.ini(&paramsgeneral);

    //string con el nombre del programa
    const char *programName = "Calibrador"; 
    mainWindow.setWindowTitle(QCoreApplication::translate("MainWindow", programName));
    
    // Verificar que los métodos existen y retornan valores válidos
    int X = paramsgeneral.get_dimX_window();
    int Y = paramsgeneral.get_dimY_window();
    
    // Imprimir los valores para depuración
    cout << "Dimensiones de ventana - X: " << X << ", Y: " << Y << endl;
    
    // Asegurarse de que los valores sean razonables
    if (X < 800) X = 1920;
    if (Y < 600) Y = 1060;
    
    // Redimensionar la ventana
    mainWindow.resize(X, Y);
    
    // Forzar el tamaño fijo para evitar que se cambie
    mainWindow.setFixedSize(X, Y);
    
    // Centrar la ventana en la pantalla
    mainWindow.setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            mainWindow.size(),
            app.desktop()->availableGeometry()
        )
    );

    mainWindow.loadProgram();
    // Mostrar la ventana
    mainWindow.show();
    
    // Iniciar el bucle de eventos
    return app.exec();
}
