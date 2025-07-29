#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

// Utils
#include "utils.hpp"

// Sockets
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/wait.h>

// STL
#include <map>
#include <vector>
#include <algorithm>

// Static Const Variables
// static const std::size_t DEFAULT_MAX_BODY_SIZE = 1 * 1024 * 1024; // 1MB
static const std::size_t DEFAULT_MAX_BODY_SIZE = 200000000;

// Colours
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

#endif