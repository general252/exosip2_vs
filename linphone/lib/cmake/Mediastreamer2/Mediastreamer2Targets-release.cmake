#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "BelledonneCommunications::mediastreamer_base" for configuration "Release"
set_property(TARGET BelledonneCommunications::mediastreamer_base APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(BelledonneCommunications::mediastreamer_base PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/mediastreamer_base.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/mediastreamer_base.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS BelledonneCommunications::mediastreamer_base )
list(APPEND _IMPORT_CHECK_FILES_FOR_BelledonneCommunications::mediastreamer_base "${_IMPORT_PREFIX}/lib/mediastreamer_base.lib" "${_IMPORT_PREFIX}/bin/mediastreamer_base.dll" )

# Import target "BelledonneCommunications::mediastreamer_voip" for configuration "Release"
set_property(TARGET BelledonneCommunications::mediastreamer_voip APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(BelledonneCommunications::mediastreamer_voip PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/mediastreamer_voip.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/mediastreamer_voip.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS BelledonneCommunications::mediastreamer_voip )
list(APPEND _IMPORT_CHECK_FILES_FOR_BelledonneCommunications::mediastreamer_voip "${_IMPORT_PREFIX}/lib/mediastreamer_voip.lib" "${_IMPORT_PREFIX}/bin/mediastreamer_voip.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
