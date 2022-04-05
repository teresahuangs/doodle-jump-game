#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "vector.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "list.h"
#include "body.h"
#include "shape.h"
#include "scene.h"
#include "forces.h" 
#include "collision.h"

// CONSTANTS: 
const vector_t WINDOW_MIN = {0.0, 0.0};
const vector_t WINDOW_MAX = {1000.0, 500.0};
const vector_t PLAYER_START = {500, 15};
const vector_t BALL_VELOCITY = {300.0, 500.0};

const double BALL_MASS = 20.0;
const double ELASTICITY = 0.85;

const double PLAYER_HEIGHT = 15.0; 
const double PLAYER_WIDTH = 45.0; 
const double PLAYER_VELOCITY = 500.0;

const double BRICK_HEIGHT = 10.5; 
const double BRICK_WIDTH = 48.0; 
const double BRICK_X_DIFF = 100.0; 
const double BRICK_X_FIRST = 50.0;
const double BRICK_Y_DIFF = 25.0; 
const double BRICK_Y_FIRST = 12.5;

const double BALL_RADIUS = 10.0; 

const size_t N = 100; 
const size_t NUM_BRICKS_X = 10; 
const size_t NUM_BRICKS_Y = 3;
const size_t NUM_COLORS = 10; 

const rgb_color_t PLAYER_COLOR = {1.0, 0.0, 0.0};
const rgb_color_t RED = {1.0, 0.0, 0.0};
const rgb_color_t ORANGE = {1.0, 0.5, 0.0};
const rgb_color_t YELLOW = {1.0, 1.0, 0.0};
const rgb_color_t LIME = {0.5, 1.0, 0.0};
const rgb_color_t GREEN = {0.0, 1.0, 0.0};
const rgb_color_t CYAN = {0.0, 1.0, 1.0};
const rgb_color_t BLUE = {0.0, 0.0, 1.0};
const rgb_color_t PURPLE = {0.5, 0.0, 1.0};
const rgb_color_t PINK = {1.0, 0.0, 1.0};
const rgb_color_t DARK_PINK = {1.0, 0.0, 0.5};

// METHODS: 
typedef enum {
    HEALTH
} body_type3_t;

list_t *make_shape_rect(double width, double length, vector_t centroid){
    list_t *rect_points = list_init(4, (free_func_t) free);
    for (size_t i = 1; i <= 4; i ++){
        double x = centroid.x;
        double y = centroid.y;
        if (i > 2){
            y = y - width;
        }
        else{
            y = y + width;
        }
        if(i > 1 && i < 4){
            x = x + length;
        }
        else{
            x = x - length;
        }
        vector_t *v = malloc(sizeof(vector_t));
        v->x = x;
        v->y = y;
        list_add(rect_points, v);
    }
    return rect_points;
}

rgb_color_t *color_list(){
    rgb_color_t *colors = malloc(sizeof(rgb_color_t) * NUM_BRICKS_X);
    colors[0] = RED;
    colors[1] = ORANGE;
    colors[2] = YELLOW;
    colors[3] = LIME;
    colors[4] = GREEN;
    colors[5] = CYAN;
    colors[6] = BLUE;
    colors[7] = PURPLE;
    colors[8] = PINK;
    colors[9] = DARK_PINK;
    return colors;
}

body_t *make_one_brick(scene_t *scene, vector_t center, rgb_color_t color, size_t health){
    list_t *brick_points = make_shape_rect(BRICK_HEIGHT, BRICK_WIDTH, center);
    body_t *brick = body_init_with_info(brick_points, INFINITY, color, HEALTH);
    body_set_centroid(brick, center);
    scene_add_body(scene, brick);
    return brick;
}

void brick_collisions_handler(body_t *body1, body_t *body2, vector_t axis, void *aux){
    double mass1 = body_get_mass(body1);
    double mass2 = body_get_mass(body2);
    vector_t vel1 = body_get_velocity(body1);
    vector_t vel2 = body_get_velocity(body2);
    
    double u1 = vec_dot(axis, vel1);
    double u2 = vec_dot(axis, vel2);

    double impulse;
    if (mass1 == INFINITY) {
        impulse = mass2;
    }
    else if (mass2 == INFINITY) {
        impulse = mass1;
    }
    else {
        impulse = mass1 * mass2 / (mass1 + mass2);
    }
    impulse *=  (1 + ELASTICITY) * (u2 - u1);

    body_add_impulse(body1, vec_multiply(impulse, axis));
    body_add_impulse(body2, vec_multiply(-impulse, axis));
    size_t health = (size_t) body_get_type(body1);
    if (health > 1){
        rgb_color_t color = body_get_color(body1); 
        vector_t center = body_get_centroid(body1);
        size_t new_health = health - 1;
        body_remove(body1);
        body_t *brick = make_one_brick((scene_t*) aux, center, color, new_health);
        create_collision((scene_t*) aux, brick, scene_get_body((scene_t*) aux, 0), brick_collisions_handler, (scene_t*) aux, NULL);
    }
    else{
        body_remove(body1);
    }
}

