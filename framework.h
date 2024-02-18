#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>


inline std::ofstream GameLog("Alpharium.log");
#define DEBUG
#ifdef DEBUG
#define LOG(msg) (GameLog << msg << std::endl)
#else
#define LOG(msg) (std::cout << msg << std::endl)
#endif
