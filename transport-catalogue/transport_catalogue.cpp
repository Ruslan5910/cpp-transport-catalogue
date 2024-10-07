#include "transport_catalogue.h"

#include <algorithm>

namespace transport {
    // метод AddRouteAndStops принимает название маршрута и вектор его остановок
    // заполняется приватное поле routes_ структурами {route, stops}
    // в словарь route_info_by_route_name_, который содержит вид на string и структуру Bus, создаю ключ (название маршрута) и закидываю в него структуру
void TransportCatalogue::AddRouteAndStops(const std::string& route, const std::vector<std::string>& stops) {
    routes_.push_back({route, stops});
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
    
const TransportCatalogue::Bus* TransportCatalogue::FindRouteByName(std::string_view route_name) const {
    if (route_info_by_route_name_.find(route_name) == route_info_by_route_name_.end()) {
        return nullptr;
    }
    return route_info_by_route_name_.at(route_name);
}
    
const TransportCatalogue::Stop* TransportCatalogue::FindStopByName(std::string_view stop_name) const {
    if (stop_info_by_stop_name_.find(stop_name) == stop_info_by_stop_name_.end()) {
        return nullptr;
    }
    return stop_info_by_stop_name_.at(stop_name);
}
    
TransportCatalogue::RouteInfo TransportCatalogue::GetRouteInfo(std::string_view route_name) const {    
    if (route_info_by_route_name_.find(route_name) == route_info_by_route_name_.end()) {
        return {};
    }
    
    RouteInfo route_info;
    std::unordered_set<std::string> unique_stops;
    
    route_info.count_stops = route_info_by_route_name_.at(route_name)->route_stops.size();
    
    for (int i = 0; i < route_info_by_route_name_.at(route_name)->route_stops.size(); ++i) {
        unique_stops.insert(route_info_by_route_name_.at(route_name)->route_stops[i]);
    }
    
    route_info.count_unique_stops = unique_stops.size();
    
    for (int i = 0; i < route_info_by_route_name_.at(route_name)->route_stops.size() - 1; ++i) {
        route_info.route_length += geo::ComputeDistance((FindStopByName(route_info_by_route_name_.at(route_name)->route_stops[i]))->stop_coordinates,
                                                        (FindStopByName(route_info_by_route_name_.at(route_name)->route_stops[i + 1]))->stop_coordinates);
    }
    return route_info;
}
       
std::unordered_set<std::string_view> TransportCatalogue::GetRoutesThrowStop(const std::string& stop_name) const {
    if (routes_throw_stop_.find(stop_name) == routes_throw_stop_.end()) {
        return {};
    } 
    return routes_throw_stop_.at(stop_name);
}

} //namespace transport