body_t *make_player(scene_t *scene, body_t *ball){
    list_t *player_points = make_shape_rect(PLAYER_HEIGHT, PLAYER_WIDTH, PLAYER_START);
    body_t *player = body_init_with_info(player_points, INFINITY, PLAYER_COLOR, INFINITY);
    body_set_centroid(player, PLAYER_START);
    scene_add_body(scene, player);
    create_physics_collision(scene, ELASTICITY, player, ball); 
    return player;
}

body_t *make_ball(scene_t *scene){
    vector_t centroid = vec_add(PLAYER_START, (vector_t) {0.0, PLAYER_HEIGHT + BALL_RADIUS});
    list_t *ball_points = make_shape_circle(BALL_RADIUS, centroid, N);
    body_t *ball = body_init_with_info(ball_points, BALL_MASS, PLAYER_COLOR, INFINITY);
    body_set_centroid(ball, centroid);
    body_set_velocity(ball, BALL_VELOCITY);
    scene_add_body(scene, ball);
    return ball;
}

void make_all_bricks(scene_t *scene){
    rgb_color_t* colors = color_list();
    for(size_t i = 0; i < NUM_BRICKS_X; i++){
        rgb_color_t column = colors[i];
        for(size_t j = 0; j < NUM_BRICKS_Y; j++){
            vector_t center = (vector_t) {BRICK_X_FIRST + i * BRICK_X_DIFF, WINDOW_MAX.y - BRICK_Y_FIRST - j * BRICK_Y_DIFF};
            body_t *brick = make_one_brick(scene, center, column, NUM_BRICKS_Y - j);
            create_collision(scene, brick, scene_get_body(scene, 0), brick_collisions_handler, scene, NULL);
        }
    }
}

scene_t *reset(scene_t *scene_old){
    scene_free(scene_old); 
    scene_t *scene = scene_init();
    body_t *ball = make_ball(scene);
    make_player(scene, ball); 
    make_all_bricks(scene);
    return scene;
}

scene_t *ball_wall_hit(scene_t *scene){
    body_t *ball = scene_get_body(scene, 0);
    double x = body_get_centroid(ball).x;
    double y = body_get_centroid(ball).y;
    double x_velo = body_get_velocity(ball).x;
    double y_velo = body_get_velocity(ball).y;
    if(x + BALL_RADIUS > WINDOW_MAX.x || x - BALL_RADIUS < WINDOW_MIN.x){
        body_set_velocity(ball, (vector_t) {-x_velo, y_velo});
    }
    if(y + BALL_RADIUS > WINDOW_MAX.y){
        body_set_velocity(ball, (vector_t) {x_velo, -y_velo});
    }
    if(y - BALL_RADIUS < WINDOW_MIN.y){
        scene = reset(scene);
    }
    return scene;
}

void on_key(scene_t *scene, char key, key_event_type_t type, double held_time) {
    body_t *player = scene_get_body(scene, 1);
    vector_t vel = body_get_velocity(player);
    if (type == KEY_PRESSED) {
        switch (key) {
            case LEFT_ARROW:
                if (body_get_centroid(player).x - PLAYER_WIDTH > WINDOW_MIN.x) {
                    body_set_velocity(player, (vector_t) {-PLAYER_VELOCITY, vel.y});
                }
                break;

            case RIGHT_ARROW:
                if (body_get_centroid(player).x + PLAYER_WIDTH < WINDOW_MAX.x) {
                    body_set_velocity(player, (vector_t) {PLAYER_VELOCITY, vel.y});
                }
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

int main(int argc, char *argv[]) {
    sdl_init(WINDOW_MIN, WINDOW_MAX);

    scene_t *scene = scene_init();
    double time = 0;
    scene = reset(scene); 

    sdl_on_key(on_key);

    while (!sdl_is_done(scene)){
        double dt = time_since_last_tick();

        scene = ball_wall_hit(scene);
        body_t *player = scene_get_body(scene, 1); 
        
        scene_tick(scene, dt);
        sdl_render_scene(scene);
    }

    scene_free(scene);
    return 0;
}