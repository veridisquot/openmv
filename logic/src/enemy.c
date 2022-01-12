#include <math.h>

#include "consts.h"
#include "enemy.h"
#include "logic_store.h"
#include "physics.h"
#include "player.h"
#include "sprites.h"

static void on_bat_create(struct world* world, entity e, void* component) {
	struct bat* bat = component;

	bat->old_position = get_component(world, e, struct transform)->position;
	bat->offset = get_component(world, e, struct transform)->position.y;
}

static void on_path_follow_create(struct world* world, entity e, void* component) {
	((struct path_follow*)component)->first_frame = true;
}

entity new_bat(struct world* world, struct room* room, v2f position, char* path_name) {
	entity e = new_entity(world);

	struct animated_sprite sprite = get_animated_sprite(animsprid_bat);

	set_component_create_func(world, struct bat, on_bat_create);
	set_component_create_func(world, struct path_follow, on_path_follow_create);

	add_componentv(world, e, struct transform,
		.position = position,
		.dimentions = { sprite.frames[0].w * sprite_scale, sprite.frames[0].h * sprite_scale });
	add_component(world, e, struct animated_sprite, sprite);
	add_componentv(world, e, struct room_child, .parent = room);
	add_componentv(world, e, struct bat, .path_name = path_name);
	add_componentv(world, e, struct enemy,
		.hp = 1, .damage = 1, .money_drop = 1);
	add_componentv(world, e, struct collider, .rect = {
		0, 0,
		sprite.frames[0].w * sprite_scale,
		sprite.frames[0].h * sprite_scale });

	if (path_name) {
		add_componentv(world, e, struct path_follow, .path_name = path_name, .room = room, .speed = 100.0f);
	}

	return e;
}

entity new_spider(struct world* world, struct room* room, v2f position) {
	entity e = new_entity(world);

	struct sprite sprite = get_sprite(sprid_spider);

	add_componentv(world, e, struct transform,
		.position = { position.x - sprite.rect.w * sprite_scale, position.y - sprite.rect.h * sprite_scale },
		.dimentions = { sprite.rect.w * sprite_scale, sprite.rect.h * sprite_scale });
	add_component(world, e, struct sprite, sprite);
	add_componentv(world, e, struct room_child, .parent = room);
	add_componentv(world, e, struct enemy,
		.hp = 5, .damage = 1, .money_drop = 1);
	add_componentv(world, e, struct spider, .room = room);
	add_componentv(world, e, struct collider, .rect = {
		0, 0,
		sprite.rect.w * sprite_scale,
		sprite.rect.h * sprite_scale });

	return e;
}

entity new_drill(struct world* world, struct room* room, v2f position) {
	entity e = new_entity(world);

	struct animated_sprite sprite = get_animated_sprite(animsprid_drill_left);

	add_componentv(world, e, struct transform,
		.position = { position.x - sprite.frames[0].w * sprite_scale, position.y - sprite.frames[0].h * sprite_scale },
		.dimentions = { sprite.frames[0].w * sprite_scale, sprite.frames[0].h * sprite_scale });
	add_component(world, e, struct animated_sprite, sprite);
	add_componentv(world, e, struct room_child, .parent = room);
	add_componentv(world, e, struct enemy,
		.hp = 10, .damage = 1, .money_drop = 1);
	add_componentv(world, e, struct collider, .rect = {
		0, 0,
		sprite.frames[0].w * sprite_scale,
		sprite.frames[0].h * sprite_scale });
	add_componentv(world, e, struct drill, .room = room);

	return e;
}

