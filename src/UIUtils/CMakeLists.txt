include_directories(
	${DEPS_DIR})

add_library(UIUtils
    Arithmetic.h
    GenericIOStream.h
    IOParams.h
    MatrixIO.hpp
    Arithmetic.cpp
    GenericIOStream.cpp
    IOParams.cpp)
