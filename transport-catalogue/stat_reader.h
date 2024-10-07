#pragma once

#include <iosfwd>
#include <string_view>
#include <iostream>

#include "transport_catalogue.h"

namespace transport {

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::ostream& output, std::istream& input);
    
} // namespace transport
