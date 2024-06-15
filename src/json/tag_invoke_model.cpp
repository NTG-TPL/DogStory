#include "tag_invoke_model.h"

namespace model {
    using namespace detail;
    using namespace boost::json;
    using namespace std::string_literals;

    void tag_invoke(value_from_tag, value& json_value, const Road& road) {
        object obj;
        obj[MapKey::START_X] = road.GetStart().x;
        obj[MapKey::START_Y] = road.GetStart().y;
        if (road.IsHorizontal()) {
            obj[MapKey::END_X] = road.GetEnd().x;
        } else if(road.IsVertical()) {
            obj[MapKey::END_Y] = road.GetEnd().y;
        }

        json_value = std::move(value_from(obj));
    }

    void tag_invoke(value_from_tag, value& json_value, const Building& building) {
        json_value = {{MapKey::POS_X,  building.GetBounds().position.x},
                      {MapKey::POS_Y,  building.GetBounds().position.y},
                      {MapKey::WIDTH,  building.GetBounds().size.width},
                      {MapKey::HEIGHT, building.GetBounds().size.height}};
    }

    void tag_invoke(value_from_tag, value& json_value, const Office& office) {
        json_value = {{MapKey::ID,       *office.GetId()},
                      {MapKey::POS_X,    office.GetPosition().x}, {MapKey::POS_Y, office.GetPosition().y},
                      {MapKey::OFFSET_X, office.GetOffset().dx}, {MapKey::OFFSET_Y, office.GetOffset().dy}};
    }

    void tag_invoke(boost::json::value_from_tag, boost::json::value& json_value, const Loot& loot){
        json_value = {{LootKey::TYPE, loot.GetType()},
                      {LootKey::POSITION, {loot.GetPosition().x, loot.GetPosition().y}}};
    }

    void tag_invoke(value_from_tag, value& json_value, const LootType& loot_type){
        object obj;
        auto try_assign_value = [&](const auto& optional, boost::json::string_view key){
            try_assign(obj, optional, key);
        };
        try_assign_value(loot_type.name, LootKey::NAME);
        try_assign_value(loot_type.file, LootKey::FILE);
        try_assign_value(loot_type.type, LootKey::TYPE);
        try_assign_value(loot_type.rotation, LootKey::ROTATION);
        try_assign_value(loot_type.color, LootKey::COLOR);
        try_assign_value(loot_type.scale, LootKey::SCALE);
        obj[LootKey::VALUE] = loot_type.value;
        json_value = std::move(obj);
    }

    void tag_invoke(value_from_tag, value& json_value, const Map& map) {
        auto get_json_array = [](const auto& container){
            boost::json::array json_array;
            std::transform(container.cbegin(), container.cend(), std::back_inserter(json_array), [](auto& obj) {
                return value_from(obj);
            });
            return json_array;
        };

        object obj;
        obj[MapKey::ID] = *map.GetId();
        obj[MapKey::NAME] = map.GetName();
        obj[MapKey::ROADS] = value_from(get_json_array(map.GetRoads()));
        obj[MapKey::BUILDINGS] = value_from(get_json_array(map.GetBuildings()));
        obj[MapKey::OFFICES] = value_from(get_json_array(map.GetOffices()));
        obj[LootKey::LOOT_TYPES] = value_from(get_json_array(map.GetLootTypes()));
        json_value = std::move(obj);
    }

    void tag_invoke(boost::json::value_from_tag, boost::json::value& json_value, const Game::Maps& maps){
        array arr;
        for (const auto& map : maps) {
            value value = {{MapKey::ID, *map->GetId()}, {MapKey::NAME, map->GetName()}};
            arr.push_back(value);
        }
        json_value = std::move(arr);
    }

    void tag_invoke(boost::json::value_from_tag, boost::json::value& json_value, const Dog::Bag& bag){
        array js_bag;
        for (const auto& loot : bag) {
            object js_loot;
            js_loot[LootKey::ID] = *loot.id;
            js_loot[LootKey::TYPE] = loot.type;
            js_bag.push_back(js_loot);
        }
        json_value = js_bag;
    }

    void tag_invoke(boost::json::value_from_tag, boost::json::value& json_value, const Dog& dog){
        object obj;

        obj[UserKey::POSITION] = {dog.GetPosition().x, dog.GetPosition().y};
        obj[UserKey::SPEED] = {dog.GetSpeed().dx, dog.GetSpeed().dy};
        obj[UserKey::DIRECTION] = std::string{dog.GetDirection()};
        obj[LootKey::BAG] = value_from(dog.GetBag());
        obj[LootKey::SCORE] = dog.GetScore();
        json_value = std::move(obj);
    }

    Road tag_invoke(value_to_tag<Road>, const value& json_value) {
        auto& road = json_value.as_object();
        if (road.size() != 3) {
            throw std::logic_error(error_wrong_amount_keys + "Road"s);
        }

        Point2i start{};
        CoordInt finish{};
        bool vertical = true;
        for (auto& [key, value] : road) {
            if (key == MapKey::START_X) {
                start.x = static_cast<CoordInt>(value.as_int64());
            } else if (key == MapKey::START_Y) {
                start.y = static_cast<CoordInt>(value.as_int64());
            } else if (key == MapKey::END_X) {
                vertical = false;
                finish = static_cast<CoordInt>(value.as_int64());
            } else if (key == MapKey::END_Y) {
                vertical = true;
                finish = static_cast<CoordInt>(value.as_int64());
            } else {
                throw std::logic_error(error_unknown_key + "in the json Road"s);
            }
        }
        return vertical ? Road{Road::VERTICAL, start, finish} : Road{Road::HORIZONTAL, start, finish};
    }

