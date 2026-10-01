#pragma once
#include "Graph.h"
#include <istream>
#include <string>

// Build graph tam; neu loi thi khong de lai graph dang load do.
Graph loadDataset(std::istream& input);
Graph loadDataset(const std::string& filename);
