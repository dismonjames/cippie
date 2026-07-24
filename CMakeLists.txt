cmake_minimum_required(VERSION 3.25)

project(
    Cippie
    VERSION 0.1.0
    DESCRIPTION "A C++ build system and package manager"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CIPPIE_BUILD_TESTS "Build Cippie tests" ON)

add_library(
    cippie_core
    src/app/Application.cpp
    src/build/BuildEngine.cpp
    src/build/BuildPlanner.cpp
    src/build/SourceScanner.cpp
    src/cache/BuildCache.cpp
    src/cli/CommandLine.cpp
    src/config/ConfigLoader.cpp
    src/config/Lexer.cpp
    src/config/Parser.cpp
    src/config/Validator.cpp
    src/diagnostics/DiagnosticPrinter.cpp
    src/diagnostics/Logger.cpp
    src/package/PackageManager.cpp
    src/platform/Platform.cpp
    src/platform/linux/LinuxProcessRunner.cpp
    src/process/Process.cpp
    src/project/ProjectLocator.cpp
    src/project/TargetSelector.cpp
    src/toolchain/TargetTriple.cpp
    src/toolchain/ToolchainDetector.cpp
    src/util/Path.cpp
)

target_include_directories(
    cippie_core
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_features(
    cippie_core
    PUBLIC
        cxx_std_23
)

target_compile_options(
    cippie_core
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wshadow
        >
        $<$<CXX_COMPILER_ID:MSVC>:
            /W4
            /permissive-
        >
)

add_executable(cippie src/main.cpp)
target_link_libraries(cippie PRIVATE cippie_core)

if(CIPPIE_BUILD_TESTS)
    enable_testing()

    add_executable(
        cippie_source_scanner_tests
        tests/unit/SourceScannerTests.cpp
    )
    target_link_libraries(cippie_source_scanner_tests PRIVATE cippie_core)
    add_test(NAME source_scanner COMMAND cippie_source_scanner_tests)

    add_executable(
        cippie_config_lexer_tests
        tests/unit/ConfigLexerTests.cpp
    )
    target_link_libraries(cippie_config_lexer_tests PRIVATE cippie_core)
    add_test(NAME config_lexer COMMAND cippie_config_lexer_tests)

    add_executable(
        cippie_config_parser_tests
        tests/unit/ConfigParserTests.cpp
    )
    target_link_libraries(cippie_config_parser_tests PRIVATE cippie_core)
    add_test(NAME config_parser COMMAND cippie_config_parser_tests)

    add_executable(
        cippie_config_validator_tests
        tests/unit/ConfigValidatorTests.cpp
    )
    target_link_libraries(cippie_config_validator_tests PRIVATE cippie_core)
    add_test(NAME config_validator COMMAND cippie_config_validator_tests)

    add_executable(
        cippie_target_selector_tests
        tests/unit/TargetSelectorTests.cpp
    )
    target_link_libraries(cippie_target_selector_tests PRIVATE cippie_core)
    add_test(NAME target_selector COMMAND cippie_target_selector_tests)

    add_executable(
        cippie_build_planner_tests
        tests/unit/BuildPlannerTests.cpp
    )
    target_link_libraries(cippie_build_planner_tests PRIVATE cippie_core)
    add_test(NAME build_planner COMMAND cippie_build_planner_tests)

    add_executable(
        cippie_process_runner_tests
        tests/unit/ProcessRunnerTests.cpp
    )
    target_link_libraries(cippie_process_runner_tests PRIVATE cippie_core)
    add_test(NAME process_runner COMMAND cippie_process_runner_tests)

    add_test(
        NAME smoke
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/SmokeTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME build_single_executable
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/BuildSingleExecutableTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME run_arguments
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/RunArgumentsTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME compiler_failure
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/CompilerFailureTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME linker_failure
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/LinkerFailureTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME subdirectory_run
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/SubdirectoryRunTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME invalid_config
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/InvalidConfigTest.sh"
            "$<TARGET_FILE:cippie>"
    )
endif()
