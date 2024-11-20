#include "json_reader.h"

#include <algorithm>
#include <vector>
#include <string>
#include <sstream>

using namespace std::literals;

void JsonReader::AddTransportInfo(transport::TransportCatalogue& catalogue) const {
    for (const auto& [request_type, commands] : doc_.GetRoot().AsMap()) {
        if (request_type == "base_requests"s) {
            for (const auto& command : commands.AsArray()) {
                if (command.AsMap().at("type"s).AsString() == "Stop"s) {
                    catalogue.AddStopAndCoordinates(command.AsMap().at("name"s).AsString(),
                                                    {command.AsMap().at("latitude"s).AsDouble(),
                                                    command.AsMap().at("longitude"s).AsDouble()});
                }
            }
        }
    }
    for (const auto& [request_type, commands] : doc_.GetRoot().AsMap()) {
        if (request_type == "base_requests"s) {
            for (const auto& command : commands.AsArray()) {
                if (command.AsMap().at("type"s).AsString() == "Stop"s) {
                    for (const auto& [name_to, distance] : command.AsMap().at("road_distances"s).AsMap()) {
                        catalogue.AddDistance(command.AsMap().at("name"s).AsString(), name_to, distance.AsInt());
                    }
                }
            }
        }
    }
    for (const auto& [request_type, commands] : doc_.GetRoot().AsMap()) {
        if (request_type == "base_requests"s) {
            for (const auto& command : commands.AsArray()) {
                if (command.AsMap().at("type"s).AsString() == "Bus"s) {
                    std::vector<std::string> stops;
               //   stops.reserve(command.AsMap().at("stops").AsArray().size());
                    bool is_roundtrip = command.AsMap().at("is_roundtrip"s).AsBool();
                    for (const auto& stop : command.AsMap().at("stops"s).AsArray()) {
                        stops.push_back(stop.AsString());
                    }
                    catalogue.AddRouteAndStops(command.AsMap().at("name"s).AsString(), stops, is_roundtrip);
                }
            }
        }
    }
}

json::Document JsonReader::GetDocument() const {
    return doc_;
}

void JsonReader::AddVisualSettings(MapRenderer& renderer) {
    VisualSettings settings;
    auto& render_settings = doc_.GetRoot().AsMap().at("render_settings"s).AsMap();
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

/*
void JsonReader::AddStopsAndBusesForRendering(const RequestHandler& handler, const MapRender render) {
    std::vector<Bus> buses_for_rendering;
    std::vector<Stop> stops_for_rendering;
    auto base_commands = doc_.GetRoot().AsMap().at("base_requests"s).AsArray();
    for (const auto command : commands) {
        if (command.AsMap().at("type"s).AsString() == "Stop"s) {
            stops_for_rendering.push_back({command.AsMap().at("name"s).AsString(), 
                                          {command.AsMap().at("latitude"s).AsDouble(), command.AsMap().at("longitude"s).AsDouble()}});
        } else if (command.AsMap().at("type"s).AsString() == "Bus"s) {
            std::vector<std::string> stops;
            for (const auto& stop : command.AsMap().at("stops"s).AsArray()) {
                stops.push_back(stop.AsString());
            }
            buses_for_rendering.push_back({command.AsMap().at("name").AsStirng(), stops});
        }
    }
    render(buses_for_rendering, stops_for_rendering);
}
*/

void JsonReader::PrintCatalogueInfo(const RequestHandler& catalogue, std::ostream& out) const {
    json::Array all_answers;
    for (const auto& [request_type, commands] : doc_.GetRoot().AsMap()) {
        if (request_type == "stat_requests"s) {
            for (const auto& command : commands.AsArray()) {
                if (command.AsMap().at("type"s).AsString() == "Stop"s) {
                    std::vector<std::string> routes_names;
                    const auto ptr = catalogue.GetBusesByStop(command.AsMap().at("name"s).AsString());
                    if (ptr) {
                        for (auto route_name : *ptr) {
                            routes_names.push_back(std::string(route_name));
                        }
                        std::sort(routes_names.begin(), routes_names.end());
                        json::Array arr;
                        for (const auto& name : routes_names) {
                            arr.push_back(name);
                        }
                        all_answers.push_back(json::Dict{{"buses"s, arr}, {"request_id"s, command.AsMap().at("id"s).AsInt()}});                       
                    } else if (!ptr){
                        all_answers.push_back(json::Dict{{"request_id"s, command.AsMap().at("id"s).AsInt()}, {"error_message"s, "not found"s}});
                        continue;
                    }
                } else if (command.AsMap().at("type"s).AsString() == "Bus"s) {
                    const auto value = catalogue.GetBusStat(command.AsMap().at("name"s).AsString());
                    if (value.has_value()) {
                        all_answers.push_back(json::Dict{{"curvature"s, value->route_length/value->straight_line_distance},
                                                         {"request_id"s, command.AsMap().at("id"s).AsInt()},
                                                         {"route_length"s, value->route_length},
                                                         {"stop_count"s, int(value->count_stops)},
                                                         {"unique_stop_count"s, int(value->count_unique_stops)}});
                    } else if (!value) {
                        all_answers.push_back(json::Dict{{"request_id"s, command.AsMap().at("id"s).AsInt()}, {"error_message"s, "not found"s}});
                    }
                } else if (command.AsMap().at("type"s) == "Map"s) {
                    std::ostringstream oss;
                    catalogue.RenderMap().Render(oss);
                    std::string svg_doc = oss.str();
                    all_answers.push_back(json::Dict{{"map"s, svg_doc}, {"request_id"s, command.AsMap().at("id"s).AsInt()}});
                }
            }
        }
    }
    json::Print(json::Document{json::Node{all_answers}}, out);
}
