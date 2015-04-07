cmake_minimum_required(VERSION 2.8)

project(qskeleton)

set(CMAKE_CXX_FLAGS "-O2")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (WIN32 AND (CMAKE_SIZEOF_VOID_P EQUAL 8))
    set(CMAKE_EXE_LINKER_FLAGS "/machine:x64")
    set(CMAKE_MODULE_LINKER_FLAGS "/machine:x64")
    set(CMAKE_SHARED_LINKER_FLAGS "/machine:x64")
    set(CMAKE_STATIC_LINKER_FLAGS "/machine:x64")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_definitions(-DNDEBUG)
endif()

set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

if (CMAKE_C_COMPILER MATCHES CMAKE_C_COMPILER-NOTFOUND)
    message(WARNING "Cannot find compiler")
endif()

message(STATUS "Using compiler: ${CMAKE_CXX_COMPILER_ID}")

set(DEPS_DIR ${ROOT_DIR}/deps/tclap/include)

add_subdirectory(src/converter)
add_subdirectory(src/ddm)
add_subdirectory(src/ddmUI)
add_subdirectory(src/elimination)
add_subdirectory(src/eliminationUI)
add_subdirectory(src/UIUtils)

set_target_properties(UIUtils PROPERTIES FOLDER Libraries)
set_target_properties(ddm_ide PROPERTIES FOLDER Libraries)
set_target_properties(elimination_ide PROPERTIES FOLDER Libraries)

if (UNIX)
	add_custom_target(post_binaries ALL DEPENDS ddmUI eliminationUI
					  COMMAND ${CMAKE_COMMAND} -E make_directory ${ROOT_DIR}/bin
					  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:ddmUI> ${ROOT_DIR}/bin/ddm
					  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:eliminationUI> ${ROOT_DIR}/bin/elimination)
else()
	add_custom_target(post_binaries ALL DEPENDS ddmUI eliminationUI
					  COMMAND ${CMAKE_COMMAND} -E make_directory ${ROOT_DIR}/bin
					  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:ddmUI> ${ROOT_DIR}/bin/ddm.exe
					  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:eliminationUI> ${ROOT_DIR}/bin/elimination.exe)
endif()

set_target_properties(post_binaries PROPERTIES FOLDER BuildInfrastructure)
