#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <functional>
#include <emscripten\emscripten.h>
#include <emscripten\bind.h>
#include <emscripten\val.h>
#include <math.h>
#define byte unsigned char
using namespace emscripten;
using namespace std;
#include "MemoryStream.h"
struct Response {
	std::function<void(val)> result;
	std::function<void(val)> error;
};