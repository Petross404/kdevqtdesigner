cmake_minimum_required(VERSION 3.7)
project(kdevqtdesigner)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH} )

set(MIN_KDEV_VERSION "5.1.0")
set(MIN_QT_VER "5.5.0")
set(MIN_KF_VER "5.20.0")

#Find what is the prefix of the KDevelop installation
# set(KDevelop_Include_DIR ${CMAKE_SYSTEM_INCLUDE_PATH}/kdevelop)
# set(KDevPlatform_Include_DIR ${CMAKE_SYSTEM_INCLUDE_PATH}/kdevplatform)

include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(FeatureSummary)

find_package(Qt5 ${MIN_QT_VER} CONFIG REQUIRED Core Gui Widgets Test Designer)
find_package(KF5 ${MIN_KF_VER} REQUIRED COMPONENTS ItemModels TextEditor TextWidgets)
find_package(KDevelop ${MIN_KDEV_VERSION} REQUIRED)
find_package(KDevPlatform ${MIN_KDEV_VERSION} REQUIRED CONFIG)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
)

include_directories( ${qtdesigner_SOURCE_DIR} ${qtdesigner_BINARY_DIR})

#add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9038 )

########### next target ###############

SET(kdevqtdesigner_PART_SRCS
    qtdesignerplugin.cpp
    qtdesignerdocument.cpp
    qtdesignerview.cpp
    qtdesignerwidget.cpp
    integration.cpp
)

qt5_add_resources(kdevqtdesigner_PART_SRCS kdevqtdesigner.qrc)
kdevplatform_add_plugin(kdevqtdesigner 
	JSON kdevqtdesigner.json SOURCES
	${kdevqtdesigner_PART_SRCS}
)

set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" "-Wl,--no-undefined" )

#TARGET_LINK_LIBRARIES(kdevqtdesigner ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS} 
#${KDEVPLATFORM_INTERFACES_LIBRARIES} ${QT_QTDESIGNER_LIBRARY} ${QT_QTDESIGNERCOMPONENTS_LIBRARY} 
#${KDEVPLATFORM_SUBLIME_LIBRARIES})

target_link_libraries(kdevqtdesigner
    KDev::Interfaces
    KDev::Project
    KDev::Sublime
    KDev::Shell
    KF5::WidgetsAddons
    KF5::TextEditor
    KF5::TextWidgets
    Qt5::UiPlugin
    Qt5::Designer
    Qt5DesignerComponents
    )

