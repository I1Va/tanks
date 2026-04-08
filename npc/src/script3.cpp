#include <random>
#include "API/server.hpp"

static api::TankId id;
static int action_cooldown = 0;
static int shoot_cooldown = 0;
static int stuck_counter = 0;
static int same_position_counter = 0;
static api::Cord last_position = {0, 0};
static api::Dir last_direction = api::Dir::UP;
static int behavior_state = 0; // 0=patrol, 1=aggressive, 2=evade
static int no_target_counter = 0;

void npc3_init(api::IServer &server, api::Cord tile_cord) {
    id = server.spawn_tank_in_tile(tile_cord);
}

void npc3_step(api::IServer &server) {
    api::TankInfo info;
    if (server.get_tank_info(id, info) != 0) return;
    
    if (action_cooldown > 0) action_cooldown--;
    if (shoot_cooldown > 0) shoot_cooldown--;
    
    if (info.pos.x == last_position.x && info.pos.y == last_position.y) {
        same_position_counter++;
        if (same_position_counter > 10) {
            stuck_counter++;
            same_position_counter = 0;
        }
    } else {
        same_position_counter = 0;
    }
    last_position = info.pos;
    
    if (stuck_counter > 3) {
        behavior_state = 2;
        stuck_counter = 0;
    }
    
    no_target_counter++;
    if (no_target_counter > 50) {
        behavior_state = 0;
        no_target_counter = 0;
    }
    
    if (action_cooldown == 0) {
        int action = -1;
        
        switch (behavior_state) {
            case 0:
                action = rand() % 8;
                if (action < 4) {
                    server.tank_move_torward(id);
                    action_cooldown = 3;
                } else if (action < 6) {
                    server.tank_rotate(id, api::RotationDir::LEFT);
                    action_cooldown = 2;
                } else if (action < 8) {
                    server.tank_rotate(id, api::RotationDir::RIGHT);
                    action_cooldown = 2;
                }
                break;
                
            case 1:
                action = rand() % 10;
                if (action < 6) {
                    server.tank_move_torward(id);
                    action_cooldown = 2;
                } else if (action < 8) {
                    server.tank_rotate(id, (rand() % 2) ? 
                        api::RotationDir::LEFT : api::RotationDir::RIGHT);
                    action_cooldown = 1;
                } else if (action < 10 && shoot_cooldown == 0) {
                    server.turret_fire(id);
                    shoot_cooldown = 15;
                    action_cooldown = 2;
                }
                break;
                
            case 2:
                action = rand() % 6;
                if (action < 2) {
                    server.tank_move_torward(id);
                    action_cooldown = 1;
                } else if (action < 4) {
                    server.tank_rotate(id, (rand() % 2) ? 
                        api::RotationDir::LEFT : api::RotationDir::RIGHT);
                    action_cooldown = 1;
                } else if (action < 6) {
                    server.tank_rotate(id, (rand() % 2) ? 
                        api::RotationDir::LEFT : api::RotationDir::RIGHT);
                    server.tank_rotate(id, (rand() % 2) ? 
                        api::RotationDir::LEFT : api::RotationDir::RIGHT);
                    action_cooldown = 2;
                }
                if (stuck_counter == 0 && same_position_counter < 5) {
                    behavior_state = 0;
                }
                break;
        }
        
        if (rand() % 100 < 5) {
            behavior_state = rand() % 3;
        }
    }
    
    last_direction = info.dir;
}

