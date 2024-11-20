#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

//RequestHandler::RequestHandler(const transport::TransportCatalogue& catalogue)
  //      : db_(catallogue) {}
    

RequestHandler::RequestHandler(const transport::TransportCatalogue& db, const MapRenderer& renderer)
    : db_(db), renderer_(renderer) {}

// Возвращает информацию о маршруте (запрос Bus)
std::optional<transport::TransportCatalogue::RouteInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    if (db_.GetRouteInfo(bus_name).count_stops == 0) {
        return std::nullopt;
    }
    return db_.GetRouteInfo(bus_name);
}

// Возвращает маршруты, проходящие через остановку
const std::unordered_set<std::string_view>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    return db_.GetRoutesThrowStop(stop_name);
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.MakeSvgDocument(db_.GetStops(), db_.GetRoutes());
}
