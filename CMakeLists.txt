cmake_minimum_required(VERSION 3.7)
project(qtdesigner)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH} )

set(MIN_KDEV_VERSION "5.1.80")
set(MIN_QT_VER "5.5.0")
set(MIN_KF_VER "5.15.0")

#Find what is the prefix of the KDevelop installation
set(KDevelop_Include_DIR "include/kdevelop")
find_path(KDevelop_PREFIX ${KDevelop_Include_DIR})
set(KDevelop_Include_Full_DIR ${KDevelop_PREFIX}/${KDevelop_Include_DIR})
message(STATUS "KDevelop headers       : " ${KDevelop_Include_Full_DIR})
					
set(KDevPlatform_Include_DIR "include/kdevplatform")
set(KDevPlatform_Include_Full_DIR ${KDevelop_PREFIX}/${KDevPlatform_Include_DIR})
message(STATUS "KDevPlatform headers   : " ${KDevPlatform_Include_Full_DIR})

include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(FeatureSummary)

find_package(Qt5 ${MIN_QT_VER} CONFIG REQUIRED Core Gui Widgets Test Designer)
find_package(KF5 ${MIN_KF_VER} REQUIRED COMPONENTS ItemModels TextEditor)
find_package(KDevelop ${MIN_KDEV_VERSION} REQUIRED)
find_package(KDevPlatform ${MIN_KDEV_VERSION} REQUIRED CONFIG)

include_directories(
    ${KDevPlatform_Include_Full_DIR}
    ${KDevelop_Include_Full_DIR}
    ${KDevPlatform_DIR}
    ${KDevelop_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    "/usr/include/qt5/"
)

include_directories( ${qtdesigner_SOURCE_DIR} ${qtdesigner_BINARY_DIR})

#add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9038 )

########### next target ###############

SET(kdevqtdesigner_PART_SRCS
qtdesignerplugin.cpp
qtdesignerdocument.cpp
qtdesignerview.cpp
qtdesignerwidget.cpp
)

qt5_add_resources(kdevqtdesigner_PART_SRCS kdevqtdesigner.qrc)
kdevplatform_add_plugin(kdevqtdesigner JSON kdevqtdesigner.json SOURCES
		${kdevqtdesigner_PART_SRCS})



#TARGET_LINK_LIBRARIES(kdevqtdesigner ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS} 
#${KDEVPLATFORM_INTERFACES_LIBRARIES} ${QT_QTDESIGNER_LIBRARY} ${QT_QTDESIGNERCOMPONENTS_LIBRARY} 
#${KDEVPLATFORM_SUBLIME_LIBRARIES})

TARGET_LINK_LIBRARIES(kdevqtdesigner
    KDev::Interfaces
    KDev::Project
    KDev::Sublime
    KDev::Shell
    KF5::WidgetsAddons
    Qt5::UiPlugin
    Qt5::Designer
    )
install(TARGETS kdevqtdesigner DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevqtdesigner.desktop DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES kdevqtdesigner.rc DESTINATION  ${DATA_INSTALL_DIR}/kdevqtdesigner )
