/**
 * tests/test_filters.cc
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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

#include "class_diagram/model/class.h"
#include "cli/cli_handler.h"
#include "common/model/diagram_filter.h"
#include "common/model/source_file.h"
#include "config/config.h"

#include "include_diagram/model/diagram.h"

#include <filesystem>

TEST_CASE("Test diagram paths filter", "[unit-test]")
{
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::source_file;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["include_test"];
    clanguml::include_diagram::model::diagram diagram;

    diagram_filter filter(diagram, config);

    auto make_path = [&](std::string_view p) {
        return source_file{config.relative_to() / p};
    };

    CHECK(filter.should_include(
        make_path("class_diagram/visitor/translation_unit_visitor.h")));
    CHECK(filter.should_include(make_path("main.cc")));
    CHECK(filter.should_include(make_path("util/util.cc")));
    CHECK_FALSE(filter.should_include(make_path("util/error.h")));
    CHECK_FALSE(filter.should_include(
        make_path("sequence_diagram/visitor/translation_unit_visitor.h")));
}

TEST_CASE("Test method_types include filter", "[unit-test]")
{
    using clanguml::class_diagram::model::class_method;
    using clanguml::common::model::access_t;
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::source_file;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["method_type_include_test"];
    clanguml::class_diagram::model::diagram diagram;

    diagram_filter filter(diagram, config);

    class_method cm{access_t::kPublic, "A", ""};
    cm.is_constructor(true);

    CHECK(filter.should_include(cm));

    cm.is_constructor(false);
    cm.is_destructor(true);

    CHECK(!filter.should_include(cm));
}

TEST_CASE("Test method_types exclude filter", "[unit-test]")
{
    using clanguml::class_diagram::model::class_method;
    using clanguml::common::model::access_t;
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::source_file;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["method_type_exclude_test"];
    clanguml::class_diagram::model::diagram diagram;

    diagram_filter filter(diagram, config);

    class_method cm{access_t::kPublic, "A", ""};

    CHECK(filter.should_include(cm));

    cm.is_constructor(true);

    CHECK(filter.should_include(cm));

    cm.is_constructor(false);
    cm.is_destructor(true);

    CHECK(!filter.should_include(cm));
}

TEST_CASE("Test elements regexp filter", "[unit-test]")
{
    using clanguml::class_diagram::model::class_method;
    using clanguml::common::model::access_t;
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::namespace_;
    using clanguml::common::model::source_file;

    using clanguml::class_diagram::model::class_;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["regex_elements_test"];
    clanguml::class_diagram::model::diagram diagram;

    diagram_filter filter(diagram, config);

    class_ c{{}};

    c.set_namespace(namespace_{"ns1"});
    c.set_name("ClassA");

    CHECK(filter.should_include(c));

    c.set_namespace(namespace_{"ns1::ns2"});
    c.set_name("ClassA");

    CHECK(filter.should_include(c));

    c.set_namespace(namespace_{"ns1::ns2"});
    c.set_name("ClassZ");

    CHECK(!filter.should_include(c));

    c.set_namespace(namespace_{"ns1::ns5::ns3"});
    c.set_name("ClassA");

    CHECK(filter.should_include(c));
}

TEST_CASE("Test namespaces regexp filter", "[unit-test]")
{
    using clanguml::class_diagram::model::class_method;
    using clanguml::class_diagram::model::class_parent;
    using clanguml::common::model::access_t;
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::namespace_;
    using clanguml::common::model::package;
    using clanguml::common::model::source_file;

    using clanguml::class_diagram::model::class_;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["regex_namespace_test"];
    clanguml::class_diagram::model::diagram diagram;

    diagram_filter filter(diagram, config);

    class_ c{{}};

    c.set_namespace(namespace_{"ns1::ns2"});
    c.set_name("ClassA");

    CHECK(filter.should_include(c));

    c.set_namespace(namespace_{"ns1::ns2::detail"});
    c.set_name("ClassAImpl");

    CHECK(!filter.should_include(c));

    c.set_namespace(namespace_{"ns1::interface"});
    c.set_name("IClassA");

    CHECK(filter.should_include(c));

    CHECK(!filter.should_include(namespace_{"ns1"}));
    CHECK(filter.should_include(namespace_{"ns1::ns2"}));
    CHECK(!filter.should_include(namespace_{"ns1::ns2::detail"}));
    CHECK(filter.should_include(namespace_{"ns1::interface"}));

    package p{{}};

    p.set_namespace({"ns1"});
    p.set_name("ns2");

    CHECK(filter.should_include(p));

    p.set_namespace({"ns1::ns2"});
    p.set_name("detail");

    CHECK(!filter.should_include(p));

    p.set_namespace({"ns1"});
    p.set_name("interface");

    CHECK(filter.should_include(p));
}

TEST_CASE("Test subclasses regexp filter", "[unit-test]")
{
    using clanguml::class_diagram::model::class_method;
    using clanguml::class_diagram::model::class_parent;
    using clanguml::common::to_id;
    using clanguml::common::model::access_t;
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::namespace_;
    using clanguml::common::model::package;
    using clanguml::common::model::source_file;
    using namespace std::string_literals;

    using clanguml::class_diagram::model::class_;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["regex_subclasses_test"];
    clanguml::class_diagram::model::diagram diagram;

    auto p = std::make_unique<package>(config.using_namespace());
    p->set_namespace({});
    p->set_name("ns1");
    diagram.add({}, std::move(p));
    p = std::make_unique<package>(config.using_namespace());
    p->set_namespace({"ns1"});
    p->set_name("ns2");
    diagram.add(namespace_{"ns1"}, std::move(p));

    auto c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("BaseA");
    c->set_id(to_id("ns1::ns2::BaseA"s));
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("A1");
    c->set_id(to_id("ns1::ns2::A1"s));
    c->add_parent({"ns1::ns2::BaseA"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("A2");
    c->set_id(to_id("ns1::ns2::A2"s));
    c->add_parent({"ns1::ns2::BaseA"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("BaseB");
    c->set_id(to_id("ns1::ns2::BaseB"s));
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("B1");
    c->set_id(to_id("ns1::ns2::B1"s));
    c->add_parent({"ns1::ns2::BaseB"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("B2");
    c->set_id(to_id("ns1::ns2::B2"s));
    c->add_parent({"ns1::ns2::BaseB"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("Common");
    c->set_id(to_id("ns1::ns2::Common"s));
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("C1");
    c->set_id(to_id("ns1::ns2::C1"s));
    c->add_parent({"ns1::ns2::Common"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    diagram.set_complete(true);

    diagram_filter filter(diagram, config);

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("A1");
    c->set_id(to_id("ns1::ns2::A1"s));
    CHECK(filter.should_include(*c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("B1");
    c->set_id(to_id("ns1::ns2::B1"s));
    CHECK(filter.should_include(*c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("C1");
    c->set_id(to_id("ns1::ns2::C1"s));
    CHECK(!filter.should_include(*c));
}

TEST_CASE("Test parents regexp filter", "[unit-test]")
{
    using clanguml::class_diagram::model::class_method;
    using clanguml::class_diagram::model::class_parent;
    using clanguml::common::to_id;
    using clanguml::common::model::access_t;
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::namespace_;
    using clanguml::common::model::package;
    using clanguml::common::model::source_file;
    using namespace std::string_literals;

    using clanguml::class_diagram::model::class_;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["regex_parents_test"];
    clanguml::class_diagram::model::diagram diagram;

    auto p = std::make_unique<package>(config.using_namespace());
    p->set_namespace({});
    p->set_name("ns1");
    diagram.add({}, std::move(p));
    p = std::make_unique<package>(config.using_namespace());
    p->set_namespace({"ns1"});
    p->set_name("ns2");
    diagram.add(namespace_{"ns1"}, std::move(p));

    auto c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("BaseA");
    c->set_id(to_id("ns1::ns2::BaseA"s));
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("A1");
    c->set_id(to_id("ns1::ns2::A1"s));
    c->add_parent({"ns1::ns2::BaseA"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("A2");
    c->set_id(to_id("ns1::ns2::A2"s));
    c->add_parent({"ns1::ns2::BaseA"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("BaseB");
    c->set_id(to_id("ns1::ns2::BaseB"s));
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("B1");
    c->set_id(to_id("ns1::ns2::B1"s));
    c->add_parent({"ns1::ns2::BaseB"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("B2");
    c->set_id(to_id("ns1::ns2::B2"s));
    c->add_parent({"ns1::ns2::BaseB"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("Common");
    c->set_id(to_id("ns1::ns2::Common"s));
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("C3");
    c->set_id(to_id("ns1::ns2::C3"s));
    c->add_parent({"ns1::ns2::Common"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    diagram.set_complete(true);

    diagram_filter filter(diagram, config);

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("BaseA");
    c->set_id(to_id("ns1::ns2::BaseA"s));
    CHECK(filter.should_include(*c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("BaseB");
    c->set_id(to_id("ns1::ns2::BaseB"s));
    CHECK(filter.should_include(*c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("Common");
    c->set_id(to_id("ns1::ns2::Common"s));
    CHECK(!filter.should_include(*c));
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
