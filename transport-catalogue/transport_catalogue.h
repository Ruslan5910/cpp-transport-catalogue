#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <deque>

#include "geo.h"
#include "domain.h"

namespace transport {
    
class TransportCatalogue {
public:   
    
    struct RouteInfo {
        size_t count_stops = 0;
        size_t count_unique_stops = 0;
        int route_length = 0;
        double straight_line_distance = 0.;
    };
    
    struct Hasher {
        size_t operator()(const std::pair<const Stop*, const Stop*>& stops) const {
            std::hash<std::string> s_hasher;
            size_t s1 = (s_hasher(stops.first->stop_name) * 37) + s_hasher(stops.second->stop_name);
            return s1;
        }
    };
    
    // добавление маршрута в базу
    void AddRouteAndStops(const std::string& route, const std::vector<std::string>& stops, bool is_roundtrip);
    
    // добавление остановки в базу
    void AddStopAndCoordinates(const std::string& stop, const geo::Coordinates& coordinates);
    
    // добавление дистнции между остановками
    void AddDistance(std::string_view, std::string_view, int distance);
    
    // поиск маршрута по имени
    const Bus* FindRouteByName(std::string_view route_name) const;
    
    // поиск остановки по имени
    const Stop* FindStopByName(std::string_view stop_name) const;
    
    // получение информации о маршруте
    RouteInfo GetRouteInfo(std::string_view route_name) const;
    
    // получение дистанции между остановками
    int GetDistance(std::string_view from, std::string_view to) const;
    
    const std::deque<Bus> GetAllRoutes() const;
    
    const std::deque<Stop> GetAllStops() const;
    
    // получить маршруты проходящие через остановку
    const std::unordered_set<std::string_view>* GetRoutesThrowStop(std::string_view stop_name) const;
    
    const std::unordered_map<std::string_view, const Stop*> GetStops() const;
    
    const std::unordered_map<std::string_view, const Bus*> GetRoutes() const;
    
private:
    std::deque<Stop> stops_;
    std::deque<Bus> routes_;
    std::unordered_map<std::string_view, const Stop*> stop_info_by_stop_name_;
    std::unordered_map<std::string_view, const Bus*> route_info_by_route_name_;
    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> routes_throw_stop_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, Hasher> distance_between_stops_;
};

} // namespace transport
