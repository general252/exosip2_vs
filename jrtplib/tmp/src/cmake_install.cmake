# Install script for directory: G:/Code/exosip2_vs/jrtplib/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "D:/Program Files (x86)/jrtplib")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/jrtplib3" TYPE FILE FILES
    "G:/Code/exosip2_vs/jrtplib/src/rtcpapppacket.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtcpbyepacket.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtcpcompoundpacket.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtcpcompoundpacketbuilder.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtcppacket.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtcppacketbuilder.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtcprrpacket.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtcpscheduler.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtcpsdesinfo.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtcpsdespacket.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtcpsrpacket.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtcpunknownpacket.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpaddress.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpcollisionlist.h"
    "G:/Code/exosip2_vs/jrtplib/tmp/src/rtpconfig.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpdebug.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpdefines.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtperrors.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtphashtable.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpinternalsourcedata.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpipv4address.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpipv4destination.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpipv6address.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpipv6destination.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpkeyhashtable.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtplibraryversion.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpmemorymanager.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpmemoryobject.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtppacket.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtppacketbuilder.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtppollthread.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtprandom.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtprandomrand48.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtprandomrands.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtprandomurandom.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtprawpacket.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpsession.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpsessionparams.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpsessionsources.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpsourcedata.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpsources.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpstructs.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtptimeutilities.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtptransmitter.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtptypes_win.h"
    "G:/Code/exosip2_vs/jrtplib/tmp/src/rtptypes.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpudpv4transmitter.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpudpv6transmitter.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpbyteaddress.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpexternaltransmitter.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpsecuresession.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpsocketutil.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpabortdescriptors.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtpselect.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtptcpaddress.h"
    "G:/Code/exosip2_vs/jrtplib/src/rtptcptransmitter.h"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "D:/Program Files (x86)/jrtplib/lib/jrtplib_d.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "D:/Program Files (x86)/jrtplib/lib" TYPE STATIC_LIBRARY FILES "G:/Code/exosip2_vs/jrtplib/tmp/src/Debug/jrtplib_d.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "D:/Program Files (x86)/jrtplib/lib/jrtplib.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "D:/Program Files (x86)/jrtplib/lib" TYPE STATIC_LIBRARY FILES "G:/Code/exosip2_vs/jrtplib/tmp/src/Release/jrtplib.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "D:/Program Files (x86)/jrtplib/lib/jrtplib.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "D:/Program Files (x86)/jrtplib/lib" TYPE STATIC_LIBRARY FILES "G:/Code/exosip2_vs/jrtplib/tmp/src/MinSizeRel/jrtplib.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "D:/Program Files (x86)/jrtplib/lib/jrtplib.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "D:/Program Files (x86)/jrtplib/lib" TYPE STATIC_LIBRARY FILES "G:/Code/exosip2_vs/jrtplib/tmp/src/RelWithDebInfo/jrtplib.lib")
  endif()
endif()

