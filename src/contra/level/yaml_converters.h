//
// Created by david on 14/2/20.
//

#ifndef CONTRA_YAML_CONVERTERS_H
#define CONTRA_YAML_CONVERTERS_H

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
    template<>
    struct convert<PickUpType> {
        static bool decode(const Node& node, PickUpType& powerUp) {
            switch(node.as<std::string>()[0]) {
                case 'M':
                    powerUp = PICKUP_MACHINE_GUN;
                    break;
                case 'F':
                    powerUp = PICKUP_FIRE_GUN;
                    break;
                case 'R':
                    powerUp = PICKUP_RAPID_FIRE;
                    break;
                case 'L':
                    powerUp = PICKUP_LASER;
                    break;
                case 'S':
                    powerUp = PICKUP_SPREAD;
                    break;
                case 'B':
                    powerUp = PICKUP_BARRIER;
                    break;
                default:
                    return false;
            }
            return true;
        }
    };
}

#endif //CONTRA_YAML_CONVERTERS_H
