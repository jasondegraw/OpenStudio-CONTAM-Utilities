CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

SET( target_name CONTAM_Utilities )
 
PROJECT( ${target_name} )

# High level project configuration

SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Products" )
SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Products" )
SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Products" )

# Build with Multiple Processes

IF(MSVC)
  OPTION( BUILD_WITH_MULTIPLE_PROCESSES "/MP compiler flag for full processor utilization" ON )
  MARK_AS_ADVANCED( BUILD_WITH_MULTIPLE_PROCESSES )
ENDIF()

# Required dependencies 

## OpenStudio

SET( OPENSTUDIO_BUILD_DIR "" CACHE PATH "Build or install directory for OpenStudio build to link against")

INCLUDE ( "${OPENSTUDIO_BUILD_DIR}/OSCore-prefix/src/OSCore-build/OpenStudioCoreConfig.cmake" )

IF(MSVC)
  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Products/Debug")
  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Products/Release")
  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Products/RelWithDebInfo")
  LIST( APPEND CMAKE_LIBRARY_PATH ${OPENSTUDIO_LIB_DIR} )

  file(GLOB DEBUG_DLLS "${OPENSTUDIO_BUILD_DIR}/OSCore-prefix/src/OSCore-build/Products/Debug/*.dll")
  file(GLOB RELEASE_DLLS "${OPENSTUDIO_BUILD_DIR}/OSCore-prefix/src/OSCore-build/Products/Release/*.dll")
  file(GLOB RELDBG_DLLS "${OPENSTUDIO_BUILD_DIR}/OSCore-prefix/src/OSCore-build/Products/RelWithDebInfo/*.dll")

  file(COPY ${DEBUG_DLLS} DESTINATION "${CMAKE_BINARY_DIR}/Products/Debug/")
  file(COPY ${RELEASE_DLLS} DESTINATION "${CMAKE_BINARY_DIR}/Products/Release/")
  file(COPY ${RELDBG_DLLS} DESTINATION "${CMAKE_BINARY_DIR}/Products/RelWithDebInfo/")
ENDIF()

INCLUDE_DIRECTORIES( ${OpenStudioCore_INCLUDE_DIRS} )

link_directories(${OpenStudioCore_LIBRARY_DIRS})

## Boost
IF(APPLE)
  set(Boost_USE_STATIC_LIBS ON)  
ENDIF()
IF (WIN32)
  set(Boost_USE_STATIC_LIBS ON)  
ENDIF (WIN32)

find_package(Boost 1.55.0 COMPONENTS filesystem regex program_options system thread chrono date_time log QUIET)

INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

## Qt
find_package(Qt5Widgets)
find_package(Qt5Sql)
find_package(Qt5Network)
find_package(Qt5Xml)
find_package(Qt5WebEngine)
find_package(Qt5WebEngineWidgets)
find_package(Qt5Concurrent)
find_package(Qt5PrintSupport)

mark_as_advanced(
  Qt5Concurrent_DIR
  Qt5Core_DIR
  Qt5Gui_DIR
  Qt5Gui_EGL_LIBRARY
  Qt5Location_DIR
  Qt5Multimedia_DIR
  Qt5MultimediaWidgets_DIR
  Qt5Network_DIR
  Qt5OpenGL_DIR
  Qt5Positioning_DIR
  Qt5PrintSupport_DIR
  Qt5Qml_DIR
  Qt5Quick_DIR
  Qt5Sensors_DIR
  Qt5Sql_DIR
  Qt5WebEngine_DIR
  Qt5WebEngineWidgets_DIR
  Qt5Widgets_DIR
  Qt5Xml_DIR
)

# Dependencies

SET( ${target_name}_depends
  ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS}
  ${QT_LIBS}
  openstudio_utilities
  openstudio_model
  openstudio_osversion
  openstudio_airflow
  # openstudio_energyplus
  boost_log
)

# Resource files
#SET( ${target_name}_qrc
#  resources.qrc
#)

# generate rules for building source files from the resources
#qt4_add_resources(${target_name}_qrcs ${${target_name}_qrc} )

# Executables

add_executable(osm2prj osm2prj.cpp)

TARGET_LINK_LIBRARIES( osm2prj 
  ${${target_name}_depends}
)

#add_executable(compinf compinf.cpp)

#TARGET_LINK_LIBRARIES( compinf ${${target_name}_depends})

#add_executable(simplefitinf simplefitinf.cpp)

#TARGET_LINK_LIBRARIES( simplefitinf ${${target_name}_depends})

#add_executable(epw2wth epw2wth.cpp)

#TARGET_LINK_LIBRARIES( epw2wth ${${target_name}_depends})

#add_executable(demomodel demomodel.cpp ${${target_name}_qrcs})

#TARGET_LINK_LIBRARIES( demomodel ${${target_name}_depends})

#add_executable(surfinf surfinf.cpp)

#TARGET_LINK_LIBRARIES( surfinf ${${target_name}_depends})
