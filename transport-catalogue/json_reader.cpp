#include "json_reader.h"
#include "json_builder.h"
#include "transport_router.h"
#include "transport_router.h"

#include <algorithm>
#include <vector>
#include <string>
#include <sstream>

using namespace std::literals;

void JsonReader::AddTransportInfo(transport::TransportCatalogue& catalogue) const {
    for (const auto& [request_type, commands] : doc_.GetRoot().AsDict()) {
        if (request_type == "base_requests"s) {
            for (const auto& command : commands.AsArray()) {
                if (command.AsDict().at("type"s).AsString() == "Stop"s) {
                    catalogue.AddStopAndCoordinates(command.AsDict().at("name"s).AsString(),
                                                    {command.AsDict().at("latitude"s).AsDouble(),
                                                    command.AsDict().at("longitude"s).AsDouble()});
                }
            }
        }
    }
    for (const auto& [request_type, commands] : doc_.GetRoot().AsDict()) {
        if (request_type == "base_requests"s) {
            for (const auto& command : commands.AsArray()) {
                if (command.AsDict().at("type"s).AsString() == "Stop"s) {
                    for (const auto& [name_to, distance] : command.AsDict().at("road_distances"s).AsDict()) {
                        catalogue.AddDistance(command.AsDict().at("name"s).AsString(), name_to, distance.AsInt());
                    }
                }
            }
        }
    }
    for (const auto& [request_type, commands] : doc_.GetRoot().AsDict()) {
        if (request_type == "base_requests"s) {
            for (const auto& command : commands.AsArray()) {
                if (command.AsDict().at("type"s).AsString() == "Bus"s) {
                    std::vector<std::string> stops;
               //   stops.reserve(command.AsMap().at("stops").AsArray().size());
                    bool is_roundtrip = command.AsDict().at("is_roundtrip"s).AsBool();
                    for (const auto& stop : command.AsDict().at("stops"s).AsArray()) {
                        stops.push_back(stop.AsString());
                    }
                    catalogue.AddRouteAndStops(command.AsDict().at("name"s).AsString(), stops, is_roundtrip);
                }
            }
        }
    }
}

RouteSettings JsonReader::GetRouteSettings() const {
    RouteSettings route_settings;
    auto& routing_settings = doc_.GetRoot().AsDict().at("routing_settings"s).AsDict();
    route_settings.bus_velocity = routing_settings.at("bus_velocity"s).AsDouble();
    route_settings.bus_wait_time = routing_settings.at("bus_wait_time"s).AsInt();
    return route_settings;
}

json::Document JsonReader::GetDocument() const {
    return doc_;
}

void JsonReader::AddVisualSettings(MapRenderer& renderer) {
    VisualSettings settings;
    auto& render_settings = doc_.GetRoot().AsDict().at("render_settings"s).AsDict();
    settings.width = render_settings.at("width"s).AsDouble();
    settings.height = render_settings.at("height"s).AsDouble();
    settings.padding = render_settings.at("padding"s).AsDouble();
    settings.line_width = render_settings.at("line_width"s).AsDouble();
    settings.stop_radius = render_settings.at("stop_radius"s).AsDouble();
    settings.bus_label_font_size = render_settings.at("bus_label_font_size"s).AsInt();
    settings.bus_label_offset = {render_settings.at("bus_label_offset"s).AsArray()[0].AsDouble(),
                                 render_settings.at("bus_label_offset"s).AsArray()[1].AsDouble()};
    settings.stop_label_font_size = render_settings.at("stop_label_font_size"s).AsInt();
    settings.stop_label_offset = {render_settings.at("stop_label_offset"s).AsArray()[0].AsDouble(),
                                  render_settings.at("stop_label_offset"s).AsArray()[1].AsDouble()};
    
    if (render_settings.at("underlayer_color"s).IsString()) {
        settings.underlayer_color = render_settings.at("underlayer_color"s).AsString();
    } else if (render_settings.at("underlayer_color"s).IsArray()) {
        if (render_settings.at("underlayer_color"s).AsArray().size() == 3) {
            settings.underlayer_color = svg::Rgb{static_cast<uint8_t>(render_settings.at("underlayer_color"s).AsArray()[0].AsInt()),
                                                 static_cast<uint8_t>(render_settings.at("underlayer_color"s).AsArray()[1].AsInt()),
                                                 static_cast<uint8_t>(render_settings.at("underlayer_color"s).AsArray()[2].AsInt())};
        } else {
            settings.underlayer_color = svg::Rgba{static_cast<uint8_t>(render_settings.at("underlayer_color"s).AsArray()[0].AsInt()),
                                                  static_cast<uint8_t>(render_settings.at("underlayer_color"s).AsArray()[1].AsInt()),
                                                  static_cast<uint8_t>(render_settings.at("underlayer_color"s).AsArray()[2].AsInt()),
                                                  render_settings.at("underlayer_color"s).AsArray()[3].AsDouble()};
        }
    }
    settings.underlayer_width = render_settings.at("underlayer_width"s).AsDouble();
    std::vector<svg::Color> colors;
    for (const auto& color_element : render_settings.at("color_palette"s).AsArray()) {
        if (color_element.IsString()) {
            colors.push_back(color_element.AsString());
        } else {
            if (color_element.AsArray().size() == 3) {
                colors.push_back(svg::Rgb{static_cast<uint8_t>(color_element.AsArray()[0].AsInt()),
                                          static_cast<uint8_t>(color_element.AsArray()[1].AsInt()),
                                          static_cast<uint8_t>(color_element.AsArray()[2].AsInt())});
            } else {
                colors.push_back(svg::Rgba{static_cast<uint8_t>(color_element.AsArray()[0].AsInt()),
                                           static_cast<uint8_t>(color_element.AsArray()[1].AsInt()),
                                           static_cast<uint8_t>(color_element.AsArray()[2].AsInt()),
                                           color_element.AsArray()[3].AsDouble()});
            }
        }
    }
    settings.color_palette = colors;
    renderer.SetSettings(settings);
}

