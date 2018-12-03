cmake_minimum_required(VERSION 2.8)

add_library(demo1 demo1.cpp)
add_library(demo2 demo2.cpp)

set(LIBNAME "libcombined.a")

add_custom_command(
        OUTPUT ${LIBNAME}
        COMMAND ar -rcT ${LIBNAME} $<TARGET_FILE:demo1> $<TARGET_FILE:demo2>
        DEPENDS demo1 demo2
        COMMENT "Combining libs..."
)

add_custom_target(combinedLibGenerator
        DEPENDS ${LIBNAME}
        )

add_library(combinedLib STATIC IMPORTED)
set_property(TARGET combinedLib PROPERTY IMPORTED_LOCATION ${LIBNAME})
add_dependencies(combinedLib combinedLibGenerator)

add_executable(mainexec main.cpp)
target_link_libraries(mainexec combinedLib)
