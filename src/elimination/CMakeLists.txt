add_library(elimination INTERFACE)

# A hack to add the interface library headers to IDE
set(elimination_headers
	Elimination.hpp
	Inequality.hpp
	Order.hpp
	Parameters.hpp)
add_custom_target(elimination_ide SOURCES ${elimination_headers})