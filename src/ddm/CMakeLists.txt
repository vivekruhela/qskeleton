add_library(ddm INTERFACE)

# A hack to add the interface library headers to IDE
set(ddm_headers
	AdjacencyChecker.hpp
	Algorithm.hpp
	Parameters.hpp
	Pivoting.hpp
	Ray.hpp
	Summary.hpp)
add_custom_target(ddm_ide SOURCES ${ddm_headers})
