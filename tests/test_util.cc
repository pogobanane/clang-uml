/**
 * tests/test_util.cc
 *
 * Copyright (c) 2021-2022 Bartek Kryza <bkryza@gmail.com>
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
#define CATCH_CONFIG_MAIN

#include "util/util.h"
#include <cx/util.h>

#include "catch.h"

TEST_CASE("Test split", "[unit-test]")
{
    using C = std::vector<std::string>;
    using namespace clanguml::util;

    const C empty{};

    CHECK(split("", " ") == C{""});
    CHECK(split("ABCD", " ") == C{"ABCD"});

    CHECK(split("std::vector::detail", "::") == C{"std", "vector", "detail"});

    CHECK(split("std::vector::detail::", "::") == C{"std", "vector", "detail"});
}

TEST_CASE("Test abbreviate", "[unit-test]")
{
    using namespace clanguml::util;

    CHECK(abbreviate("", 10) == "");
    CHECK(abbreviate("abcd", 10) == "abcd");
    CHECK(abbreviate("abcd", 2) == "ab");
    CHECK(abbreviate("abcdefg", 5) == "ab...");
}

TEST_CASE("Test replace_all", "[unit-test]")
{
    using namespace clanguml::util;

    const std::string orig = R"(
                        Lorem ipsum \clanguml{note} something something...

                        \clanguml{style}

                        )";
    std::string text{orig};

    CHECK(replace_all(text, "NOTHERE", "HERE") == false);
    CHECK(replace_all(text, "\\clanguml", "@clanguml") == true);
    CHECK(replace_all(text, "something", "nothing") == true);
    CHECK(replace_all(text, "something", "nothing") == false);
    CHECK(replace_all(text, "nothing", "something") == true);
    CHECK(replace_all(text, "@clanguml", "\\clanguml") == true);

    CHECK(text == orig);
}

TEST_CASE("Test parse_unexposed_template_params", "[unit-test]")
{
    using namespace clanguml::cx::util;

    const std::string int_template_str{"ns1::ns2::class1<int>"};

    auto int_template = parse_unexposed_template_params(
        int_template_str, [](const auto &n) { return n; });

    CHECK(int_template.size() == 1);
    CHECK(int_template[0].template_params_.size() == 1);
    CHECK(int_template[0].type() == "ns1::ns2::class1");
    CHECK(int_template[0].template_params_[0].type() == "int");

    const std::string int_int_template_str{"ns1::ns2::class1<int, int>"};

    auto int_int_template = parse_unexposed_template_params(
        int_int_template_str, [](const auto &n) { return n; });

    CHECK(int_int_template.size() == 1);
    CHECK(int_int_template[0].template_params_.size() == 2);
    CHECK(int_int_template[0].type() == "ns1::ns2::class1");
    CHECK(int_int_template[0].template_params_[0].type() == "int");
    CHECK(int_int_template[0].template_params_[1].type() == "int");

    const std::string nested_template_str{
        "class1<int, ns1::class2<int, std::vector<std::string>>>"};

    auto nested_template = parse_unexposed_template_params(
        nested_template_str, [](const auto &n) { return n; });

    CHECK(nested_template.size() == 1);
    CHECK(nested_template[0].template_params_.size() == 2);
    CHECK(nested_template[0].type() == "class1");
    CHECK(nested_template[0].template_params_[0].type() == "int");
    const auto &class2 = nested_template[0].template_params_[1];
    CHECK(class2.type() == "ns1::class2");
    CHECK(class2.template_params_[0].type() == "int");
    CHECK(class2.template_params_[1].type() == "std::vector");
    CHECK(
        class2.template_params_[1].template_params_[0].type() == "std::string");
}
