#pragma once

#include "request_handler.h"
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
class JsonReader {
public:
    JsonReader(std::istream& in)
        : doc_(json::Load(in)) {}
    
    void AddTransportInfo(transport::TransportCatalogue& catalogue) const;
    
    json::Document GetDocument() const;
    
    void AddVisualSettings(MapRenderer& settings);
    
 // void AddStopsAndBusesForRendering(RequestHandler& handler, MapRender& render);
    
    void PrintCatalogueInfo(const RequestHandler& catalogue_, std::ostream& out) const;
    
private:
    json::Document doc_;
};