void JsonReader::PrintCatalogueInfo(const RequestHandler& catalogue, std::ostream& out) const {
    json::Array all_answers;
    for (const auto& [request_type, commands] : doc_.GetRoot().AsDict()) {
        if (request_type == "stat_requests"s) {
            for (const auto& command : commands.AsArray()) {
                if (command.AsDict().at("type"s).AsString() == "Stop"s) {
                    std::vector<std::string> routes_names;
                    const auto ptr = catalogue.GetBusesByStop(command.AsDict().at("name"s).AsString());
                    if (ptr) {
                        for (auto route_name : *ptr) {
                            routes_names.push_back(std::string(route_name));
                        }
                        std::sort(routes_names.begin(), routes_names.end());
                        json::Array arr;
                        for (const auto& name : routes_names) {
                            arr.push_back(name);
                        }
                        all_answers.push_back(json::Builder{}
                                                   .StartDict()
                                                       .Key("buses"s).Value(arr)
                                                       .Key("request_id"s).Value(command.AsDict().at("id"s).AsInt())
                                                   .EndDict()
                                                   .Build());                      
                    } else if (!ptr){
                        all_answers.push_back(json::Builder{}
                                                   .StartDict()
                                                       .Key("request_id"s).Value(command.AsDict().at("id"s).AsInt())
                                                       .Key("error_message"s).Value("not found"s)
                                                   .EndDict()
                                                   .Build());
                        continue;
                    }
                } else if (command.AsDict().at("type"s).AsString() == "Bus"s) {
                    const auto value = catalogue.GetBusStat(command.AsDict().at("name"s).AsString());
                    if (value.has_value()) {
                        all_answers.push_back(json::Builder{}
                                                   .StartDict()
                                                        .Key("curvature"s).Value(value->route_length/value->straight_line_distance)
                                                        .Key("request_id"s).Value(command.AsDict().at("id"s).AsInt())
                                                        .Key("route_length"s).Value(value->route_length)
                                                        .Key("stop_count"s).Value(int(value->count_stops))
                                                        .Key("unique_stop_count"s).Value(int(value->count_unique_stops))
                                                   .EndDict()
                                                   .Build());
                    } else if (!value) {
                        all_answers.push_back(json::Builder{}
                                                   .StartDict()
                                                       .Key("request_id"s).Value(command.AsDict().at("id"s).AsInt())
                                                       .Key("error_message"s).Value("not found"s)
                                                   .EndDict()
                                                   .Build());
                    }
                } else if (command.AsDict().at("type"s) == "Map"s) {
                    std::ostringstream oss;
                    catalogue.RenderMap().Render(oss);
                    std::string svg_doc = oss.str();
                    all_answers.push_back(json::Builder{}
                                               .StartDict()
                                                   .Key("map"s).Value(svg_doc)
                                                   .Key("request_id"s).Value(command.AsDict().at("id"s).AsInt())
                                               .EndDict()
                                               .Build());
                } else if (command.AsDict().at("type"s) == "Route"s) {
                    std::string from = command.AsDict().at("from"s).AsString();
                    std::string to = command.AsDict().at("to"s).AsString();
                    const auto& value = catalogue.BuildRoute(from, to);
                    if (value.has_value()) {
                        json::Array graph_results;
                        size_t span = 0;
                        for (const auto& [key, edge] : value->edge_info_by_id) {
                            size_t i = 0;
                            graph_results.push_back(json::Builder{}
                                                                .StartDict()
                                                                    .Key("stop_name"s).Value(key)
                                                                    .Key("time"s).Value(value->bus_wait_time)
                                                                    .Key("type"s).Value("Wait"s)
                                                               .EndDict()
                                                               .Build());
                            graph_results.push_back(json::Builder{}
                                                                .StartDict()
                                                                     .Key("bus"s).Value(edge.route_name)
                                                                     .Key("span_count"s).Value(static_cast<int>(edge.to - edge.from + 1 - span))
                                                                     .Key("time").Value(edge.weight - value->bus_wait_time)
                                                                     .Key("type"s).Value("Bus"s)
                                                                .EndDict()
                                                            .Build());
                            ++i;
                            span = edge.to - edge.from + 1;
                        }
                        all_answers.push_back(json::Builder{}
                                                   .StartDict()
                                                        .Key("items"s).Value(graph_results)
                                                        .Key("request_id"s).Value(command.AsDict().at("id"s).AsInt())
                                                        .Key("total_time"s).Value(value->total_time)
                                                    .EndDict()
                                                    .Build());
                    } else if (!value) {
                        all_answers.push_back(json::Builder{}
                                                   .StartDict()
                                                       .Key("request_id"s).Value(command.AsDict().at("id"s).AsInt())
                                                       .Key("error_message"s).Value("not found"s)
                                                   .EndDict()
                                                   .Build());
                    }
                }
            }
        }
    }
    json::Print(json::Document{json::Node{all_answers}}, out);
};
