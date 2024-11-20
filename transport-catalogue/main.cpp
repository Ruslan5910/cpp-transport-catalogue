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
    RequestHandler handler(catalogue, renderer);
    reader.PrintCatalogueInfo(handler, std::cout);
}
