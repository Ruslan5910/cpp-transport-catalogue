#include "transport_router.h"

#include <cstdint>

int seconds_in_hour = 3600;
int meters_per_kilometer = 1000;
int seconds_in_minute = 60;

TransportRouter::TransportRouter(RouteSettings settings, const transport::TransportCatalogue& catalogue) {
    AddRouteSettings(settings);
    AddGraph(catalogue);
}

void TransportRouter::AddRouteSettings(RouteSettings settings) {
    settings_ = settings;
}

void TransportRouter::AddGraph(const transport::TransportCatalogue& catalogue) {
    size_t stop_number = 0;
    for (const auto& stop : catalogue.GetAllStops()) {
        stop_by_vertex_id_[stop_number] = stop.stop_name;
        vertex_id_by_stop_[stop.stop_name] = stop_number;
        ++stop_number;
    }
    graph::DirectedWeightedGraph<Weight> graph_temp(catalogue.GetAllStops().size());
    for (const auto& route : catalogue.GetAllRoutes()) {
        if (route.is_roundtrip == false) {
            for (size_t from = 0; from < route.route_stops.size(); ++from) {
                uint32_t route_segment_distance_l = 0;
                uint32_t route_segment_distance_r = 0;
                for (size_t to = from + 1; to < route.route_stops.size(); ++to) {
                    graph::Edge<Weight> edge1;
                    edge1.from = vertex_id_by_stop_.at(route.route_stops[from]);
                    edge1.to = vertex_id_by_stop_.at(route.route_stops[to]);
                    edge1.route_name = route.route_name;
                    route_segment_distance_l += catalogue.GetDistance(route.route_stops[to - 1],
                                                                      route.route_stops[to]);
                    edge1.weight = (route_segment_distance_l / ((settings_.bus_velocity / seconds_in_hour) * meters_per_kilometer) / seconds_in_minute) + settings_.bus_wait_time;
                       
                    graph::Edge<Weight> edge2;
                    edge2.from = vertex_id_by_stop_.at(route.route_stops[to]);
                    edge2.to = vertex_id_by_stop_.at(route.route_stops[from]);
                    edge2.route_name = route.route_name;
                    route_segment_distance_r += catalogue.GetDistance(route.route_stops[to],
                                                                      route.route_stops[to - 1]);
                    edge2.weight = (route_segment_distance_r / ((settings_.bus_velocity / seconds_in_hour) * meters_per_kilometer) / seconds_in_minute) + settings_.bus_wait_time;
                    
                    graph_temp.AddEdge(edge1);
                    graph_temp.AddEdge(edge2);
                }
            }
        } else {
            for (size_t from = 0; from < route.route_stops.size() - 1; ++from) {
                uint32_t route_segment_distance = 0;
                for (size_t to = from + 1; to < route.route_stops.size(); ++to) {
                    if (from == 0 && to == route.route_stops.size() - 1) {
                        continue;
                    }
                    graph::Edge<Weight> edge;
                    edge.from = vertex_id_by_stop_.at(route.route_stops[from]);
                    edge.to = vertex_id_by_stop_.at(route.route_stops[to]);
                    edge.route_name = route.route_name;
                    route_segment_distance += catalogue.GetDistance(route.route_stops[to - 1], route.route_stops[to]);
                    edge.weight = (route_segment_distance / ((settings_.bus_velocity / 3600) * 1000) / 60) + settings_.bus_wait_time;
                    graph_temp.AddEdge(edge);    
                }
            }
        }
    }
    graph_ = std::move(graph_temp);
    router_ = std::make_unique<graph::Router<Weight>>(graph_);
}

    std::optional<GraphResults> TransportRouter::BuildRoute(std::string_view from, std::string_view to) {
    GraphResults result;
    std::optional<graph::Router<Weight>::RouteInfo> route_result;
    size_t vertex_from = vertex_id_by_stop_.at(std::string(from));
    size_t vertex_to = vertex_id_by_stop_.at(std::string(to));
    route_result = router_->BuildRoute(vertex_from, vertex_to);
    if (!route_result) {
        return std::nullopt;
    }
    result.bus_wait_time = settings_.bus_wait_time;
    result.total_time = route_result->weight;
    for (const auto& edge_id : route_result->edges) {
        result.edge_info_by_id[stop_by_vertex_id_[graph_.GetEdge(edge_id).from]] = graph_.GetEdge(edge_id);
    }
    return result;
}
