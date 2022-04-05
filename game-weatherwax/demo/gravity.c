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

#define WINDOW_TITLE "CS 3"
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 500

const double OUT_RAD_MAX = 100.0;
const double OUT_RAD_MIN = 60.0;
const double IN_RAD_MAX = 50.0;
const double IN_RAD_MIN = 25.0;
const double ROT_SPEED_MIN = 1;
const double ROT_SPEED_MAX = 5;
const double INITIAL_X_MIN = 200.0;
const double INITIAL_X_MAX = 250.0;
const double INITIAL_Y = 0.0;
const vector_t VELO = {250, 250};
const vector_t MIN = {0, 0};
const vector_t MAX = {WINDOW_WIDTH, WINDOW_HEIGHT};
const double ACC = 500.0;
const double INTERVAL = 1;
const double MIN_ELAST = 0.85;
const double MAX_ELAST = 0.95;


void hit_wall(body_t *star) {

    bool coll_y = false;

    list_t *star_shape = body_get_shape(star);

    for (int i = 0; i < list_size(star_shape); i++) {
        vector_t *vec = (vector_t *)list_get(star_shape, i);
        double y = vec->y;
        if (y <= MIN.y) {
            if (!coll_y) {
                body_set_centroid(star, vec_add(body_get_centroid(star), (vector_t) {0.0, -(y - MIN.y)}));
                vector_t currvel = body_get_velocity(star);
                currvel.y = (double) body_get_elasticity(star) * -currvel.y;
                body_set_velocity(star, currvel);
                coll_y = true;
            }
        }
    }

    list_free(star_shape);
}

bool exited_right_wall(body_t *star) {
    list_t *star_shape = body_get_shape(star);

    for (int i = 0; i < list_size(star_shape); i++) {
        vector_t *vec = (vector_t *) list_get(star_shape, i);
        double x = vec->x;
        if (x <= MAX.x) {
            list_free(star_shape);
            return false;
        }
    }
    list_free(star_shape);
    return true;
}

void update_all_stars(scene_t *scene, double dt) {
    scene_tick(scene, dt);

    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *star = scene_get_body(scene, i);
        body_set_velocity(star, vec_add(body_get_velocity(star), (vector_t) {0.0, -ACC * dt}));
        if (exited_right_wall(star)) {
            scene_remove_body(scene, i);
        }
        else {
            hit_wall(star);
        }
    }
}

size_t make_random_star(scene_t *scene, size_t count) {
    double radius = rand_range(OUT_RAD_MIN, OUT_RAD_MAX);
    double radius_inside = rand_range(IN_RAD_MIN, IN_RAD_MAX);

    vector_t center = (vector_t) {radius, WINDOW_HEIGHT - radius};

    double rot_speed = rand_range(-ROT_SPEED_MAX, ROT_SPEED_MAX);

    vector_t velocity = {0.0, 0.0};
    velocity.x = rand_range(INITIAL_X_MIN, INITIAL_X_MAX);
    velocity.y = INITIAL_Y;

    double elasticity = rand_range(MIN_ELAST, MAX_ELAST);

    rgb_color_t rgb = {0.0, 0.0, 0.0};
    rgb.r = rand_range(0.0, 1.0);
    rgb.g = rand_range(0.0, 1.0);
    rgb.b = rand_range(0.0, 1.0);

    double mass = 1.0;

    list_t *star_shape = make_shape_star(center, count + 2, radius, radius_inside);
    body_t *star = body_init(star_shape, mass, rgb);
    body_set_elasticity(star, elasticity);
    body_set_velocity(star, velocity);
    body_set_rot_speed(star, rot_speed);

    scene_add_body(scene, star);
    count++;
    return count;
}

int main(){
    sdl_init(MIN, MAX);

    scene_t *scene = scene_init();
    
    double run_time = 0;
    size_t count = 0;
    
    while (!sdl_is_done(scene)){
        double dt = time_since_last_tick();
        run_time += dt;

        if (run_time > count * INTERVAL) {
            count = make_random_star(scene, count);
        }
        
        update_all_stars(scene, dt);

        sdl_render_scene(scene);
    }

    scene_free(scene);
    return 0;
}