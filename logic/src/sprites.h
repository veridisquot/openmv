#pragma once

#include "common.h"
#include "coresys.h"
#include "room.h"
#include "video.h"

/* Texture IDs */
enum {
	texid_player = 0,
	texid_upgrades,
	texid_tsblue,
	texid_fx,
	texid_icon,
	texid_arms,
	texid_bad,
	texid_back
};

/* Sprite IDs  */
enum {
	animsprid_player_run_right = 0,
	animsprid_player_run_left,
	animsprid_player_jump_right,
	animsprid_player_jump_left,
	animsprid_player_fall_right,
	animsprid_player_fall_left,
	animsprid_player_idle_right,
	animsprid_player_idle_left,
	animsprid_muzzle_flash,
	animsprid_projectile_impact,
	animsprid_blood,
	animsprid_bat,
	animsprid_coin,
	sprid_upgrade_jetpack,
	sprid_upgrade_health_pack,
	sprid_upgrade_health_booster,
	sprid_fx_jetpack,
	sprid_icon_ptr,
	sprid_projectile,
	sprid_hud_hp,
	sprid_hud_hp_bar,
	sprid_coin,
	sprid_door,
	sprid_door_open,
	sprid_terminal
};

void preload_sprites();
struct sprite get_sprite(u32 id);
struct animated_sprite get_animated_sprite(u32 id);
struct texture* get_texture(u32 id);
