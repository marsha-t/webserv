#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
#include <string>
#include <cstring>      // for memset
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <sys/stat.h>
#include <fcntl.h>

// Utils
#include "utils.hpp"

// Sockets
#include <unistd.h>			// for close(), read(), write()
#include <netinet/in.h>		// for sockaddr_in
// #include <sys/socket.h> // for socket functions
#include <arpa/inet.h>  // for htons, inet_pton
#include <poll.h>
#include <sys/wait.h>

// STL
#include <map>
#include <vector>
#include <algorithm>

// Static Const Variables
static const std::size_t DEFAULT_MAX_BODY_SIZE = 1 * 1024 * 1024; // 1MB

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