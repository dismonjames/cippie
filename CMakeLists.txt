cmake_minimum_required(VERSION 3.25)

project(
    Cippie
    VERSION 0.1.5
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
    src/build/BuildGraph.cpp
    src/build/BuildPlanner.cpp
    src/build/SourceScanner.cpp
    src/build/TargetBuilder.cpp
    src/build/TaskScheduler.cpp
    src/build/ThreadPool.cpp
    src/cache/BuildCache.cpp
    src/cache/DepFileParser.cpp
    src/cache/FileHasher.cpp
    src/cache/ManifestStore.cpp
    src/cli/CommandLine.cpp
    src/config/CippiefileEditor.cpp
    src/config/ConfigLoader.cpp
    src/config/Lexer.cpp
    src/config/Parser.cpp
    src/config/Validator.cpp
    src/diagnostics/DiagnosticPrinter.cpp
    src/diagnostics/Logger.cpp
    src/package/DependencyResolver.cpp
    src/package/GitFetcher.cpp
    src/package/HttpFetcher.cpp
    src/package/LockFile.cpp
    src/package/PackageCache.cpp
    src/package/PackageLock.cpp
    src/package/PackageManager.cpp
    src/package/PackageRegistry.cpp
    src/package/SemanticVersion.cpp
    src/package/VersionRequirement.cpp
    src/platform/Platform.cpp
    src/process/Process.cpp
    src/project/ProjectGenerator.cpp
    src/project/ProjectLocator.cpp
    src/project/TargetSelector.cpp
    src/toolchain/TargetTriple.cpp
    src/toolchain/ToolchainDetector.cpp
    src/toolchain/ToolchainRegistry.cpp
    src/util/ArchiveExtractor.cpp
    src/util/BuildLock.cpp
    src/util/CleanRunner.cpp
    src/util/UpdateChecker.cpp
    src/util/Path.cpp
    src/util/SHA256.cpp
)

if(WIN32)
    target_sources(cippie_core PRIVATE src/platform/windows/WindowsProcessRunner.cpp)
else()
    target_sources(cippie_core PRIVATE src/platform/linux/LinuxProcessRunner.cpp)
endif()

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
            /D_CRT_SECURE_NO_WARNINGS
        >
)

add_executable(cippie src/main.cpp)
target_link_libraries(cippie PRIVATE cippie_core)

