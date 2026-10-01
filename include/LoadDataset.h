#pragma once

#include "Graph.h"

#include <istream>
#include <string>

using namespace std;


// =========================
// LOAD DATASET
// =========================

// Doc dataset tu stream
Graph loadDataset(
    istream& input
);

// Doc dataset tu file JSON
Graph loadDataset(
    const string& filename
);