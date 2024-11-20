#include "map_renderer.h"

#include <map>
#include <algorithm>
#include <utility>
#include <set>

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

struct StringFirstComparator {
    bool operator()(const std::pair<std::string, geo::Coordinates>& lhs,
                    const std::pair<std::string, geo::Coordinates>& rhs) const {
        return lhs.first < rhs.first; 
    }
};

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

void MapRenderer::SetSettings(VisualSettings settings) {
    settings_ = settings;
}
svg::Document MapRenderer::MakeSvgDocument(std::unordered_map<std::string_view, const Stop*> stop_info_by_stop_name,
                                           std::unordered_map<std::string_view, const Bus*> route_info_by_route_name) const {
    svg::Document doc;
    size_t color_palette_index = 0;
    std::vector<geo::Coordinates> all_coordinates;
    all_coordinates.reserve(stop_info_by_stop_name.size());
    std::vector<std::vector<geo::Coordinates>> coordinates_for_render;
    coordinates_for_render.reserve(route_info_by_route_name.size());
    for (const auto& [stop_name, stop_struct] : stop_info_by_stop_name) {
        for (const auto& [key, value] : route_info_by_route_name) {
            if (std::find(value->route_stops.begin(), value->route_stops.end(), stop_name) != value->route_stops.end()) {
                all_coordinates.push_back(stop_struct->stop_coordinates);
            }
        }
    }
    SphereProjector projector(all_coordinates.begin(), all_coordinates.end(),
                              settings_.width, settings_.height, settings_.padding);
    
    std::map<std::string_view, const Bus*> sorted_by_name_routes_info(route_info_by_route_name.begin(),
                                                                      route_info_by_route_name.end());
    
    //---------------------------------------------------------------ОТРИСОВКА ЛИНИЙ МАРШРУТА----------------------------------------------------------------
    for (const auto& [route_name, route_struct] : sorted_by_name_routes_info) {
        if (route_struct->route_stops.size()) {
            std::vector<geo::Coordinates> route_stops_coordinates;
            if (route_struct->is_roundtrip) {
                route_stops_coordinates.reserve(route_struct->route_stops.size());
            } else {
                route_stops_coordinates.reserve(route_struct->route_stops.size() * 2 - 1);
            }
            for (const std::string& route_stop : route_struct->route_stops) {
                route_stops_coordinates.push_back(stop_info_by_stop_name.at(route_stop)->stop_coordinates);
            }
            if (!route_struct->is_roundtrip) {
                route_stops_coordinates.insert(route_stops_coordinates.end(),
                                               std::next(route_stops_coordinates.rbegin()), 
                                               route_stops_coordinates.rend());
            }
            coordinates_for_render.push_back(route_stops_coordinates);
        }
    }
    for (const auto& vec_coord : coordinates_for_render) {
        svg::Polyline polyline;
        for (const auto& coord : vec_coord) {
            polyline.AddPoint(projector(coord));
        }
        polyline.SetFillColor(svg::NoneColor)
                .SetStrokeColor(settings_.color_palette[color_palette_index])
                .SetStrokeWidth(settings_.line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        doc.Add(polyline);
        ++color_palette_index;
        if (color_palette_index == settings_.color_palette.size()) {
            color_palette_index = 0;
        }
    }
    
    //-------------------------------------------------------------ОТРИСОВКА НАЗВАНИЙ МАРШРУТОВ----------------------------------------------------------------
    std::vector<std::vector<std::pair<std::string, geo::Coordinates>>> all_route_names_for_render;
    all_route_names_for_render.reserve(sorted_by_name_routes_info.size());
    for (const auto& [route_name, route_struct] : sorted_by_name_routes_info) {
        std::vector<std::pair<std::string, geo::Coordinates>> route_for_render;
        route_for_render.reserve(sorted_by_name_routes_info.size() * 2);
        if (route_struct->route_stops.size()) {
            if (route_struct->is_roundtrip) {
                route_for_render.push_back(std::make_pair(std::string(route_name),
                                                          stop_info_by_stop_name.at(route_struct->route_stops.back())->stop_coordinates));
            } else if (!(route_struct->is_roundtrip) && (route_struct->route_stops.front() != route_struct->route_stops.back())) {
                route_for_render.push_back(std::make_pair(std::string(route_name),
                                                          stop_info_by_stop_name.at(route_struct->route_stops.front())->stop_coordinates));
                route_for_render.push_back(std::make_pair(std::string(route_name),
                                                          stop_info_by_stop_name.at(route_struct->route_stops.back())->stop_coordinates));
            } else if (!route_struct->is_roundtrip && route_struct->route_stops.front() == route_struct->route_stops.back()) {
                route_for_render.push_back(std::make_pair(std::string(route_name),
                                                          stop_info_by_stop_name.at(route_struct->route_stops.back())->stop_coordinates));
            }
            all_route_names_for_render.push_back(route_for_render);
        }
    }

    size_t color_index = 0;
    for (const std::vector<std::pair<std::string, geo::Coordinates>>& vec_name_and_coord : all_route_names_for_render) {
        for (const std::pair<std::string, geo::Coordinates>& name_and_coord : vec_name_and_coord) {
            svg::Text text;
            svg::Text underlayer;
            underlayer.SetFillColor(settings_.underlayer_color)
                      .SetStrokeColor(settings_.underlayer_color)
                      .SetStrokeWidth(settings_.underlayer_width)
                      .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                      .SetPosition(projector(name_and_coord.second))
                      .SetOffset(settings_.bus_label_offset)
                      .SetFontSize(settings_.bus_label_font_size)
                      .SetFontFamily("Verdana")
                      .SetFontWeight("bold")
                      .SetData(name_and_coord.first);
            text.SetFillColor(settings_.color_palette[color_index])
                .SetPosition(projector(name_and_coord.second))
                .SetOffset(settings_.bus_label_offset)
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetData(name_and_coord.first);
            doc.Add(underlayer);
            doc.Add(text);
        }
        ++color_index;
        if (color_index == settings_.color_palette.size()) {
            color_index = 0;
        }
    }
    
    //-------------------------------------------------------ОТРИСОВКА КРУГОВ ОБОЗНАЧАЮЩИХ ОСТАНОВКИ-----------------------------------------------------------
    
    std::set<std::pair<std::string, geo::Coordinates>, StringFirstComparator> sorted_stops_for_render;
    for (const auto& [route_name, route_struct] : sorted_by_name_routes_info) {
        for (const std::string& stop : route_struct->route_stops) {
            sorted_stops_for_render.emplace(stop, stop_info_by_stop_name.at(stop)->stop_coordinates);
        }
    }
    for (std::pair<std::string, geo::Coordinates> stop_and_coord : sorted_stops_for_render) {
        svg::Circle circle;
        circle.SetCenter(projector(stop_and_coord.second))
              .SetRadius(settings_.stop_radius)
              .SetFillColor("white");
        doc.Add(circle);
    }
    
    
    //-------------------------------------------------------------ОТРИСОВКА НАЗВАНИЯ ОСТАНОВОК----------------------------------------------------------------
    
    std::set<std::pair<std::string, geo::Coordinates>, StringFirstComparator> sorted_stop_names_for_render;
    for (const auto& [route_name, route_struct] : sorted_by_name_routes_info) {
        for (const std::string& stop: route_struct->route_stops) {
            sorted_stop_names_for_render.emplace(stop, stop_info_by_stop_name.at(stop)->stop_coordinates);
        }
    }
    for (std::pair<std::string, geo::Coordinates> stop_and_coord : sorted_stop_names_for_render) {
        svg::Text underlayer;
        underlayer.SetFillColor(settings_.underlayer_color)
                  .SetStrokeColor(settings_.underlayer_color)
                  .SetStrokeWidth(settings_.underlayer_width)
                  .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                  .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                  .SetPosition(projector(stop_and_coord.second))
                  .SetOffset(settings_.stop_label_offset)
                  .SetFontSize(settings_.stop_label_font_size)
                  .SetFontFamily("Verdana")
                  .SetData(stop_and_coord.first);
        svg::Text text;
        text.SetFillColor("black")
            .SetPosition(projector(stop_and_coord.second))
            .SetOffset(settings_.stop_label_offset)
            .SetFontSize(settings_.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetData(stop_and_coord.first);
        doc.Add(underlayer);
        doc.Add(text);
    }
    
    return doc;
}
    
    
VisualSettings MapRenderer::GetSettings() {
    return settings_;
}
