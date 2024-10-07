#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <deque>

#include "geo.h"

namespace transport {
    
class TransportCatalogue {
public:   

    struct Stop {
        std::string stop_name;
        geo::Coordinates stop_coordinates;
    };

    struct Bus {
        std::string route_name;
        std::vector<std::string> route_stops;
    };
    
    struct RouteInfo {
        size_t count_stops = 0;
        size_t count_unique_stops = 0;
        double route_length = 0.;
    };
    
    // добавление маршрута в базу
    void AddRouteAndStops(const std::string& route, const std::vector<std::string>& stops);
    
    // добавление остановки в базу
    void AddStopAndCoordinates(const std::string& stop, const geo::Coordinates& coordinates);
    
    // поиск маршрута по имени
    const Bus* FindRouteByName(std::string_view route_name) const;
    
    // поиск остановки по имени
    const Stop* FindStopByName(std::string_view stop_name) const;
    
    // получение информации о маршруте
    RouteInfo GetRouteInfo(std::string_view route_name) const;
    
    // получить маршруты проходящие через остановку
    const std::unordered_set<std::string_view>* GetRoutesThrowStop(std::string_view stop_name) const;
    
private:
    std::deque<Stop> stops_;
    std::deque<Bus> routes_;
    std::unordered_map<std::string_view, const Stop*> stop_info_by_stop_name_;
    std::unordered_map<std::string_view, const Bus*> route_info_by_route_name_;
    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> routes_throw_stop_;
};

} // namespace transport
