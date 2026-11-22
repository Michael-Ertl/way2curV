#pragma once

#include "curves/curve.hpp"
#include <string>
#include <vector>

void read_config(std::vector<Curve> &curves, const std::string &path);
int create_config();
int configuration(std::vector<Curve> &curve_configs);
