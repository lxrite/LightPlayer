/*
 *    config.hpp:
 *
 *    Copyright (C) 2017-2025 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_CONFIG_HPP
#define LIGHT_PLAYER_CONFIG_HPP

#define LIGHT_PLAYER_PLATFORM_UKNOWN        0
#define LIGHT_PLAYER_PLATFORM_WIN_DESKTOP   1
#define LIGHT_PLAYER_PLATFORM_WIN_UWP       2

#ifdef _WIN32
	#include <winapifamily.h>
	#if WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
		#define LIGHT_PLAYER_PLATFORM LIGHT_PLAYER_PLATFORM_WIN_DESKTOP
	#elif WINAPI_FAMILY == WINAPI_FAMILY_APP
		#define LIGHT_PLAYER_PLATFORM LIGHT_PLAYER_PLATFORM_WIN_UWP
	#else
		#error "Unsupported platform."
	#endif
#else
	#error "Unsupported platform."
#endif

#if LIGHT_PLAYER_PLATFORM == LIGHT_PLAYER_PLATFORM_WIN_DESKTOP
#if defined(LIGHT_PLAYER_EXPORTS)
#   define LIGHT_PLAYER_API __declspec(dllexport)
#else
#   define LIGHT_PLAYER_API __declspec(dllimport)
#endif
#else
#	define LIGHT_PLAYER_API
#endif

#endif // LIGHT_PLAYER_CONFIG_HPP
