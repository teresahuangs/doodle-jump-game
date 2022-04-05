#include <stdlib.h>
#include "sdl_wrapper.h"
#include <math.h>
#include <assert.h>
#include "polygon.h"
#include "vector.h"
#include "list.h"
#include <stdbool.h>
#include "shape.h"
#include <assert.h>

const vector_t WINDOW_MIN = {0.0, 0.0};
const vector_t WINDOW_MAX = {1000.0, 500.0};
const int POINT_NUM = 5;
const int BIG_R = 60;
const int SMALL_R = 25;
const double ROT_SPEED = 0.5;
const vector_t VELOCITY = {.x = 100, .y = 100};
const double R = 10.0 /255.0;
const double G = 200.0 / 255.0;
const double B = 40.0/255.0;
const double ELASTICITY = 1.0;
const double MASS = 1.0;

void hit_wall(body_t *star) {
    bool coll_x = false;
    bool coll_y = false;

    list_t *star_shape = body_get_shape(star);

    for (int i = 0; i < list_size(star_shape); i++) {
        vector_t *vec = (vector_t *)list_get(star_shape, i);
        double y = vec->y;
        double x = vec->x;
        if (!coll_x) {
            if (x <= WINDOW_MIN.x) {
                body_set_centroid(star, vec_add(body_get_centroid(star), (vector_t) {WINDOW_MIN.x - x, 0.0}));
                vector_t currvel = body_get_velocity(star);
                currvel.x = (double) body_get_elasticity(star) * -currvel.x;
                body_set_velocity(star, currvel);
                coll_x = true;
            }
            if (x >= WINDOW_MAX.x) {
                body_set_centroid(star, vec_add(body_get_centroid(star), (vector_t) {WINDOW_MAX.x - x, 0.0}));
                vector_t currvel = body_get_velocity(star);
                currvel.x = (double) body_get_elasticity(star) * -currvel.x;
                body_set_velocity(star, currvel);
                coll_x = true;
            }
        }
        if (!coll_y) {
            if (y <= WINDOW_MIN.y) {
                body_set_centroid(star, vec_add(body_get_centroid(star), (vector_t) {0.0, WINDOW_MIN.y - y}));
                vector_t currvel = body_get_velocity(star);
                currvel.y = (double) body_get_elasticity(star) * -currvel.y;
                body_set_velocity(star, currvel);
                coll_y = true;
            }
            if (y >= WINDOW_MAX.y) {
                body_set_centroid(star, vec_add(body_get_centroid(star), (vector_t) {0.0, WINDOW_MAX.y - y}));
                vector_t currvel = body_get_velocity(star);
                currvel.y = (double) body_get_elasticity(star) * -currvel.y;
                body_set_velocity(star, currvel);
                coll_y = true;
            }
        }
    }
}


int main (int arc, char *argv[]) {
    scene_t *scene = scene_init();

    sdl_init(WINDOW_MIN, WINDOW_MAX);
    vector_t start = (vector_t) {(WINDOW_MAX.x / 2.0), (WINDOW_MAX.y / 2.0)};
    rgb_color_t rgb = {R, G, B};

    list_t *star_shape = make_shape_star(start, POINT_NUM, BIG_R, SMALL_R);
    body_t *star = body_init(star_shape, MASS, rgb);
    body_set_elasticity(star, ELASTICITY);
    body_set_velocity(star, VELOCITY);
    body_set_rot_speed(star, ROT_SPEED);

    scene_add_body(scene, star);

    while (!sdl_is_done(scene)) {
        double dt = time_since_last_tick();
        
        scene_tick(scene, dt);

        // Check for collisions and update velocity
        hit_wall(star);

        sdl_render_scene(scene);
    }

    scene_free(scene);
}