install(
    TARGETS cippie
    RUNTIME DESTINATION bin
)

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

    add_executable(
        cippie_build_graph_tests
        tests/unit/BuildGraphTests.cpp
    )
    target_link_libraries(cippie_build_graph_tests PRIVATE cippie_core)
    add_test(NAME build_graph COMMAND cippie_build_graph_tests)

    add_executable(
        cippie_include_propagation_tests
        tests/unit/IncludePropagationTests.cpp
    )
    target_link_libraries(cippie_include_propagation_tests PRIVATE cippie_core)
    add_test(NAME include_propagation COMMAND cippie_include_propagation_tests)

    add_executable(
        cippie_artifact_naming_tests
        tests/unit/ArtifactNamingTests.cpp
    )
    target_link_libraries(cippie_artifact_naming_tests PRIVATE cippie_core)
    add_test(NAME artifact_naming COMMAND cippie_artifact_naming_tests)

    add_executable(
        cippie_clean_safety_tests
        tests/unit/CleanSafetyTests.cpp
    )
    target_link_libraries(cippie_clean_safety_tests PRIVATE cippie_core)
    add_test(NAME clean_safety COMMAND cippie_clean_safety_tests)

    add_executable(
        cippie_project_name_tests
        tests/unit/ProjectNameTests.cpp
    )
    target_link_libraries(cippie_project_name_tests PRIVATE cippie_core)
    add_test(NAME project_name COMMAND cippie_project_name_tests)

    add_executable(
        cippie_file_hasher_tests
        tests/unit/FileHasherTests.cpp
    )
    target_link_libraries(cippie_file_hasher_tests PRIVATE cippie_core)
    add_test(NAME file_hasher COMMAND cippie_file_hasher_tests)

    add_executable(
        cippie_cache_key_tests
        tests/unit/CacheKeyTests.cpp
    )
    target_link_libraries(cippie_cache_key_tests PRIVATE cippie_core)
    add_test(NAME cache_key COMMAND cippie_cache_key_tests)

    add_executable(
        cippie_dep_file_parser_tests
        tests/unit/DepFileParserTests.cpp
    )
    target_link_libraries(cippie_dep_file_parser_tests PRIVATE cippie_core)
    add_test(NAME dep_file_parser COMMAND cippie_dep_file_parser_tests)

    add_executable(
        cippie_manifest_store_tests
        tests/unit/ManifestStoreTests.cpp
    )
    target_link_libraries(cippie_manifest_store_tests PRIVATE cippie_core)
    add_test(NAME manifest_store COMMAND cippie_manifest_store_tests)

    add_executable(
        cippie_job_count_parsing_tests
        tests/unit/JobCountParsingTests.cpp
    )
    target_link_libraries(cippie_job_count_parsing_tests PRIVATE cippie_core)
    add_test(NAME job_count_parsing COMMAND cippie_job_count_parsing_tests)

    add_executable(
        cippie_task_scheduler_tests
        tests/unit/TaskSchedulerTests.cpp
    )
    target_link_libraries(cippie_task_scheduler_tests PRIVATE cippie_core)
    add_test(NAME task_scheduler COMMAND cippie_task_scheduler_tests)

    add_executable(
        cippie_captured_output_tests
        tests/unit/CapturedOutputTests.cpp
    )
    target_link_libraries(cippie_captured_output_tests PRIVATE cippie_core)
    add_test(NAME captured_output COMMAND cippie_captured_output_tests)

    add_executable(
        cippie_build_lock_tests
        tests/unit/BuildLockTests.cpp
    )
    target_link_libraries(cippie_build_lock_tests PRIVATE cippie_core)
    add_test(NAME build_lock COMMAND cippie_build_lock_tests)

    add_executable(
        cippie_semantic_version_tests
        tests/unit/SemanticVersionTests.cpp
    )
    target_link_libraries(cippie_semantic_version_tests PRIVATE cippie_core)
    add_test(NAME semantic_version COMMAND cippie_semantic_version_tests)

    add_executable(
        cippie_version_requirement_tests
        tests/unit/VersionRequirementTests.cpp
    )
    target_link_libraries(cippie_version_requirement_tests PRIVATE cippie_core)
    add_test(NAME version_requirement COMMAND cippie_version_requirement_tests)

    add_executable(
        cippie_archive_security_tests
        tests/unit/ArchiveSecurityTests.cpp
    )
    target_link_libraries(cippie_archive_security_tests PRIVATE cippie_core)
    add_test(NAME archive_security COMMAND cippie_archive_security_tests)

    add_executable(
        cippie_sha256_tests
        tests/unit/SHA256Tests.cpp
    )
    target_link_libraries(cippie_sha256_tests PRIVATE cippie_core)
    add_test(NAME sha256 COMMAND cippie_sha256_tests)

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

    add_test(
        NAME static_library
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/StaticLibraryTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME shared_library
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/SharedLibraryTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME multi_exec_static_lib
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/MultiExecStaticLibTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME test_target
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/TestTargetTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME project_new
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/ProjectNewTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME clean
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/CleanTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME incremental_build
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/IncrementalBuildTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME parallel_build
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/ParallelBuildTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME clean_cache
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/CleanCacheTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME lock_conflict
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/LockConflictTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME path_package
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/PathPackageTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME registry_package
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/RegistryPackageTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME cippie_add_remove
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/CippieAddRemoveTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_executable(
        cippie_target_triple_tests
        tests/unit/TargetTripleTests.cpp
    )
    target_link_libraries(cippie_target_triple_tests PRIVATE cippie_core)
    add_test(NAME target_triple COMMAND cippie_target_triple_tests)

    add_executable(
        cippie_toolchain_detector_tests
        tests/unit/ToolchainDetectorTests.cpp
    )
    target_link_libraries(cippie_toolchain_detector_tests PRIVATE cippie_core)
    add_test(NAME toolchain_detector COMMAND cippie_toolchain_detector_tests)

    add_test(
        NAME doctor
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/DoctorTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME cross_target_cache_isolation
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/CrossTargetCacheIsolationTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME self_host_first_gen
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/SelfHostFirstGenTest.sh"
            "$<TARGET_FILE:cippie>"
            "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    add_test(
        NAME self_host_second_gen
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/SelfHostSecondGenTest.sh"
            "$<TARGET_FILE:cippie>"
            "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    add_executable(
        cippie_version_consistency_tests
        tests/unit/VersionConsistencyTests.cpp
    )
    target_link_libraries(cippie_version_consistency_tests PRIVATE cippie_core)
    add_test(NAME version_consistency COMMAND cippie_version_consistency_tests)

    add_executable(
        cippie_exit_code_tests
        tests/unit/ExitCodeTests.cpp
    )
    target_link_libraries(cippie_exit_code_tests PRIVATE cippie_core)
    add_test(NAME exit_code COMMAND cippie_exit_code_tests)

    add_test(
        NAME installation
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/InstallationTest.sh"
            "$<TARGET_FILE:cippie>"
    )

    add_test(
        NAME install_script
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/InstallScriptTest.sh"
    )
endif()
