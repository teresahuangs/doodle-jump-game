#include <stdio.h>
#include <vector.h>
#include <math.h>
#include <stdlib.h>
#include "polygon.h"
#include "sdl_wrapper.h"
#include "list.h"
#include "body.h"
#include "shape.h"
#include "scene.h"
#include "rand_utils.h"

vector_t WINDOW_MIN = {0.0, 0.0};
vector_t WINDOW = {1000.0, 500.0};
double PACMAN_SIZE = 50.0; 
double PACMAN_ANGLE = M_PI / 6;
vector_t PAC_START = {500.0, 250.0};
double MASS = 50.0;
rgb_color_t PAC_COLOR = {0.2, 0.2, 0.6};
double PAC_ELASTICITY = 0.8;
vector_t INITIAL_VELOCITY = {0.0, 0.0};
double MINIMUM_SPEED = 50.0;
double ORIENTATION_UP = M_PI/2;
double ORIENTATION_DOWN = M_PI * 3/2;
double ORIENTATION_LEFT = M_PI;
double ORIENTATION_RIGHT = 0;
size_t N = 100;
double CIRC_RAD = 5.0;
double CIRC_MASS = 0.5;
rgb_color_t CIRC_COLOR = {0.2, 0.9, 0.6};
double CIRC_ELASTICITY = 0.7;
int CIRC_NUM = 30;
const double INTERVAL = 0.1;
double ACC = 100;

body_t *make_pacman(scene_t *scene){
    list_t *pac_points = make_shape_pacman(PACMAN_ANGLE, PACMAN_SIZE, PAC_START, N);
    body_t *pacman = body_init(pac_points, MASS, PAC_COLOR);
    body_set_elasticity(pacman, PAC_ELASTICITY);
    body_set_centroid(pacman, PAC_START);
    body_set_velocity(pacman, INITIAL_VELOCITY);
    scene_add_body(scene, pacman);
    return pacman;
}

void collisions(scene_t *scene){
    body_t *curr = scene_get_body(scene, 0);
    vector_t cent = body_get_centroid(curr);
    if (cent.x < WINDOW_MIN.x) {
        cent.x = WINDOW.x;
    }
    if (cent.x > WINDOW.x) {
        cent.x = WINDOW_MIN.x;
    }
    if (cent.y < WINDOW_MIN.y) {
        cent.y = WINDOW.y;
    }
    if (cent.y > WINDOW.y) {
        cent.y = WINDOW_MIN.y;
    }
    body_set_centroid(curr, cent);
}

void make_ball(scene_t *scene, int circ_count) {
    int count = 0;
    while (count != circ_count) {
        vector_t center = {rand_range(0.0, WINDOW.x), rand_range(0.0, WINDOW.y)};
        list_t *circle_points = make_shape_circle(CIRC_RAD, center, N);
        body_t *circle = body_init(circle_points, CIRC_MASS, CIRC_COLOR);
        body_set_elasticity(circle, CIRC_ELASTICITY);
        scene_add_body(scene, circle);
        count++;
    }
}

void pacman_eat_balls(scene_t *scene){
    body_t *pacman = scene_get_body(scene, 0);
    for (size_t i = 1; i < scene_bodies(scene); i++){
        list_t *man = body_get_shape(pacman);
        list_t *ball = body_get_shape(scene_get_body(scene, i));
        if(polygon_overlap(man, ball)){
            scene_remove_body(scene, i);
        }
        list_free(man);
        list_free(ball);
    }
}

void update_velocity(scene_t *scene, double held_time) {
    body_t *pacman = scene_get_body(scene, 0);
    vector_t vel = {0.0, 0.0};
    double orientation = body_get_rotation(pacman);

    if (orientation == ORIENTATION_UP) {
        vel.y = MINIMUM_SPEED + ACC * held_time;
    }
    else if (orientation == ORIENTATION_DOWN) {
        vel.y = -MINIMUM_SPEED - ACC * held_time;
    }
    else if (orientation == ORIENTATION_LEFT) {
        vel.x = -MINIMUM_SPEED - ACC * held_time;
    }
    else if (orientation == ORIENTATION_RIGHT) {
        vel.x = MINIMUM_SPEED + ACC * held_time;
    }

    body_set_velocity(pacman, vel);
}

double flip(double x) {
    if (x > 0) {
        x = -x;
    }
    return x;
}

void on_key(scene_t *scene, char key, key_event_type_t type, double held_time) {
    body_t *pacman = scene_get_body(scene, 0);
    // vector_t vel = body_get_velocity(pacman);
    double orientation = body_get_rotation(pacman);
    if (type == KEY_PRESSED) {
        switch (key) {
            case UP_ARROW:
                if (orientation != ORIENTATION_UP) {
                    body_set_rotation(pacman, ORIENTATION_UP);
                }
                update_velocity(scene, held_time);
                break;

            case DOWN_ARROW:
                if (orientation != ORIENTATION_DOWN) {
                    body_set_rotation(pacman, ORIENTATION_DOWN);
                }
                update_velocity(scene, held_time);
                break;

            case LEFT_ARROW:
                if (orientation != ORIENTATION_LEFT) {
                    body_set_rotation(pacman, ORIENTATION_LEFT);
                }
                update_velocity(scene, held_time);
                break;

            case RIGHT_ARROW:
                if (orientation != ORIENTATION_RIGHT) {
                    body_set_rotation(pacman, ORIENTATION_RIGHT);
                }
                update_velocity(scene, held_time);
                break;
        }
    }
    else if (type == KEY_RELEASED) {
        switch (key) {
            case UP_ARROW:
                if (orientation == ORIENTATION_UP) {
                    body_set_velocity(pacman, (vector_t) {0.0, MINIMUM_SPEED});
                }
                break;

            case DOWN_ARROW:
                if (orientation == ORIENTATION_DOWN) {
                    body_set_velocity(pacman, (vector_t) {0.0, -MINIMUM_SPEED});
                }
                break;

            case LEFT_ARROW:
                if (orientation == ORIENTATION_LEFT) {
                    body_set_velocity(pacman, (vector_t) {-MINIMUM_SPEED, 0.0});
                }
                break;

            case RIGHT_ARROW:
                if (orientation == ORIENTATION_RIGHT) {
                    body_set_velocity(pacman, (vector_t) {MINIMUM_SPEED, 0.0});
                }
                break;
        }
    }
}

int main(){
    sdl_init(WINDOW_MIN, WINDOW);

    scene_t *scene = scene_init();
    double time = 0;

    make_pacman(scene);
    make_ball(scene, CIRC_NUM);
    sdl_on_key(on_key);

    while (!sdl_is_done(scene)){
        double dt = time_since_last_tick();
        collisions(scene);
        scene_tick(scene, dt);
        pacman_eat_balls(scene);
        sdl_render_scene(scene);
        time += dt;
        if (time > 1.5) {
            make_ball(scene, CIRC_NUM / 6);
            time = 0;
        }
    }

    scene_free(scene);
    return 0;
}
