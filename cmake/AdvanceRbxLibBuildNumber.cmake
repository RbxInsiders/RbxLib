foreach(_required_variable IN ITEMS
    RBXLIB_VERSION_FILE
    RBXLIB_VERSION_LOCK_FILE
)
    if(NOT DEFINED ${_required_variable})
        message(FATAL_ERROR "${_required_variable} must be provided")
    endif()
endforeach()

get_filename_component(_lock_directory "${RBXLIB_VERSION_LOCK_FILE}" DIRECTORY)
file(MAKE_DIRECTORY "${_lock_directory}")
file(LOCK "${RBXLIB_VERSION_LOCK_FILE}" GUARD PROCESS TIMEOUT 30
    RESULT_VARIABLE _lock_result)
if(NOT _lock_result STREQUAL "0")
    message(FATAL_ERROR "Could not lock the RbxLib build version: ${_lock_result}")
endif()

file(READ "${RBXLIB_VERSION_FILE}" _current_version)
string(STRIP "${_current_version}" _current_version)
if(NOT _current_version MATCHES "^([0-9]+\\.[0-9]+\\.[0-9]+\\.)([0-9]+)$")
    message(FATAL_ERROR "Invalid RbxLib build version: ${_current_version}")
endif()

set(_build_number "${CMAKE_MATCH_2}")
if(_build_number GREATER_EQUAL 4294967295)
    message(FATAL_ERROR "RbxLib build number cannot exceed 4294967295")
endif()

math(EXPR _next_build_number "${_build_number} + 1")
file(WRITE "${RBXLIB_VERSION_FILE}" "${CMAKE_MATCH_1}${_next_build_number}\n")
message(STATUS "Advanced RbxLib build version to ${CMAKE_MATCH_1}${_next_build_number}")