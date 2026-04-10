cmake_minimum_required(VERSION 3.19)
project(CloudStorage LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network)

qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

qt_add_executable(CloudStorage
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    icons.qrc
    process_http.h
    process_http.cpp
    login.h login.cpp login.ui
    src/json.hpp
    sha256.h
    base64.h
    base64.cpp
    sha256.cpp
    information.h
    mythread.h mythread.cpp


)

target_link_libraries(CloudStorage
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
)

include(GNUInstallDirs)

install(TARGETS CloudStorage
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET CloudStorage
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})
