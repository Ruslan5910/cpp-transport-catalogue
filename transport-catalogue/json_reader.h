#pragma once

#include "request_handler.h"
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

class JsonReader {
public:
    JsonReader(std::istream& in)
        : doc_(json::Load(in)) {}
    
    void AddTransportInfo(transport::TransportCatalogue& catalogue) const;
    
    void AddRouteSettings(TransportRouter& transport_router) const;
    
    json::Document GetDocument() const;
    
    void AddVisualSettings(MapRenderer& settings);
    
    void PrintCatalogueInfo(const RequestHandler& catalogue_, std::ostream& out) const;
    
private:
    json::Document doc_;
};