    Building tag_invoke(value_to_tag<Building>, const value& json_value) {
        if (json_value.as_object().size() != 4) {
            throw std::logic_error(error_wrong_amount_keys + "Building");
        }

        return Building(Rectangle{Point2i{extruct<CoordInt>(json_value, MapKey::POS_X),
                                          extruct<CoordInt>(json_value, MapKey::POS_Y)},
                                  Size{extruct<DimensionInt>(json_value, MapKey::WIDTH),
                                       extruct<DimensionInt>(json_value, MapKey::HEIGHT)}});
    }

    Office tag_invoke(value_to_tag<Office>, const value& json_value) {
        return Office{Office::Id{extruct<std::string>(json_value, MapKey::ID)},
                      Point2i{extruct<CoordInt>(json_value, MapKey::POS_X),
                              extruct<CoordInt>(json_value, MapKey::POS_Y)},
                      Offset{extruct<DimensionInt>(json_value, MapKey::OFFSET_X),
                             extruct<DimensionInt>(json_value, MapKey::OFFSET_Y)}};
    }

    LootType tag_invoke(boost::json::value_to_tag<LootType>, const boost::json::value& json_value){
        LootType loot {try_extruct<std::string>(json_value, LootKey::NAME),
                       try_extruct<std::string>(json_value, LootKey::FILE),
                       try_extruct<std::string>(json_value, LootKey::TYPE),
                       try_extruct<DimensionInt>(json_value, LootKey::ROTATION),
                       try_extruct<std::string>(json_value, LootKey::COLOR),
                       try_extruct<DimensionDouble>(json_value, LootKey::SCALE),
                       extruct<DimensionInt>(json_value, LootKey::VALUE)};
        return loot;
    }

    Map tag_invoke(boost::json::value_to_tag<Map>, const boost::json::value& json_value){
        auto& json_obj = json_value.as_object();

        DimensionDouble default_dog_speed = json_obj.at(MapKey::DEFAULT_DOG_SPEED).as_double();
        size_t default_bag_capacity = json_obj.at(LootKey::DEFAULT_BAG_CAPACITY).as_uint64();

        DimensionDouble dog_speed = default_dog_speed;
        if(json_obj.contains(MapKey::DOG_SPEED)){
            dog_speed = json_obj.at(MapKey::DOG_SPEED).as_double();
        }

        size_t bag_capaciy = default_bag_capacity;
        if(json_obj.contains(LootKey::BAG_CAPACITY)){
            bag_capaciy = json_obj.at(LootKey::BAG_CAPACITY).as_uint64();
        }

        Map map(Map::Id(extruct<std::string>(json_value, MapKey::ID)),
                extruct<std::string>(json_value, MapKey::NAME), dog_speed, bag_capaciy);

        for (auto& [key, value]: json_obj) {
            if (key == MapKey::ID || key == MapKey::NAME) {
                continue;
            } else if (key == MapKey::ROADS) {
                for (auto& jroad : value.as_array()) {
                    map.AddRoad(value_to<Road>(jroad));
                }
            } else if (key == MapKey::BUILDINGS) {
                for (const auto& jbuilding : value.as_array()) {
                    map.AddBuilding(value_to<Building>(jbuilding));
                }
            } else if (key == MapKey::OFFICES) {
                for (const auto& joffice : value.as_array()) {
                    map.AddOffice(value_to<Office>(joffice));
                }
            } else if (key == LootKey::LOOT_TYPES) {
                for (auto& lootType : value.as_array()) {
                    map.AddLootType(value_to<LootType>(lootType));
                }
            }
        }
        return map;
    }

    Game tag_invoke(boost::json::value_to_tag<Game>, const boost::json::value& json_value){
        auto& json_obj = json_value.as_object();

        model::Game game;
        DimensionDouble default_dog_speed = DefaultValues::DOG_SPEED;
        if(json_obj.contains(MapKey::DEFAULT_DOG_SPEED)){
            default_dog_speed = json_obj.at(MapKey::DEFAULT_DOG_SPEED).as_double();
        }

        size_t default_bag_capacity = DefaultValues::BAG_CAPACITY;
        if(json_obj.contains(LootKey::DEFAULT_BAG_CAPACITY)){
            default_bag_capacity = json_obj.at(LootKey::DEFAULT_BAG_CAPACITY).as_uint64();
        }

        std::chrono::milliseconds default_retirement_time = DefaultValues::RETIREMENT_TIME;
        if(json_obj.contains(UserKey::RETIREMENT_TIME)){
            auto ms = static_cast<int64_t>(json_obj.at(UserKey::RETIREMENT_TIME).as_double() * DefaultValues::MS_IN_SECOND);
            default_retirement_time = std::chrono::milliseconds(ms);
        }
        game.SetDogRetirementTime(default_retirement_time);

        if (json_obj.contains(LootKey::LOOT_CONFIG)) {
            auto& loot = json_obj.at(LootKey::LOOT_CONFIG);
            game.SetLootGeneratorConfig(loot.at(LootKey::PERIOD).as_double(), loot.at(LootKey::PROBABILITY).as_double());
        } else {
            throw std::logic_error("There are no loot generation parameters in the configuration file");
        }

        auto maps = json_obj.at(MapKey::MAPS).as_array();
        for (auto& map: maps) {
            map.as_object()[LootKey::DEFAULT_BAG_CAPACITY] = default_bag_capacity;
            map.as_object()[MapKey::DEFAULT_DOG_SPEED] = default_dog_speed;
            game.AddMap(boost::json::value_to<model::Map>(map));
        }
        return game;
    }
}  // namespace model
