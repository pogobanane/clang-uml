/**
 * @file tests/test_compilation_database.cc
 *
 * Copyright (c) 2021-2024 Bartek Kryza <bkryza@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_CONSOLE_WIDTH 512

#include "catch.h"

#include "cli/cli_handler.h"
#include "common/compilation_database.h"
#include "util/util.h"

#include "catch.h"
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>

std::shared_ptr<spdlog::logger> make_sstream_logger(std::ostream &ostr)
{
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(ostr);
    return std::make_shared<spdlog::logger>(
        "clanguml-logger", std::move(oss_sink));
}

TEST_CASE("Test compilation_database should work", "[unit-test]")
{
    using clanguml::common::compilation_database;
    using clanguml::common::compilation_database_ptr;
    using clanguml::common::model::access_t;
    using clanguml::common::model::relationship_t;
    using clanguml::util::contains;
    using std::filesystem::path;

    // This is executed by cmake in the directory `<BUILD_DIRECTORY>/tests`
    auto cfg =
        clanguml::config::load("./test_compilation_database_data/config.yml");

    try {
        const auto db =
            clanguml::common::compilation_database::auto_detect_from_directory(
                cfg);

        auto all_files = db->getAllFiles();

        REQUIRE(all_files.size() == 3);
        REQUIRE(contains(all_files,
            path("src/class_diagram/generators/json/class_diagram_generator.cc")
                .make_preferred()
                .string()));
        REQUIRE(contains(all_files,
            path("src/class_diagram/generators/plantuml/"
                 "class_diagram_generator.cc")
                .make_preferred()
                .string()));
        REQUIRE(contains(all_files,
            path("src/class_diagram/model/class.cc")
                .make_preferred()
                .string()));

        auto ccs = db->getAllCompileCommands();

        REQUIRE(contains(ccs.at(0).CommandLine, "-Wno-error"));
        REQUIRE(contains(ccs.at(0).CommandLine, "-Wno-unknown-warning-option"));
        REQUIRE(
            !contains(ccs.at(0).CommandLine, "-Wno-deprecated-declarations"));
    }
    catch (clanguml::error::compilation_database_error &e) {
        REQUIRE(false);
    }
}

TEST_CASE("Test compilation_database should throw", "[unit-test]")
{
    using clanguml::common::compilation_database;
    using clanguml::common::compilation_database_ptr;
    using clanguml::error::compilation_database_error;
    using clanguml::util::contains;

    auto cfg = clanguml::config::load(
        "./test_compilation_database_data/config_bad.yml");

    REQUIRE_THROWS_AS(
        clanguml::common::compilation_database::auto_detect_from_directory(cfg),
        compilation_database_error);
}

///
/// Main test function
///
int main(int argc, char *argv[])
{
    Catch::Session session;
    using namespace Catch::clara;

    bool debug_log{false};
    auto cli = session.cli() |
        Opt(debug_log, "debug_log")["-u"]["--debug-log"]("Enable debug logs");

    session.cli(cli);

    int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0)
        return returnCode;

    clanguml::cli::cli_handler clih;

    std::vector<const char *> argvv = {
        "clang-uml", "--config", "./test_config_data/simple.yml"};

    if (debug_log)
        argvv.push_back("-vvv");
    else
        argvv.push_back("-q");

    clih.handle_options(argvv.size(), argvv.data());

    return session.run();
}
