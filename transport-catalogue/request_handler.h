#pragma once 

#include "map_renderer.h" 

#include "transport_catalogue.h" 
#include "transport_router.h" 

#include <optional> 
 
class RequestHandler { 
public: 
    RequestHandler(const transport::TransportCatalogue& db, const MapRenderer& renderer, TransportRouter& transport_router); 

    std::optional<transport::TransportCatalogue::RouteInfo> GetBusStat(const std::string_view& bus_name) const; 

    const std::unordered_set<std::string_view>* GetBusesByStop(const std::string_view& stop_name) const; 

    svg::Document RenderMap() const; 

    std::optional<GraphResults> BuildRoute(std::string_view from, std::string_view to) const; 

private: 
    const transport::TransportCatalogue& db_; 
    const MapRenderer& renderer_; 
    TransportRouter& transport_router_; 
}; 
