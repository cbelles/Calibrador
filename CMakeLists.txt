cmake_minimum_required(VERSION 3.7)

PROJECT(Calibrador)


#===================================================================================

#===================================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)


INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

set(CMAKE_CXX_FLAGS " -O3 -fPIC -pthread -w")

# FIND PACKAGES
# test for qt5, use that if possible

if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
    set (X86 TRUE)
else ()
    set (X86 FALSE)
endif ()

IF ((UNIX) AND (X86))
	set(CMAKE_PREFIX_PATH "/home/pi/Qt5.14.0/5.14.0/gcc_64/lib/cmake")
ENDIF()

find_package(Qt5 COMPONENTS Core Gui Widgets PrintSupport REQUIRED)
find_package(Qt5 COMPONENTS Widgets QUIET)
if(Qt5_FOUND)
    find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core Widgets Network )
else()
    find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core Widgets)
endif()
message(STATUS "Qt Version: " ${QT_VERSION})
message(STATUS "Arquitectura: " ${CMAKE_HOST_SYSTEM_PROCESSOR})

SET (USE_OPENCV ON)
SET (USE_BOOST ON)
SET (USE_OPENLICENSEMANAGER ON)
SET (USE_WIRINGOP ON)

INCLUDE(${CMAKE_SOURCE_DIR}/../Ext/cmake/FruitVision.cmake)


set(RESOURCE ${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc)
QT5_ADD_RESOURCES(RESOURCES_RCC ${RESOURCE})

SET (SOURCES	main.cpp src/mainwindow.cpp  
                src/CalibersTable.cpp
                src/SalidasTable.cpp
                src/CalibrationProgram.cpp
                src/CalibrationDimension.cpp
                src/CaliberHistory.cpp 

                src/Params/Params.cpp 
                src/Params/ParamsGeneral.cpp
                src/Params/ParamsFruta.cpp
                
                src/FruitColorIndexs/ColorIndexLabel.cpp
                src/FruitColorIndexs/ColorIndexBar.cpp

                src/Widgets/LedStrip.cpp
                src/Widgets/CounterWidget.cpp
                src/Widgets/DimensionsWidget.cpp

                src/Cameras/CameraManager.cpp
                src/Cameras/Mensaje.cpp
                src/Cameras/CaliberClassifier.cpp
                src/Cameras/OutputManager.cpp
                
                src/OutputBoards/OutputBoardManager.cpp

                src/Dialogs/OutputPositionsDialog.cpp
                )
				
SET (HEADERS    include/mainwindow.h   
                include/CalibersTable.h
                include/SalidasTable.h 
                include/CalibrationProgram.h  
                include/Caliber.h  
                include/CalibrationDimension.h
                include/CaliberHistory.h 
                include/ExpulsionManager.h
                include/Fotocelula.h

                include/Params/Params.h 
                include/Params/ParamsGeneral.h
                include/Params/ParamsFruta.h

                include/license.h 
                include/license_mac.h

                include/Utils/json.hpp

                include/FruitColorIndexs/ColorIndexLabel.h
                include/FruitColorIndexs/ColorIndexBar.h

                include/Widgets/LedStrip.h
                include/Widgets/CounterWidget.h
                include/Widgets/DimensionsWidget.h

                include/Cameras/CameraManager.h
                include/Cameras/Mensaje.h
                include/Cameras/CaliberClassifier.h
                include/Cameras/OutputManager.h

                include/OutputBoards/OutputBoardManager.h

                include/Dialogs/OutputPositionsDialog.h

                include/Linux/network.h
                include/ADVobfuscator_Lib/MetaString.h
                include/ADVobfuscator_Lib/Inline.h
                include/ADVobfuscator_Lib/Indexes.h
                include/ADVobfuscator_Lib/MetaRandom.h
                include/ADVobfuscator_Lib/Log.h
                include/ADVobfuscator_Lib/ObfuscatedCall.h
                include/ADVobfuscator_Lib/ObfuscatedCallWithPredicate.h)

set(CMAKE_BUILD_RPATH "/usr/lib/aarch64-linux-gnu")
ADD_EXECUTABLE( ${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES_RCC})                       
TARGET_LINK_LIBRARIES(  ${PROJECT_NAME}  
                        Qt5::Core Qt5::Widgets Qt5::Gui 
                        Qt5::PrintSupport
                        ${OPENCV_CORE_LIBRARIES} ${OPENCV_HIGHGUI_LIBRARIES} ${OPENCV_IMGPROC_LIBRARIES} ${OPENCV_IMGCODECS_LIBRARIES}
                        ${OPENLICENSEMANAGER_LIBRARIES} ${BCRYPT_LIBRARIES} 
                        ${WIRINGOP_LIBRARIES}
                        )

# Añadir el ejecutable de prueba
ADD_EXECUTABLE(TestExpulsionManager 
              testexpulsion.cpp 
              include/ExpulsionManager.h)
TARGET_LINK_LIBRARIES(TestExpulsionManager)

ADD_EXECUTABLE(TestTarjetasSalidas TestSalidas.cpp)
TARGET_LINK_LIBRARIES(TestTarjetasSalidas Qt5::Core Qt5::Widgets Qt5::Gui )

