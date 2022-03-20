/**
 * src/class_diagram/generators/plantuml/class_diagram_generator.h
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
#pragma once

#include "class_diagram/model/class.h"
#include "class_diagram/model/diagram.h"
#include "class_diagram/model/enum.h"
#include "class_diagram/visitor/translation_unit_visitor.h"
#include "common/generators/plantuml/generator.h"
#include "common/model/relationship.h"
#include "config/config.h"
#include "cx/compilation_database.h"
#include "util/util.h"

#include <cppast/cpp_entity_index.hpp>
#include <cppast/libclang_parser.hpp>
#include <glob/glob.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace clanguml {
namespace class_diagram {
namespace generators {
namespace plantuml {

using diagram_config = clanguml::config::class_diagram;
using diagram_model = clanguml::class_diagram::model::diagram;
template <typename C, typename D>
using common_generator =
    clanguml::common::generators::plantuml::generator<C, D>;

using clanguml::class_diagram::model::class_;
using clanguml::class_diagram::model::enum_;
using clanguml::common::model::package;
using clanguml::common::model::relationship_t;
using clanguml::common::model::scope_t;

using namespace clanguml::util;

class generator : public common_generator<diagram_config, diagram_model> {
public:
    generator(diagram_config &config, diagram_model &model);

    void generate_link(
        std::ostream &ostr, const class_diagram::model::class_element &e) const;

    void generate_alias(const class_ &c, std::ostream &ostr) const;

    void generate_alias(const enum_ &e, std::ostream &ostr) const;

    void generate(const class_ &c, std::ostream &ostr,
        std::ostream &relationships_ostr) const;

    void generate(const enum_ &e, std::ostream &ostr,
        std::ostream &relationships_ostr) const;

    void generate(const package &p, std::ostream &ostr,
        std::ostream &relationships_ostr) const;

    void generate(std::ostream &ostr) const override;
};

}
}
}
}
