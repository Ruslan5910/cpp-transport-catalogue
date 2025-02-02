#pragma once

#include "graph.h"
#include "ranges.h"
#include "router.h"
#include "transport_catalogue.h"

#include <vector>
#include <string>
#include <string_view>
#include <memory>
#include <map>
#include <optional>

using Weight = double;

struct RouteSettings {
    double bus_velocity = 0.;
    int bus_wait_time = 0;
};

struct GraphResults {
    std::map<std::string, graph::Edge<Weight>> edge_info_by_id;
    int bus_wait_time;
    double total_time = 0;
};

class TransportRouter {
public:
    void AddRouteSettings(RouteSettings settings);
    
    void AddGraph(const transport::TransportCatalogue& catalogue);
    
    std::optional<GraphResults> BuildRoute(std::string_view from, std::string_view to);
    
private:
    std::map<size_t, std::string> stop_by_vertex_id_;
    std::map<std::string, size_t> vertex_id_by_stop_;
    RouteSettings settings_;
    graph::DirectedWeightedGraph<Weight> graph_;
    std::unique_ptr<graph::Router<Weight>> router_;
};
