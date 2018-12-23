/*
 *    config.hpp:
 *
 *    Copyright (C) 2017-2018 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_CONFIG_HPP
#define LIGHT_PLAYER_CONFIG_HPP

#if defined(LIGHT_PLAYER_EXPORTS)
#   define LIGHT_PLAYER_API __declspec(dllexport)
#else
#   define LIGHT_PLAYER_API __declspec(dllimport)
#endif

#endif // LIGHT_PLAYER_CONFIG_HPP
