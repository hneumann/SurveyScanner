CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
INCLUDE(FindPkgConfig)

SET( PROJECT_NAME SurveyScanner )
PROJECT( ${PROJECT_NAME} )

SET(SurveyScanner_LIBNAME "${PROJECT_NAME}")
SET(SurveyScanner_VERSION_MAJOR    "0")
SET(SurveyScanner_VERSION_MINOR    "1")
SET(SurveyScanner_VERSION_REVISION "0")
SET(SurveyScanner_VERSION   "${SurveyScanner_VERSION_MAJOR}.${SurveyScanner_VERSION_MINOR}.${SurveyScanner_VERSION_REVISION}")
SET(SurveyScanner_SOVERSION "${SurveyScanner_VERSION_MAJOR}.${SurveyScanner_VERSION_MINOR}")
SET(SurveyScanner_PACKAGE   "${SurveyScanner_LIBNAME}-${SurveyScanner_VERSION_MAJOR}.${SurveyScanner_VERSION_MINOR}")

SET(PREFIX      ${CMAKE_INSTALL_PREFIX})
SET(BINDIR     "${PREFIX}/bin")
SET(INCLUDEDIR "${PREFIX}/include/SurveyScanner")
SET(MANDIR     "${PREFIX}/man")
SET(LIBDIR     "${PREFIX}/lib")
SET(PKGDIR     "${PREFIX}/lib/pkgconfig")
SET(CFGDIR     "${PREFIX}/etc/SurveyScanner")
SET(DATADIR    "${PREFIX}/share/SurveyScanner")
#SET(KINECTDIR "D:/hneumann/install/lib")
#SET(BOOST_ROOT "C:/opt/boost-1.57")
# set(BOOST_INCLUDEDIR C:/opt/boost-1.57/include/boost-1_57)
# set(BOOST_LIBRARYDIR C:/opt-boost-1.57/x86)
#SET(BOOST_LIBRARYDIR "C:/opt/boost-1.57/include/boost-1_57/boost")


SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -fstack-protector")

SET(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" )
# if ("$ENV{Boost_DIR}" STREQUAL "")
   # set(BOOST_ROOT ${CMAKE_INSTALL_PREFIX})
# endif()

# set(Boost_USE_STATIC_LIBS        ON)

# set (Boost_DEBUG TRUE)

# search for Boost (we need boost >= 1.54 in order to have boost.log)
# find_package(Boost 1.54  COMPONENTS  thread system date_time chrono)
# if (Boost_FOUND)
  # message(STATUS "using boost version ${Boost_VERSION} from ${Boost_LIBRARY_DIRS}")
  # INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS} )
  # add_definitions(-DBOOST_LOG_AVAILABLE=1)
# else (Boost_FOUND)
  # message(STATUS "could not find a boost version with boost log. building without boost-log support!")
  # find_package(Boost REQUIRED COMPONENTS thread system date_time chrono) # chrono program_options system filesystem regex 
  # if (Boost_FOUND)
  #   message(STATUS "using boost version ${Boost_VERSION} from ${Boost_LIBRARY_DIRS}")
  #   INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS} )
  # else (Boost_FOUND)
  #   message(FATAL_ERROR "could not find boost")
  # endif (Boost_FOUND)
# endif(Boost_FOUND)

# ADD_DEFINITIONS(-D BOOST_ALL_DYN_LINK)

# Search for opencv

find_package(OpenCV 2.4.9 REQUIRED core imgproc highgui ml gpu ocl contrib flann calib3d objdetect video flann features2d )

  MESSAGE(STATUS "using opencv version ${OPENCV_VERSION} from ${OPENCV_PREFIX}")
  INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})


#this dummy list will make all header files appear if you use qtcreator to open the cmake project
file(GLOB HEADER_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} include/*.h  )


add_executable(${SurveyScanner_LIBNAME} 
	src/SurveyScanner.cpp
	${HEADER_LIST}
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
LINK_DIRECTORIES(${OpenCV_LIBRARY_DIRS}  ${LIBDIR} ${CMAKE_CURRENT_SOURCE_DIR}/lib  ${Boost_LIBS} )
target_link_libraries(${SurveyScanner_LIBNAME}  ${OpenCV_LIBS}  ${Boost_LIBRARIES} )
SET_TARGET_PROPERTIES(${SurveyScanner_LIBNAME} PROPERTIES VERSION ${SurveyScanner_VERSION} SOVERSION ${SurveyScanner_SOVERSION})


#build & install bart_iio
#ADD_EXECUTABLE(bart_iio src/bart_iio.cc ${HEADER_LIST})
#TARGET_LINK_LIBRARIES(bart_iio ${OpenCV_LIBS} ${Boost_LIBRARIES} ${Boost_LIBS} bart)
#install(TARGETS bart_iio DESTINATION bin)




install (TARGETS SurveyScanner DESTINATION lib )
INSTALL(FILES ${HEADER_LIST} DESTINATION ${INCLUDEDIR})
set_target_properties(${SurveyScanner_LIBNAME} PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
set_target_properties(${SurveyScanner_LIBNAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
set_property(TARGET ${SurveyScanner_LIBNAME} PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)


#create package config file
SET(PKGCONFIG_FILENAME "SurveyScanner.pc")
configure_file("${PKGCONFIG_FILENAME}.in" "${PKGCONFIG_FILENAME}" @ONLY)
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PKGCONFIG_FILENAME}" DESTINATION "${PKGDIR}")

#create cmake config files
SET(CMAKECONFIG_FILENAME "SurveyScannerConfig.cmake")
configure_file("${CMAKECONFIG_FILENAME}.in" "${CMAKECONFIG_FILENAME}" @ONLY)
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKECONFIG_FILENAME}" DESTINATION "${DATADIR}")
SET(CMAKECONFIGVERSION_FILENAME "SurveyScannerConfigVersion.cmake")
configure_file("${CMAKECONFIGVERSION_FILENAME}.in" "${CMAKECONFIGVERSION_FILENAME}" @ONLY)
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKECONFIGVERSION_FILENAME}" DESTINATION "${DATADIR}")



