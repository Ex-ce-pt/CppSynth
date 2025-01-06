cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

#set(CPPSYNTH_BUILD_TYPE $<$<CONFIG:Debug>:DEBUG>)
#set(CPPSYNTH_BUILD_TYPE $<$<CONFIG:Release>:RELEASE>)

# Define a project
project("CppSynth")

# Get all source files
file(GLOB_RECURSE CPPSYNTH_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
# Get all header files
file(GLOB_RECURSE CPPSYNTH_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h" "{CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

# Add source and header files
add_executable("CppSynth" "${CPPSYNTH_SOURCES}" "${CPPSYNTH_HEADERS}")
# Add include directories
target_include_directories("CppSynth" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

#target_link_directories("CppSynth" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/Windows/Debug/OpenAL/" "${CMAKE_CURRENT_SOURCE_DIR}/lib/Windows/Release/OpenAL/");

if(WIN32)
    # Get all debug libraries
    file(GLOB_RECURSE CPPSYNTH_LIBS_DEBUG CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/lib/Windows/Debug/*/*.lib")
    file(GLOB_RECURSE CPPSYNTH_DLLS_DEBUG CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/lib/Windows/Debug/*/*.dll")
    # Get all release libraries
    file(GLOB_RECURSE CPPSYNTH_LIBS_RELEASE CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/lib/Windows/Release/*/*.lib")
    file(GLOB_RECURSE CPPSYNTH_DLLS_RELEASE CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/lib/Windows/Release/*/*.dll")
endif()


# Debug configuration
target_link_libraries("CppSynth" PUBLIC "${CPPSYNTH_LIBS_DEBUG}")
target_compile_definitions("CppSynth" PUBLIC DEBUG)

# Release configuration
#target_link_libraries("CppSynth" "${CPPSYNTH_DLLS_RELEASE}" "${CPPSYNTH_LIBS_RELEASE}")
#target_compile_definitions("CppSynth" PUBLIC RELEASE)
