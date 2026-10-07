#pragma once

#include "Graph.h"

#include <istream>
#include <string>
using namespace std;

// Doc dataset tu stream
Graph loadDataset(
    istream& input
);


Graph loadDataset(
    const string& filename
);