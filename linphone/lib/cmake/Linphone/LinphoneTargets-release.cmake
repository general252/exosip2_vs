#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "BelledonneCommunications::linphone" for configuration "Release"
set_property(TARGET BelledonneCommunications::linphone APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(BelledonneCommunications::linphone PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/linphone.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/linphone.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS BelledonneCommunications::linphone )
list(APPEND _IMPORT_CHECK_FILES_FOR_BelledonneCommunications::linphone "${_IMPORT_PREFIX}/lib/linphone.lib" "${_IMPORT_PREFIX}/bin/linphone.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
