cmake_minimum_required(VERSION 3.19)

project(VioletClient LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(RESOURCES
    resources.qrc
)

#find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network Sql)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Sql)

qt_standard_project_setup()

qt_add_executable(VioletClient
    WIN32 MACOSX_BUNDLE
    main.cpp
    violetclient.cpp
    violetclient.h
    violetclient.ui
    protocol.h
    protocol.cpp
    unlogincenter.h unlogincenter.cpp unlogincenter.ui
    pvp.h pvp.cpp pvp.ui
    messagedispatcher.h messagedispatcher.cpp
    clickabletextbrowser.h clickabletextbrowser.cpp
    resources.qrc
    ${RESOURCES}
    sqlitehelper.h sqlitehelper.cpp
    logincenter.h logincenter.cpp logincenter.ui
    registerviolet.h registerviolet.cpp registerviolet.ui
    usermainwindow.h usermainwindow.cpp usermainwindow.ui
    README.md
    android/AndroidManifest.xml

)

target_link_libraries(VioletClient
    PRIVATE
    Qt::Core
    Qt::Widgets
    Qt::Network
    Qt::Sql
)

include(GNUInstallDirs)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    # 必须包含，否则找不到ui_*.h
    ${CMAKE_CURRENT_BINARY_DIR}
)

if (ANDROID)
    include(FetchContent)
    FetchContent_Declare(
        android_openssl
        DOWNLOAD_EXTRACT_TIMESTAMP true
        URL https://github.com/KDAB/android_openssl/archive/refs/heads/master.zip
    )
FetchContent_MakeAvailable(android_openssl)
include(${android_openssl_SOURCE_DIR}/android_openssl.cmake)
add_android_openssl_libraries(VioletClient)
endif()

install(TARGETS VioletClient
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET VioletClient
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
