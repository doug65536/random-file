#pragma once

#include "config.h"
#include "targetver.h"

#include <memory>
#include <cstdio>
#include <cstdint>
#include <stdexcept>

#include <limits>
#include <vector>
#include <algorithm>

#include <string>
#include <sstream>

#include <fstream>
#include <iostream>

#if defined(HAVE_WINDOWS_H)
#include <windows.h>
#include <wincrypt.h>
#elif defined(HAVE_UNISTD_H)
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#endif
