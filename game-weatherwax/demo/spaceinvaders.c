#include <stdio.h>
#include <vector.h>
#include <math.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "polygon.h"
#include "sdl_wrapper.h"
#include "list.h"
#include "body.h"
#include "shape.h"
#include "scene.h"
#include "forces.h" 
#include "collision.h"
#include "rand_utils.h"

const vector_t WINDOW_MIN = {0.0, 0.0};
const vector_t WINDOW = {1000.0, 500.0};
const vector_t PLAYER_START = {500.0, 10.0};
const vector_t ATTACKER_VELOCITY = {50.0, 0.0};
const vector_t ATTACK_SHOT_VELOCITY = {0.0, -100.0};
const vector_t DEFEND_SHOT_VELOCITY = {0.0, 100.0};
const vector_t CHANGE = {0.0, -96.0};
const vector_t MICRO_SHIFT = {2.0, 0};
const double FIRE_PROB = 0.01;

const rgb_color_t PLAYER_COLOR = {0.0, 1.0, 0.0};
const rgb_color_t ATTACKER_COLOR = {0.5, 0.5, 0.5};
const rgb_color_t DEFEND_COLOR = {0.0, 0.8, 0.0};
const rgb_color_t ATTACK_COLOR = {1.0, 0.0, 0.0};

const double ORIENTATION_LEFT = M_PI;
const double ORIENTATION_RIGHT = 0;
const double PLAYER_VELOCITY = 500;
const double ANGLE = 3.5*M_PI/3;
const double ROTATION = 0.25*M_PI/3;
const double ATTACKER_RADIUS = 30.0;
const double PLAYER_X_RAD = 50.0; 
const double PLAYER_Y_RAD = 10.0;
const double SHOT_X_RAD = 3.0; 
const double SHOT_Y_RAD = 5.0;
const double MASS = 50.0;
const double SMALL_MASS = 1.0;
const double X_CHANGE = 100; 
const double Y_CHANGE = 32;
const double TIME_CUTOFF = 0.5; 

const size_t N = 100;
const size_t shot_N = 20;
const size_t NUM_ATTACKERS = 8; // number of attackers per line at start 
const size_t NUM_ROWS = 3; //number of rows of attackers

const size_t DEATH_STAR = 13;
const double STAR_SHIFT = 100.0;
const double STAR_MIN = 5.0;
const double STAR_MAX = 30.0;
const double STAR_VEL = 1000.0;
const double EXIT_SPEED = 60.0;

typedef enum {
    PLAYER,
    ATTACKER,
    SHOT1,
    STAR
} body_type1_t;

body_t *make_player(scene_t *scene) {
    list_t *player_points = make_shape_ellipse(2*M_PI, PLAYER_X_RAD, PLAYER_Y_RAD, PLAYER_START, N);
    body_t *player = body_init_with_info(player_points, MASS, PLAYER_COLOR, PLAYER);
    scene_add_body(scene, player);
    return player;
}

body_t *make_one_attacker(scene_t *scene, vector_t start){
    list_t *attacker_points = make_shape_part_circle(ANGLE, ATTACKER_RADIUS, start, N);
    body_t *attacker = body_init_with_info(attacker_points, MASS, ATTACKER_COLOR, ATTACKER);
    body_set_rotation(attacker, ROTATION);
    body_set_centroid(attacker, start);
    body_set_velocity(attacker, ATTACKER_VELOCITY);
    scene_add_body(scene, attacker); 
    create_destructive_collision(scene, attacker, scene_get_body(scene, 0));
    return attacker;
}

// makes an array of attackers at the beginning of the scene 
size_t make_attackers(scene_t *scene, size_t count){
    double y_max = WINDOW.y - Y_CHANGE;
    for (size_t x = 1; x < NUM_ATTACKERS + 1; x++){
        for (size_t y = 0; y < NUM_ROWS; y++){
            vector_t start = (vector_t) {(double) X_CHANGE*x, (double) (y_max - y*Y_CHANGE)}; 
            make_one_attacker(scene, start);
        }
    }
    return (NUM_ROWS * NUM_ATTACKERS);
}

