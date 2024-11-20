#pragma once

#include "geo.h"

#include <string>
#include <vector>

struct Stop {
    std::string stop_name;
    geo::Coordinates stop_coordinates;
};

struct Bus {
    std::string route_name;
    std::vector<std::string> route_stops;
    bool is_roundtrip = false;
};