void enemy_system(struct world* world, struct room* room, double ts) {
	/* Bat system */
	for (view(world, view, type_info(struct transform), type_info(struct bat))) {
		struct transform* transform = view_get(&view, struct transform);
		struct bat* bat = view_get(&view, struct bat);

		if (!has_component(world, view.e, struct path_follow)) {
			bat->offset += ts;

			transform->position.y = bat->old_position.y + (float)sin(bat->offset) * 100.0f;
		}
	}

	for (view(world, view, type_info(struct transform), type_info(struct path_follow))) {
		struct transform* transform = view_get(&view, struct transform);
		struct path_follow* follow = view_get(&view, struct path_follow);

		struct path* path = get_path(follow->room, follow->path_name);

		if (follow->first_frame) {
			transform->position = path->points[0];
			follow->first_frame = false;
		}

		v2f target = path->points[follow->node];

		v2f dir = v2f_normalised(v2f_sub(target, transform->position));
		transform->position = v2f_add(transform->position, v2f_mul(dir, make_v2f(follow->speed * ts, follow->speed * ts)));

		float dist_sqrd = powf(target.x - transform->position.x, 2.0f) + powf(target.y - transform->position.y, 2.0f);
		if (dist_sqrd < 10.0f) {
			follow->node += follow->reverse ? -1 : 1;

			if (!follow->reverse && follow->node >= path->count) {
				follow->reverse = true;
				follow->node = path->count - 1;
			} else if (follow->reverse && follow->node < 0) {
				follow->reverse = false;
				follow->node = 0;
			}
		}
	}

	/* Spider system */
	for (view(world, view, type_info(struct transform), type_info(struct spider), type_info(struct enemy), type_info(struct collider))) {
		struct transform* transform = view_get(&view, struct transform);
		struct spider* spider = view_get(&view, struct spider);
		struct enemy* enemy = view_get(&view, struct enemy);
		struct collider* collider = view_get(&view, struct collider);

		struct transform* p_transform = get_component(world, logic_store->player, struct transform);

		if (!spider->triggered) {
			float dist_sqrd = powf(p_transform->position.x - transform->position.x, 2.0f) + powf(p_transform->position.y - transform->position.y, 2.0f);

			if (dist_sqrd < 100000) {
				spider->triggered = true;
			}
		} else {
			spider->velocity.y += g_gravity * ts; 

			transform->position = v2f_add(transform->position, v2f_mul(spider->velocity, make_v2f(ts, ts)));

			struct rect e_rect = {
				transform->position.x + collider->rect.x,
				transform->position.y + collider->rect.y,
				collider->rect.w, collider->rect.h
			};

			v2i normal;
			if (rect_room_overlap(spider->room, e_rect, &normal) && normal.y == 1) {
				spider->velocity.y = -800;

				if (p_transform->position.x < transform->position.x) {
					spider->velocity.x = -100;
				} else {
					spider->velocity.x = 100;
				}
			} else {
				if (handle_body_collisions(spider->room, collider->rect, &transform->position, &spider->velocity)) {
					spider->velocity.x = 0.0;
				}
			}
		}
	}

	/* Drill system */
	for (view(world, view, type_info(struct transform), type_info(struct drill), type_info(struct enemy),
		type_info(struct animated_sprite), type_info(struct collider))) {
		struct transform* transform = view_get(&view, struct transform);
		struct drill* drill = view_get(&view, struct drill);
		struct enemy* enemy = view_get(&view, struct enemy);
		struct animated_sprite* sprite = view_get(&view, struct animated_sprite);
		struct collider* collider = view_get(&view, struct collider);

		struct transform* p_transform = get_component(world, logic_store->player, struct transform);

		if (!drill->triggered) {
			float dist_sqrd = powf(p_transform->position.x - transform->position.x, 2.0f) + powf(p_transform->position.y - transform->position.y, 2.0f);

			if (dist_sqrd < 100000) {
				drill->triggered = true;
			}
		} else {
			drill->velocity.y += g_gravity * ts;

			if (p_transform->position.x < transform->position.x) {
				drill->velocity.x -= 100 * ts;
			} else {
				drill->velocity.x += 100 * ts;
			}

			v2f to_player = v2f_sub(transform->position, p_transform->position);

			if (to_player.x > 0.0f && sprite->id != animsprid_drill_left) {
				*sprite = get_animated_sprite(animsprid_drill_left);
			} else if (to_player.x < 0.0f && sprite->id != animsprid_drill_right) {
				*sprite = get_animated_sprite(animsprid_drill_right);
			}

			transform->position = v2f_add(transform->position, v2f_mul(drill->velocity, make_v2f(ts, ts)));

			struct rect e_rect = {
				transform->position.x + collider->rect.x,
				transform->position.y + collider->rect.y,
				collider->rect.w, collider->rect.h
			};

			handle_body_collisions(drill->room, collider->rect, &transform->position, &drill->velocity);
		}
	}

	for (view(world, view, type_info(struct transform), type_info(struct enemy), type_info(struct collider))) {
		struct transform* transform = view_get(&view, struct transform);
		struct enemy* enemy = view_get(&view, struct enemy);
		struct collider* collider = view_get(&view, struct collider);

		struct rect e_rect = {
			transform->position.x + collider->rect.x,
			transform->position.y + collider->rect.y,
			collider->rect.w, collider->rect.h
		};

		for (view(world, p_view, type_info(struct transform), type_info(struct projectile), type_info(struct collider))) {
			struct transform* p_transform = view_get(&p_view, struct transform);
			struct projectile* projectile = view_get(&p_view, struct projectile);
			struct collider* p_collider = view_get(&p_view, struct collider);

			struct rect p_rect = {
				p_transform->position.x + p_collider->rect.x,
				p_transform->position.y + p_collider->rect.y,
				p_collider->rect.w, p_collider->rect.h
			};

			if (rect_overlap(e_rect, p_rect, null)) {
				new_impact_effect(world, p_transform->position, animsprid_blood);
				i32 dmg = projectile->damage;
				if (dmg > enemy->hp) { dmg = enemy->hp; }
				new_damage_number(world, transform->position, -dmg);
				destroy_entity(world, p_view.e);

				enemy->hp -= projectile->damage;

				struct player* player = get_component(world, logic_store->player, struct player);

				enemy->invul = true;
				enemy->invul_timer = 0.1;

				if (enemy->hp <= 0) {
					/* Chance to get a heart is much higher if the player has low hp. */
					double chance = 5;
					if (player->hp < player->max_hp) {
						chance = 30;
					}

					if (random_chance(chance)) {
						struct rect heart_rect = get_sprite(sprid_upgrade_health_pack).rect;

						new_heart(world, room, transform->position, 1);
					} else {
						struct rect coin_rect = get_sprite(sprid_coin).rect;

						for (i32 i = 0; i < enemy->money_drop; i++) {
							new_coin_pickup(world, room, transform->position);
						}
					}

					new_impact_effect(world, transform->position, animsprid_poof);
					destroy_entity(world, view.e);
				}
			}
		}
	}

	for (view(world, view, type_info(struct enemy), type_info(struct animated_sprite))) {
		struct enemy* enemy = view_get(&view, struct enemy);
		struct animated_sprite* sprite = view_get(&view, struct animated_sprite);

		if (enemy->invul) {
			enemy->invul_timer -= ts;
			if (enemy->invul_timer <= 0.0) {
				enemy->invul = false;
			}
		}

		sprite->inverted = enemy->invul;
	}

	for (view(world, view, type_info(struct enemy), type_info(struct sprite))) {
		struct enemy* enemy = view_get(&view, struct enemy);
		struct sprite* sprite = view_get(&view, struct sprite);

		if (enemy->invul) {
			enemy->invul_timer -= ts;
			if (enemy->invul_timer <= 0.0) {
				enemy->invul = false;
			}
		}

		sprite->inverted = enemy->invul;
	}
}
