#include "transport_catalogue.h"

#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <set>

using namespace std::literals;

namespace transport {
    // метод AddRouteAndStops принимает название маршрута и вектор его остановок
    // заполняется приватное поле routes_ структурами {route, stops}
    // в словарь route_info_by_route_name_, который содержит вид на string и структуру Bus, создаю ключ (название маршрута) и закидываю в него структуру
void TransportCatalogue::AddRouteAndStops(const std::string& route, const std::vector<std::string>& stops, bool is_roundtrip) {
    routes_.push_back({route, stops, is_roundtrip});
    route_info_by_route_name_[routes_.back().route_name] = &routes_.back();
    for (const std::string& stop : stops) {
        routes_throw_stop_.at(stop).insert(routes_.back().route_name); 
    }
}
    
void TransportCatalogue::AddStopAndCoordinates(const std::string& stop, const geo::Coordinates& coordinates) {
    stops_.push_back({stop, coordinates});
    stop_info_by_stop_name_[stops_.back().stop_name] = &stops_.back();
    routes_throw_stop_[stops_.back().stop_name];
}
    
void TransportCatalogue::AddDistance(std::string_view from, std::string_view to, int distance) {
    const auto from_ptr = FindStopByName(from);
    const auto to_ptr = FindStopByName(to);
    distance_between_stops_[{from_ptr, to_ptr}] = distance;
    if (distance_between_stops_.find({to_ptr, from_ptr}) == distance_between_stops_.end()) {
        distance_between_stops_[{to_ptr, from_ptr}] = distance;
    }
}
    
const Bus* TransportCatalogue::FindRouteByName(std::string_view route_name) const {
    auto it = route_info_by_route_name_.find(route_name);
    if (it  == route_info_by_route_name_.end()) {
        return nullptr;
    }
    return it->second;
}
    
const Stop* TransportCatalogue::FindStopByName(std::string_view stop_name) const {
    auto it = stop_info_by_stop_name_.find(stop_name);
    if (it == stop_info_by_stop_name_.end()) {
        return nullptr;
    }
    return it->second;
}
    
TransportCatalogue::RouteInfo TransportCatalogue::GetRouteInfo(std::string_view route_name) const {    
    if (route_info_by_route_name_.find(route_name) == route_info_by_route_name_.end()) {
        return {};
    }
    RouteInfo route_info;
    std::unordered_set<std::string> unique_stops;
    const Bus* route_info_by_name = route_info_by_route_name_.at(route_name);
    if (route_info_by_name->is_roundtrip == true) {
        route_info.count_stops = route_info_by_name->route_stops.size();
        for (size_t i = 0; i < route_info_by_name->route_stops.size(); ++i) {
            unique_stops.insert(route_info_by_name->route_stops[i]);
        }
        route_info.count_unique_stops = unique_stops.size();
        for (size_t i = 0; i < route_info_by_name->route_stops.size() - 1; ++i) {
            route_info.route_length += GetDistance(FindStopByName(route_info_by_name->route_stops[i])->stop_name, 
                                                   FindStopByName(route_info_by_name->route_stops[i + 1])->stop_name);
            route_info.straight_line_distance += geo::ComputeDistance((FindStopByName(route_info_by_name->route_stops[i]))->stop_coordinates,  
                                                                  (FindStopByName(route_info_by_name->route_stops[i + 1]))->stop_coordinates);
        }
    } else if (route_info_by_name->is_roundtrip == false) {
        route_info.count_stops = (route_info_by_name->route_stops.size() * 2) - 1;
        for (size_t i = 0; i < route_info_by_name->route_stops.size(); ++i) {
            unique_stops.insert(route_info_by_name->route_stops[i]);
        }
        route_info.count_unique_stops = unique_stops.size();
        for (size_t i = 0; i < route_info_by_name->route_stops.size() - 1; ++i) {
            route_info.route_length += GetDistance(FindStopByName(route_info_by_name->route_stops[i])->stop_name, 
                                               FindStopByName(route_info_by_name->route_stops[i + 1])->stop_name);
            route_info.straight_line_distance += geo::ComputeDistance((FindStopByName(route_info_by_name->route_stops[i]))->stop_coordinates,  
                                                                  (FindStopByName(route_info_by_name->route_stops[i + 1]))->stop_coordinates) * 2;
        }
        for (int i = route_info_by_name->route_stops.size() - 1; i > 0; --i) {
            route_info.route_length += GetDistance(FindStopByName(route_info_by_name->route_stops[i])->stop_name,
                                                   FindStopByName(route_info_by_name->route_stops[i - 1])->stop_name);
        }
    }
    return route_info;
}

    
int TransportCatalogue::GetDistance(std::string_view from, std::string_view to) const {
    const Stop* from_ptr = FindStopByName(from);
    const Stop* to_ptr = FindStopByName(to);
    auto it = distance_between_stops_.find({from_ptr, to_ptr});
    if (it == distance_between_stops_.end()) {
        throw std::invalid_argument("Таких остановок нет"s);
    }
    return it->second;
}   
    
       
const std::unordered_set<std::string_view>* TransportCatalogue::GetRoutesThrowStop(std::string_view stop_name) const {
    auto it = routes_throw_stop_.find(stop_name);
    if (it == routes_throw_stop_.end()) {
        return nullptr;
    } 
    return &(it->second);
}
    
const std::unordered_map<std::string_view, const Stop*> TransportCatalogue::GetStops() const {
    return stop_info_by_stop_name_;
}
    
const std::unordered_map<std::string_view, const Bus*> TransportCatalogue::GetRoutes() const {
    return route_info_by_route_name_;
}

} //namespace transport
