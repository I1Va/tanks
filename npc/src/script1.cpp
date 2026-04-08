#include <random>
#include "API/server.hpp"

static api::TankId id;
static int action_cooldown = 0;
static int shoot_cooldown = 0;
static int stuck_counter = 0;
static int same_position_counter = 0;
static api::Cord last_position = {0, 0};
static api::Dir last_direction = api::Dir::UP;
static int behavior_state = 1;
static int no_target_counter = 0;
static int aggressive_charge = 0;

void npc1_init(api::IServer &server, api::Cord tile_cord) {
    id = server.spawn_tank_in_tile(tile_cord);
}

void npc1_step(api::IServer &server) {
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
    if (no_target_counter > 80) {
        behavior_state = 1;
        no_target_counter = 0;
    }
    
    aggressive_charge++;
    if (aggressive_charge > 30) {
        behavior_state = 1;
        aggressive_charge = 0;
    }
    
    if (action_cooldown == 0) {
        int action = -1;
        
        switch (behavior_state) {
            case 0:
                action = rand() % 8;
                if (action < 5) {
                    server.tank_move_torward(id);
                    action_cooldown = 2;
                } else if (action < 7) {
                    server.tank_rotate(id, api::RotationDir::LEFT);
                    action_cooldown = 1;
                } else if (action < 8) {
                    server.tank_rotate(id, api::RotationDir::RIGHT);
                    action_cooldown = 1;
                }
                break;
                
            case 1:
                action = rand() % 10;
                if (action < 7) {
                    server.tank_move_torward(id);
                    action_cooldown = 1;
                } else if (action < 9) {
                    server.tank_rotate(id, (rand() % 2) ? 
                        api::RotationDir::LEFT : api::RotationDir::RIGHT);
                    action_cooldown = 0;
                } else if (action < 10 && shoot_cooldown == 0) {
                    server.turret_fire(id);
                    shoot_cooldown = 6;
                    action_cooldown = 1;
                }
                
                if (shoot_cooldown == 0 && (rand() % 100) < 40) {
                    server.turret_fire(id);
                    shoot_cooldown = 6;
                }
                break;
                
            case 2:
                action = rand() % 6;
                if (action < 3) {
                    server.tank_move_torward(id);
                    action_cooldown = 0;
                } else if (action < 5) {
                    server.tank_rotate(id, (rand() % 2) ? 
                        api::RotationDir::LEFT : api::RotationDir::RIGHT);
                    action_cooldown = 0;
                } else if (action < 6) {
                    server.tank_rotate(id, (rand() % 2) ? 
                        api::RotationDir::LEFT : api::RotationDir::RIGHT);
                    server.tank_rotate(id, (rand() % 2) ? 
                        api::RotationDir::LEFT : api::RotationDir::RIGHT);
                    action_cooldown = 1;
                }
                if (stuck_counter == 0 && same_position_counter < 3) {
                    behavior_state = 1;
                }
                break;
        }
        
        if (rand() % 100 < 20) {
            behavior_state = 1;
        }
    }
    
    last_direction = info.dir;
}