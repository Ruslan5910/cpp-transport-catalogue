#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;
using namespace transport;

int main() {
    TransportCatalogue catalogue;
    {    
        InputReader reader(cin);
        reader.ApplyCommands(catalogue);
    }
    ParseAndPrintStat(catalogue, cout, cin);
}
