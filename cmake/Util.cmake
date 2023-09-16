function(add_files)
    cmake_parse_arguments(PARAM "" "" "CONDITION" ${ARGN})
    set(PARAM_FILES "${PARAM_UNPARSED_ARGUMENTS}")

    if(PARAM_CONDITION)
        if(NOT (${PARAM_CONDITION}))
            return()
        endif()
    endif()

    foreach(FILE IN LISTS PARAM_FILES)
        target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${FILE})
    endforeach()
endfunction()

function (add_local_library LIB_NAME LIB_TYPE)
    if(${LIB_TYPE} STREQUAL  "shared")
        message(INFO "[BUILDING] getting dynamic link info for - ${LIB_NAME}...")
        add_library(${LIB_NAME} SHARED IMPORTED)

        if(CMAKE_SYSTEM_NAME MATCHES Linux)
            set_property ( TARGET ${LIB_NAME} PROPERTY IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/Binarys/${LIB_NAME}.so )
        elseif(CMAKE_SYSTEM_NAME MATCHES Windows)
            set_target_properties(${LIB_NAME} PROPERTIES
              IMPORTED_IMPLIB  ${PROJECT_SOURCE_DIR}/Binarys/${LIB_NAME}.lib
              IMPORTED_LOCATION  ${PROJECT_SOURCE_DIR}/Binarys/${LIB_NAME}.dll
            )
        elseif(CMAKE_SYSTEM_NAME MATCHES Android)
        else()
            message(FATAL_ERROR " system not supported")
        endif()
    else()
        message(INFO "[BUILDING] getting static link info for - ${LIB_NAME}...")
        add_library( ${LIB_NAME} STATIC IMPORTED)
        if(CMAKE_SYSTEM_NAME MATCHES Linux)
            set_property ( TARGET ${LIB_NAME} PROPERTY IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/Binarys/${LIB_NAME}.a )
        elseif(CMAKE_SYSTEM_NAME MATCHES Windows)
              set_target_properties(${LIB_NAME} PROPERTIES
                IMPORTED_IMPLIB  ${PROJECT_SOURCE_DIR}/Binarys/${LIB_NAME}.lib
              )
        elseif(CMAKE_SYSTEM_NAME MATCHES Android)
        else()
            message(FATAL_ERROR "system not supported")
        endif()
    endif()
endfunction()