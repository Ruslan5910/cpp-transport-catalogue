#include "request_handler.h"

RequestHandler::RequestHandler(const transport::TransportCatalogue& db, const MapRenderer& renderer)
    : db_(db), renderer_(renderer) {}

std::optional<transport::TransportCatalogue::RouteInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    if (db_.GetRouteInfo(bus_name).count_stops == 0) {
        return std::nullopt;
    }
    return db_.GetRouteInfo(bus_name);
}

const std::unordered_set<std::string_view>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    return db_.GetRoutesThrowStop(stop_name);
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.MakeSvgDocument(db_.GetStops(), db_.GetRoutes());
}
