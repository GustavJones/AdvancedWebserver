#pragma once

#if defined(_MSC_VER)
#undef ADVANCEDWEBSERVER_CONFIGURATION_API

#ifdef BUILDING_ADVANCEDWEBSERVER_CONFIGURATION
#define ADVANCEDWEBSERVER_CONFIGURATION_API __declspec(dllexport)
#else
#define ADVANCEDWEBSERVER_CONFIGURATION_API __declspec(dllimport)
#endif

#elif defined(__GNUC__)
#undef ADVANCEDWEBSERVER_CONFIGURATION_API

#ifdef BUILDING_ADVANCEDWEBSERVER_CONFIGURATION
#define ADVANCEDWEBSERVER_CONFIGURATION_API __attribute__((visibility("default")))
#else
#define ADVANCEDWEBSERVER_CONFIGURATION_API
#endif

#endif