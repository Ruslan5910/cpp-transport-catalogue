#include "stat_reader.h"
#include <string_view>

namespace transport {
    
void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::ostream& output, std::istream& input) {
    int stat_request_count;
    input >> stat_request_count >> std::ws;
    for (int i = 0; i < stat_request_count; ++i) {
        std::string request;
        std::getline(input, request);     
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
                << route_info.count_unique_stops << std::string(" unique stops, ") << route_info.route_length << std::string(" route length, ") << route_info.route_length/route_info.straight_line_distance << " curvature"<<std::endl;
            }
        } else if (command == "Stop") {
            auto result = transport_catalogue.GetRoutesThrowStop(line);        
            if (!result) {
                output << std::string("Stop ") << line << std::string(": not found") << std::endl;
            } else if (result->empty()) {
                output << std::string("Stop ") << line << std::string(": no buses") << std::endl;
            } else {
                output << std::string("Stop ") << line << std::string(": buses");
                for (const std::string_view& route : std::set<std::string_view>(result->begin(), result->end())) {
                    output << std::string(" ") << route;
                }
                output << std::endl;
            }
        } else {
            throw std::invalid_argument("unknown command");
        }
    }
}
    
} // namespace transport
