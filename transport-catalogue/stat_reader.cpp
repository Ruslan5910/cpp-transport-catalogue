#include "stat_reader.h"

namespace transport {
    
void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) {
    TransportCatalogue::RouteInfo route_info;
    auto space = request.find(' ');
    auto last_char = request.find_last_not_of(' ');
    std::string line = std::string(request.substr(space + 1, last_char + 1 - space));
    std::string command = std::string(request.substr(0, space));
    if (command == "Bus") {
        route_info = transport_catalogue.GetRouteInfo(line);
        if (route_info.count_stops == 0) {
            output << std::string("Bus ") << line << std::string(": not found") << std::endl;
        } else {
            output << std::string("Bus ") << line << std::string(": ") << route_info.count_stops << std::string(" stops on route, ") 
               << route_info.count_unique_stops << std::string(" unique stops, ") << route_info.route_length << std::string(" route length") << std::endl;
        }
    } else if (command == "Stop") {
         if (transport_catalogue.FindStopByName(line) == nullptr) {
            output << std::string("Stop ") << line << std::string(": not found") << std::endl;
             return;
        }
        const auto result = std::set<std::string_view>(transport_catalogue.GetRoutesThrowStop(line).begin(), transport_catalogue.GetRoutesThrowStop(line).end());
        if (result.empty()) {
            output << std::string("Stop ") << line << std::string(": no buses") << std::endl;
        } else {
            output << std::string("Stop ") << line << std::string(": buses");
            for (const std::string_view& route : result) {
                output << std::string(" ") << route;
            }
            output << std::endl;
        }
    } else {
        throw std::invalid_argument("unknown command");
    }
}

    
} // namespace transport
