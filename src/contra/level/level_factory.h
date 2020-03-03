//
// Created by david on 3/3/20.
//

#ifndef CONTRA_LEVEL_FACTORY_H
#define CONTRA_LEVEL_FACTORY_H

#include <string>
#include <memory>
#include <yaml-cpp/yaml.h>
#include "scrolling_level.h"
#include "perspective_level.h"

class LevelFactory {
private:
    const std::unordered_map<int, std::shared_ptr<Sprite>> *spritesheets;
    PlayerStats *stats;
    AvancezLib *engine;
public:
    LevelFactory(const std::unordered_map<int, std::shared_ptr<Sprite>> *spritesheets, short numPlayers,
                 PlayerStats *stats, AvancezLib *engine) : spritesheets(spritesheets),
                                                           stats(stats), engine(engine) {}

    Level *LoadLevel(const std::string &folder, short num_players) {
        SDL_Log("LevelLoader::LoadLevel(%s, %d players)", &folder[0], num_players);
        Level *level = nullptr;
        try {
            YAML::Node scene_root = YAML::LoadFile(folder + "/level.yaml");
            if (scene_root["level_type"]) {
                char level_type = scene_root["level_type"].as<char>();
                switch (level_type) {
                    case 'S': {
                        level = new ScrollingLevel();
                        level->Create(folder, spritesheets, scene_root, num_players, stats, engine);
                        break;
                    }
                    case 'P': {
                        level = new PerspectiveLevel();
                        level->Create(folder, spritesheets, scene_root, num_players, stats, engine);
                        break;
                    }
                    default:
                        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unrecognised level type '%c'", level_type);
                        break;
                }
            } else {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                        "Invalid level '%s/level.yaml', no level_type found.", &folder[0]);
            }
        } catch (YAML::BadFile &exception) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load level file: %s/level.yaml", &folder[0]);
        }
        return level;
    }
};

#endif //CONTRA_LEVEL_FACTORY_H
