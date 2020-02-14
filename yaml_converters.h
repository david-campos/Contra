//
// Created by david on 14/2/20.
//

#ifndef CONTRA_YAML_CONVERTERS_H
#define CONTRA_YAML_CONVERTERS_H

#include "vector2D.h"
#include "scene.h"
#include <yaml-cpp/yaml.h>

namespace YAML {
    template<>
    struct convert<Vector2D> {
        static bool decode(const Node& node, Vector2D& rhs) {
            if(!node.IsSequence() || node.size() != 2) {
                return false;
            }

            rhs.x = node[0].as<double>();
            rhs.y = node[1].as<double>();
            return true;
        }
    };
}

#endif //CONTRA_YAML_CONVERTERS_H