// checks if bodies hit walls
void wall(scene_t *scene) {
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        body_type1_t type = body_get_type(body);
        vector_t centroid = body_get_centroid(body);
        if (type == PLAYER) {
            if (centroid.x + PLAYER_X_RAD > WINDOW.x) {
                body_set_velocity(body, VEC_ZERO);
                body_set_centroid(body, (vector_t) {WINDOW.x - PLAYER_X_RAD, PLAYER_START.y});
            }
            if (centroid.x - PLAYER_X_RAD < WINDOW_MIN.x) {
                body_set_velocity(body, VEC_ZERO);
                body_set_centroid(body, (vector_t) {WINDOW_MIN.x + PLAYER_X_RAD, PLAYER_START.y});
            }
            if (centroid.y - PLAYER_Y_RAD > WINDOW.y) {
                body_set_velocity(body, VEC_ZERO);
            }
        }
        else if (type == ATTACKER) {
            vector_t velo = body_get_velocity(body);
            if (centroid.x > WINDOW.x) {
                body_set_velocity(body, vec_negate(velo));
                body_set_centroid(body, vec_subtract(vec_add(centroid, CHANGE), MICRO_SHIFT));
            }
            if (centroid.x < WINDOW_MIN.x) {
                body_set_velocity(body, vec_negate(velo));
                body_set_centroid(body, vec_add(vec_add(centroid, CHANGE), MICRO_SHIFT));
            }
            if (centroid.y + ATTACKER_RADIUS < WINDOW_MIN.y) {
                body_remove(body);
            }
        }
        else if (type == SHOT1) {
            if (centroid.y - SHOT_Y_RAD > WINDOW.y) {
                body_remove(body);
            }
            if (centroid.y + SHOT_Y_RAD < WINDOW_MIN.y) {
                body_remove(body);
            }
        }
        else if (type == STAR) {
            if (centroid.y + STAR_MAX < WINDOW_MIN.y) {
                body_remove(body);
            }
        }
    }
}



// // makes a bullet
// body_t *make_shot(scene_t *scene, vector_t start, rgb_color_t color, vector_t velocity) {
//     list_t *shot_points = make_shape_ellipse(2*M_PI, SHOT_X_RAD, SHOT_Y_RAD, start, shot_N);
//     body_t *shot = body_init_with_info(shot_points, SMALL_MASS, color, SHOT);
//     body_set_centroid(shot, start);
//     body_set_velocity(shot, velocity);
//     scene_add_body(scene, shot);
//     return shot;
// }

void make_galaxy_star(scene_t *scene) {
    double radius = rand_range(STAR_MIN, STAR_MAX);
    list_t *galaxy_star_points = make_shape_star((vector_t) {rand_range(WINDOW_MIN.x, WINDOW.x), WINDOW.y + STAR_SHIFT}, 4, radius, radius/4);
    body_t *galaxy_star = body_init_with_info(galaxy_star_points, MASS, ATTACKER_COLOR, STAR);
    body_set_velocity(galaxy_star, (vector_t) {0.0, -STAR_VEL});
    scene_add_body(scene, galaxy_star);
}

// makes a random attacker shoot a bullet
void attack(scene_t *scene) {
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        if (body_get_type(body) == ATTACKER) {
            if ((double) rand()/(double) RAND_MAX < FIRE_PROB) {
                vector_t location = body_get_centroid(body);
                body_t *shot = make_shot(scene, location, SHOT_Y_RAD, SMALL_MASS, ATTACK_COLOR, ATTACK_SHOT_VELOCITY);
                create_destructive_collision(scene, scene_get_body(scene, 0), shot);
            }
        }
    }
}

