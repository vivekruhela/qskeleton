include_directories(
	${DEPS_DIR}
    ../ddm
    ../UIUtils
    ../utils)

add_executable(ddmUI
    main.cpp)

if (UNIX)
    target_link_libraries(ddmUI
        rt
        -Wl,-whole-archive
		ddm
		UIUtils
        -Wl,-no-whole-archive)
else()
    target_link_libraries(ddmUI
		ddm
        UIUtils)
endif()
