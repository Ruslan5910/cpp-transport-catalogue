#pragma once

#include <iostream>

#include "geo.h"
#pragma once

#include <iostream>

#include "geo.h"
#include "transport_catalogue.h"

namespace transport {
    
namespace parsewords {
    
struct StopsAndMeters {
    std::vector<std::string_view> stops_to;
    std::vector<int> meters;
};   
    
struct ParseDescription {
    std::string_view coordinates;
    std::string_view distances;
};
    
struct CommandDescription {
    // Определяет, задана ли команда (поле command непустое)
    explicit operator bool() const {
        return !command.empty();
    }

    bool operator!() const {
        return !operator bool();
    }

    std::string command;      // Название команды
    std::string id;           // id маршрута или остановки
    std::string description;  // Параметры команды
};

} // namespace parsewords
    
class InputReader {
public:
   explicit InputReader(std::istream& in)
       : in_(in) 
    {
        int base_request_count;
        in_ >> base_request_count >> std::ws;
        for (int i = 0; i < base_request_count; ++i) {
            std::string line;
            std::getline(in_, line);   //Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino
            ParseLine(line);
        }
    }      

    /**
     * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
     */
    void ParseLine(std::string_view line);

    /**
     * Наполняет данными транспортный справочник, используя команды из commands_
     */
    void ApplyCommands(TransportCatalogue& catalogue);

private:
    std::vector<parsewords::CommandDescription> commands_;
    std::istream& in_;
};
    
} // namespace transport
