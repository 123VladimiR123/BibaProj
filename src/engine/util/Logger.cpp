//
// Created by Владимир on 09.08.2025.
//

#include <iostream>
#include <streambuf>
#include "Logger.h"

NullStream Logger::nullStream{};

#ifdef ENGINE_DEBUG_MODE_LOGGING
std::ostream *Logger::stream{&std::cout};
#else
std::ostream *Logger::stream{&nullStream};
#endif