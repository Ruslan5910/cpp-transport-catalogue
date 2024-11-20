#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

#include <optional>

class RequestHandler {
public:
   // RequestHandler(const transport::TransportCatalogue& catalogue);
    
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const transport::TransportCatalogue& db, const MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<transport::TransportCatalogue::RouteInfo> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::unordered_set<std::string_view>* GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const transport::TransportCatalogue& db_;
    const MapRenderer& renderer_;
};
