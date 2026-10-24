cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# ?
#set(CPPSYNTH_BUILD_TYPE $<$<CONFIG:Debug>:DEBUG>)
#set(CPPSYNTH_BUILD_TYPE $<$<CONFIG:Release>:RELEASE>)

###################################
# Define the project

project("CppSynth")

set(CPPSYNTH_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CPPSYNTH_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")

# Get all source files
file(
    GLOB_RECURSE CPPSYNTH_SOURCES CONFIGURE_DEPENDS
    "${CPPSYNTH_ROOT_DIR}/src/*.cpp"
)
# Get all header files
file(
    GLOB_RECURSE CPPSYNTH_HEADERS CONFIGURE_DEPENDS
    "${CPPSYNTH_ROOT_DIR}/src/*.h"
    "${CPPSYNTH_ROOT_DIR}/src/*.hpp"
    "${CPPSYNTH_ROOT_DIR}/include/*.h"
    "${CPPSYNTH_ROOT_DIR}/include/*.hpp"
)

# Create executable target & add source and header files
add_executable("CppSynth" "${CPPSYNTH_SOURCES}" "${CPPSYNTH_HEADERS}")
# Add include directories
target_include_directories("CppSynth" PUBLIC "${CPPSYNTH_ROOT_DIR}/include/")

if(WIN32)

    # Get all debug libraries
    file(GLOB_RECURSE CPPSYNTH_LIBS_DEBUG CONFIGURE_DEPENDS "${CPPSYNTH_ROOT_DIR}/lib/Windows/Debug/*/*.lib")
    file(GLOB_RECURSE CPPSYNTH_DLLS_DEBUG CONFIGURE_DEPENDS "${CPPSYNTH_ROOT_DIR}/lib/Windows/Debug/*/*.dll")
    # Get all release libraries
    file(GLOB_RECURSE CPPSYNTH_LIBS_RELEASE CONFIGURE_DEPENDS "${CPPSYNTH_ROOT_DIR}/lib/Windows/Release/*/*.lib")
    file(GLOB_RECURSE CPPSYNTH_DLLS_RELEASE CONFIGURE_DEPENDS "${CPPSYNTH_ROOT_DIR}/lib/Windows/Release/*/*.dll")

    # The executable always looks for DLL files in the same directory as itself,
    # so for it to function there need to be a copy of the DLLs in the build directory.
    # There probably is a smarter way to achieve that...
    
    # Copy over the DLL files (Windows, Debug)
    foreach(DYNAMIC_LIBRARY IN LISTS CPPSYNTH_DLLS_DEBUG)
        add_custom_command(
            TARGET "CppSynth" POST_BUILD
            COMMAND powershell -Command Copy-Item -Path "\"${DYNAMIC_LIBRARY}\"" -Destination "\"${CPPSYNTH_OUT_DIR}\""
            VERBATIM
        )
    endforeach()

    #foreach(DYNAMIC_LIBRARY IN LISTS CPPSYNTH_DLLS_RELEASE)
    #    add_custom_command(
    #        TARGET "CppSynth" POST_BUILD
    #        COMMAND powershell -Command Copy-Item -Path "\"${DYNAMIC_LIBRARY}\"" -Destination "\"${CPPSYNTH_OUT_DIR}\""
    #        VERBATIM
    #    )
    #endforeach()

elseif(UNIX)

    message(FATAL_ERROR "An appropriate CMake configuration is not yet done for the Linux build.")

endif()


# Debug configuration
target_link_libraries("CppSynth" PUBLIC "${CPPSYNTH_LIBS_DEBUG}")
target_compile_definitions("CppSynth" PUBLIC DEBUG)

# Release configuration
#target_link_libraries("CppSynth" "${CPPSYNTH_DLLS_RELEASE}" "${CPPSYNTH_LIBS_RELEASE}")
#target_compile_definitions("CppSynth" PUBLIC RELEASE)
