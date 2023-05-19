#pragma once
#include <format>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <cassert>
#include <memory>
using std::string;
using std::vector;
using std::shared_ptr;
using std::make_shared;
#define INFO(x,...) std::cout << std::format("{}", std::format(x, __VA_ARGS__)) << std::endl;


