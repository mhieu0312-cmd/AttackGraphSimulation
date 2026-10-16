#pragma once

#include "Graph.h"

#include <istream>
#include <string>
using namespace std;

Graph loadDataset(istream& input);
Graph loadDataset(const string& filename);
