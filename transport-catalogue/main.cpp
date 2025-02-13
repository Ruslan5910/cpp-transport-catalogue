#include <iostream>

#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"

int main() { 
    transport::TransportCatalogue catalogue;
    JsonReader reader(std::cin);
    reader.AddTransportInfo(catalogue);
    MapRenderer renderer;
    reader.AddVisualSettings(renderer);

    RouteSettings settings = reader.GetRouteSettings();
    TransportRouter transport_router(settings, catalogue);

    RequestHandler handler(catalogue, renderer, transport_router);

    reader.PrintCatalogueInfo(handler, std::cout);
}