// shoots a bullet from player 
void defend(scene_t *scene, size_t attacker_count) {
    vector_t location = body_get_centroid(scene_get_body(scene, 0));
    body_t *shot = make_shot(scene, location, SHOT_Y_RAD, SMALL_MASS, DEFEND_COLOR, DEFEND_SHOT_VELOCITY);
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        if (body_get_type(body) == ATTACKER) {
            create_destructive_collision(scene, body, shot);
        }
    }
}

void on_key(scene_t *scene, char key, key_event_type_t type, double held_time) {
    body_t *player = scene_get_body(scene, 0);
    vector_t vel = body_get_velocity(player);
    if (type == KEY_PRESSED) {
        switch (key) {
            case LEFT_ARROW:
                if (body_get_centroid(player).x - PLAYER_X_RAD > WINDOW_MIN.x) {
                    body_set_velocity(player, (vector_t) {-PLAYER_VELOCITY, vel.y});
                }
                break;

            case RIGHT_ARROW:
                if (body_get_centroid(player).x + PLAYER_X_RAD < WINDOW.x) {
                    body_set_velocity(player, (vector_t) {PLAYER_VELOCITY, vel.y});
                }
                break;

            case SDLK_SPACE:
                defend(scene, NUM_ATTACKERS * NUM_ROWS);
                break;

        }
    }
    else if (type == KEY_RELEASED) {
        switch (key) {
            case LEFT_ARROW:
                if (body_get_velocity(player).x == -PLAYER_VELOCITY) {
                    body_set_velocity(player, (vector_t) {0.0, vel.y});
                    break;
                }

            case RIGHT_ARROW:
                if (body_get_velocity(player).x == PLAYER_VELOCITY) {
                    body_set_velocity(player, (vector_t) {0.0, vel.y});
                    break;
                }     
        }
    }
}

int main() {
    sdl_init(WINDOW_MIN, WINDOW);

    scene_t *scene = scene_init();
    double time = 0;

    body_t *player = make_player(scene);
    vector_t player_location = PLAYER_START;
    size_t attacker_count = make_attackers(scene, NUM_ATTACKERS); 

    sdl_on_key(on_key);

    while (!sdl_is_done(scene)){
        double dt = time_since_last_tick();
        if (body_get_type(scene_get_body(scene, 0)) == PLAYER) {
            wall(scene);
            time += dt;
            if (scene_bodies(scene) > 1 && body_get_type(scene_get_body(scene, 1)) == ATTACKER) {                    
                if (time > TIME_CUTOFF) {
                    attack(scene); 
                    time = 0;
                }

                player_location = body_get_centroid(player);
            }

            scene_tick(scene, dt);
            if (scene_bodies(scene) == 1 || body_get_type(scene_get_body(scene, 1)) != ATTACKER) {
                // Win state
                if (scene_bodies(scene) > 1 && body_get_type(scene_get_body(scene, 1)) == SHOT1) {
                    for (size_t i = 1; i < scene_bodies(scene); i++) {
                        body_remove(scene_get_body(scene, i));
                    }
                }
                if (time > TIME_CUTOFF) {
                    make_galaxy_star(scene);
                    time = 0;
                }
                vector_t player_vel = body_get_velocity(player);
                body_set_velocity(player, (vector_t) {player_vel.x, EXIT_SPEED});
                if (body_get_centroid(player).y - PLAYER_Y_RAD > WINDOW.y) {
                    break;
                }
            }
        }
        else {
            list_t *explosion_star = make_shape_star(player_location, DEATH_STAR, PLAYER_X_RAD, (PLAYER_X_RAD + PLAYER_Y_RAD)/8);
            body_t *explosion = body_init(explosion_star, MASS, PLAYER_COLOR);
            scene_add_body(scene, explosion);
        }
        sdl_render_scene(scene);
    }

    scene_free(scene);
    return 0;
}