#include "scene.h"
#include "sdl_wrapper.h"
#include <stdlib.h>
#include "shape.h"
#include "forces.h"
#include "rand_utils.h"


#define WINDOW_TITLE "CS 3"
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 500

const size_t NUM_BODIES = 100;
const vector_t WINDOW_MIN = {0, 0};
const vector_t WINDOW_MAX = {WINDOW_WIDTH, WINDOW_HEIGHT};
const size_t NUM_POINTS = 4;
const double OUTER_R_MAX = 10.0;
const double OUTER_R_MIN = 3.0;
const double SCALE = 2.0;
const double MASS_MIN = 1.0;
const double MASS_MAX = 5.0;
const double G = 10000.0;


scene_t *make_bodies_scene() {
    scene_t *scene = scene_init();

    for (size_t i = 0; i < NUM_BODIES; i++) {
        vector_t center = {rand_range(WINDOW_MIN.x, WINDOW_MAX.x), rand_range(WINDOW_MIN.y, WINDOW_MAX.y)};
        double outer_r = rand_range(OUTER_R_MIN, OUTER_R_MAX);
        list_t *star = make_shape_star(center, 4, outer_r, outer_r/SCALE);
        double mass = rand_range(MASS_MIN, MASS_MAX);

        rgb_color_t color = {0.0, 0.0, 0.0};
        color.r = rand_range(0.0, 1.0);
        color.g = rand_range(0.0, 1.0);
        color.b = rand_range(0.0, 1.0);

        body_t *body = body_init(star, mass, color);

        scene_add_body(scene, body);
    }

    return scene;
}

void apply_gravity(scene_t *scene) {
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        for (size_t j = 0; j < i; j++) {
            create_newtonian_gravity(scene, G, scene_get_body(scene, i), scene_get_body(scene, j));
        }
    }
}


int main() {
    // Create n bodies
    sdl_init(WINDOW_MIN, WINDOW_MAX);

    scene_t *scene = make_bodies_scene();

    // Give them gravity
    apply_gravity(scene);


    while (!sdl_is_done(scene)) {
        double dt = time_since_last_tick();
        scene_tick(scene, dt);

        sdl_render_scene(scene);
    }

    scene_free(scene);
}