include_directories(
	${DEPS_DIR}
    ../elimination
    ../ddm
    ../utils
    ../UIUtils)

add_executable(eliminationUI
    main.cpp)

if (UNIX)
    target_link_libraries(eliminationUI
        rt
        -Wl,-whole-archive
		elimination
        UIUtils
        -Wl,-no-whole-archive)
else()
    target_link_libraries(eliminationUI
		elimination
        UIUtils)
endif()

