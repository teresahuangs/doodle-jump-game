#include <stdio.h>
#include <vector.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "polygon.h"
#include "sdl_wrapper.h"
#include "list.h"
#include "body.h"
#include "shape.h"
#include "scene.h"
#include "forces.h"
#include "rand_utils.h"

vector_t WINDOW_MIN = {0.0, 0.0};
vector_t WINDOW_MAX = {1000.0, 500.0};
vector_t CENTER = {500.0, 250.0};
size_t NUM_BALLS = 60;
double CIRC_RAD = 10.0;
size_t N = 100;
double CIRC_ELASTICITY = 0.7;
double CIRC_MASS = 0.5;
rgb_color_t CIRC_NO_COLOR = {1, 1, 1};
double SPRING1 = 40;
double DRAG = 0.5;

scene_t *balls(){
    scene_t *scene = scene_init();

    double scale_x = WINDOW_MAX.x / NUM_BALLS;
    double scale_y = WINDOW_MAX.y / (2 * pow(CENTER.x, 2));
    size_t rad = WINDOW_MAX.x / (2 * NUM_BALLS);

    for (size_t i = 0; i < NUM_BALLS; i++){
        rgb_color_t color = {0.0, 0.0, 0.0};
        color.r = rand_range(0.0, 1.0);
        color.g = rand_range(0.0, 1.0);
        color.b = rand_range(0.0, 1.0);
    
        list_t *circle_points = make_shape_circle(CIRC_RAD, CENTER, N);
        body_t *circle = body_init(circle_points, CIRC_MASS, color);
        // body_set_elasticity(circle, CIRC_ELASTICITY);
        vector_t circle_pos = {scale_x * i + rad, scale_y * pow((scale_x * i) + rad - CENTER.x, 2) + CENTER.y};
        
        list_t *no_circle_points = make_shape_circle(1, CENTER, N);
        body_t *no_circle = body_init(no_circle_points, INFINITY, CIRC_NO_COLOR);
        // body_set_elasticity(no_circle, CIRC_ELASTICITY);
        vector_t no_position = {scale_x * i + rad, CENTER.y};

        body_set_centroid(circle, circle_pos);
        body_set_centroid(no_circle, no_position);

        scene_add_body(scene, no_circle);
        scene_add_body(scene, circle);

        create_spring(scene, SPRING1, circle, no_circle);
        create_drag(scene, DRAG, circle);
    }
    return scene;
}


int main() {
    sdl_init(WINDOW_MIN, WINDOW_MAX);
    scene_t *scene = balls();

    srand((unsigned)time(0));

  
    while (!sdl_is_done(scene)){
        double dt = time_since_last_tick();
        scene_tick(scene, dt);
        sdl_render_scene(scene);
    }
    scene_free(scene);
    return 0;
}