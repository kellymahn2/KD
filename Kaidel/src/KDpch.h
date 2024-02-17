#pragma once

#include "Kaidel/Core/PlatformDetection.h"
#include "Kaidel/Core/Timer.h"
#include "Kaidel/Core/JobSystem.h"

#ifdef KD_PLATFORM_WINDOWS
	#ifndef NOMINMAX
		// See github.com/skypjack/entt/wiki/Frequently-Asked-Questions#warning-c4003-the-min-the-max-and-the-macro
		#define NOMINMAX
	#endif
#endif

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <string_view>
#include <fstream>
#include <sstream>
#include <array>
#include <vector>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>

#include "Kaidel/Core/Base.h"

#include "Kaidel/Core/Log.h"


#ifdef KD_PLATFORM_WINDOWS
	#include <Windows.h>
#endif
