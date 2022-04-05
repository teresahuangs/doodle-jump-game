#include <stdio.h>

#include <vector.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
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

#include "game_make_objects.h"
#include "game_screen.h"
#include "game_actions.h"

#include <assert.h>

/* ###################################
 * TODO BUGS for later:
 *  * Fix resizing window/ moving window bug by pausing game temporarily, some SDL magic
 * ###################################
 */

// window info
const vector_t WINDOW_MIN = {0.0, 0.0};
const vector_t WINDOW_MAX = {500.0, 1000.0};

// base info
const double BASE_SIZE = 10.0;
const rgb_color_t BASE_COLOR = {1.0, 1.0, 1.0};
const double BASE_MASS = 0.1;

// sprite info
const double SPRITE_RAD = 30.0;
const double SPRITE_MASS = 20.0;
const rgb_color_t SPRITE_COLOR = {1.0, 0.0, 1.0};
const vector_t X_VELO = {500.0, 0.0};
const double BOUNCE_HEIGHT = 300.0;
const double SPRITE_MAX_HEIGHT = 500.0;
const size_t SPRITE_RESOLUTION = 40;
const double EPSILON = 2.0;
const image_t SPRITE_IMAGE = {"media/grasshopper.png", 60, 60};
const image_t SPRITE_JET_IMAGE = {"media/grasshopper_jet.png", 60, 60};

// platform information 
const double PLATFORM_HEIGHT = 10.0; 
const double PLATFORM_WIDTH = 80.0; 
const double PLATFORM_MASS = INFINITY; 
const double PLATFORM_ELASTICITY = 1.0;
const double NUM_PLATFORMS_PER_SCREEN = 8.0;
const vector_t VEC_MOVING = {50.0, 0.0};

const rgb_color_t PLATFORM_COLOR = {0.0, 0.0, 1.0};
const image_t PLATFORM_IMAGE = {"media/platform.jpg", 80, 10};

const rgb_color_t MOVING_COLOR = {0.0, 1.0, 0.0};
const image_t MOVING_PLAT_IMAGE = {"media/moving_platform.jpg", 80, 10};

const rgb_color_t BREAKING_PLAT_COLOR = {1.0, 0.0, 0.0};
const image_t BREAKING_PLAT_IMAGE = {"media/breaking_platform.jpg", 80, 10};

//start screen 
const image_t START_SCREEN_IMAGE = {"media/start_screen.png", 500, 1000};

//death screen 
const image_t DEATH_SCREEN_IMAGE = {NULL, 500, 1000};

// monster info
const double MONSTER_RAD = 30.0;
const double MONSTER_MASS = 20.0;
const rgb_color_t MONSTER_COLOR = {0.0, 0.0, 1.0};
const rgb_color_t MOVING_MONSTER_COLOR = {0.0, 1.0, 1.0};
const vector_t MONSTER_MOVING_VEL = {100.0, 0.0};
const size_t MONSTER_N = 100;
const size_t MONSTER_RESOLUTION = 40;
const image_t MONSTER_IMAGE = {"media/bird.png", 60, 60};
const double MONSTER_MIN = 50.0;
const double MONSTER_MAX = 1000.0;

// sprite's bullet information
const vector_t BULLET_VEL = {0.0, 150.0};
const double SHOT_SIZE = 5.0;
const double SHOT_MASS = 1.0;
const rgb_color_t DEFEND_COLOR = {0.0, 0.8, 0.0};
const vector_t DEFEND_SHOT_VELOCITY = {0.0, 800.0};
const size_t SHOT_RESOLUTION = 20;

// spring information
const double SPRING_SIZE = 20.0;
const double SPRING_MASS = 1.0;
const rgb_color_t SPRING_COLOR = {0.5, 0.5, 0.5};
const double SPRING_BOUNCE_HEIGHT = 800.0;
const double SPRING_PROB = 0.2;
const double SPRING_SCORE = 50.0;
const image_t SPRING_IMAGE = {"media/spring.png", 20, 20};

// Jet information
const double JET_SIZE = 20.0;
const double JET_MASS = 10.0;
const rgb_color_t JET_COLOR = {1.0, 0.0, 0.0};
const image_t JET_IMAGE = {"media/jet_boots.png", 20, 20};
const double LOW_ACC = 1000;
const double HIGH_ACC = 4000;
const double STOP_ACC = 300;
const double START_DEC = 3500;
const double JET_PROB = 0.1;

// Constant acceleration downwards
const double ACC = 2000;

// Indicator information
const double INDICATOR_HEIGHT = 10.0;
const double INDICATOR_WIDTH = 50.0;
const rgb_color_t INDICATOR_COLOR = {1.0, 1.0, 0.0};
const double INDICATOR_MASS = 0.1;
const double INDICATOR_SIZE = 1;

// scoretile info
const double SCORETILE_HEIGHT = 30.0;
const double SCORETILE_WIDTH = 50.0;
const rgb_color_t SCORETILE_COLOR = {0.0, 0.0, 0.0};
const double SCORETILE_MASS = 0.1;
const double SCORETILE_SIZE = 1;



// ===== GROUPINGS =====
// makes num_platforms unmovable platform randomly spaced in the x direction at the top of the screen
// these range a double of height above the screen so then when we move them down they will fill the right area
// Returns the max height of the top rendered object, relative to the screen base.
double make_constant_platforms(scene_t *scene, double start_y, double height, double min_jump) {
    double y_pos = start_y;
    while (y_pos < start_y + height) {
        y_pos += rand_range(min_jump, BOUNCE_HEIGHT - EPSILON);
        vector_t center = {rand_range(WINDOW_MIN.x, WINDOW_MAX.x - EPSILON), y_pos};
        vector_t velo = (vector_t) {0.0, 0.0};
        rgb_color_t color = PLATFORM_COLOR;
        image_t image = PLATFORM_IMAGE;
        if (rand_range(0, 1) < moving_plat_ratio) {
            if (rand_range(0, 1) > 0.5) {
                velo = vec_subtract(velo, VEC_MOVING);
            }
            else{
                velo = vec_add(velo, VEC_MOVING);
            }
            color = MOVING_COLOR;
            image = MOVING_PLAT_IMAGE;
        }
        if (velo.x == 0.0 && rand_range(0, 1) < SPRING_PROB) {
            vector_t base = vec_add(center, (vector_t) {rand_range(-PLATFORM_WIDTH/2 + SPRING_SIZE/2, PLATFORM_WIDTH/2 - SPRING_SIZE/2), PLATFORM_HEIGHT/2});
            make_spring(scene, base, SPRING_SIZE, SPRING_MASS, ACC, SPRING_BOUNCE_HEIGHT, SPRING_COLOR, SPRING_IMAGE);
        }
        else if (velo.x == 0.0 && rand_range(0, 1) < JET_PROB) {
            vector_t base = vec_add(center, (vector_t) {rand_range(-PLATFORM_WIDTH/2 + JET_SIZE/2, PLATFORM_WIDTH/2 - JET_SIZE/2), PLATFORM_HEIGHT/2});
            make_jet(scene, base, JET_SIZE, JET_MASS, JET_COLOR, ACC, LOW_ACC, HIGH_ACC, STOP_ACC, START_DEC);
        }
        make_platform(scene, center, PLATFORM_WIDTH, PLATFORM_HEIGHT, PLATFORM_MASS, velo, color, image, ACC, BOUNCE_HEIGHT, platform_collision);
    }

    return y_pos;
}

// makes the indicators on the screen 
void make_indicators(scene_t *scene, list_t *centroids) {
    for (size_t i = 0; i < list_size(centroids); i++) {
        vector_t curr_centroid = *(vector_t *) list_get(centroids, i);
        body_t *indicator = make_indicator(scene, curr_centroid, INDICATOR_MASS, INDICATOR_WIDTH, INDICATOR_HEIGHT, INDICATOR_COLOR);
    }
}

level_info_t levels(double elevation) {
    size_t level_index = 0;
    for (level_index = 0; level_index < LEVEL_COUNT; level_index ++) {
        if (elevation < LEVEL_TRIGGERS[level_index]) {
            break;
        }
    }



void make_breaking_platforms(scene_t *scene, double start_y, double height, double num_platforms_per_screen) {
    double num_platforms = num_platforms_per_screen * height / WINDOW_MAX.y;

    if (num_platforms < 0) {
        num_platforms = - num_platforms;
    }

    if (rand_range(0.0, 1.0) < num_platforms - (double) floor(num_platforms)) {
        num_platforms ++;
    }

    bool overlap = true;
    vector_t center;
    for (size_t i = 0; i < (size_t) num_platforms; i++) {
        while (overlap) {
            center = (vector_t) {rand_range(WINDOW_MIN.x, WINDOW_MAX.x), start_y + rand_range(0.0, height)};
            body_t *plat = make_platform(scene, center, PLATFORM_WIDTH, PLATFORM_HEIGHT, PLATFORM_MASS, VEC_ZERO, BREAKING_PLAT_COLOR, BREAKING_PLAT_IMAGE, ACC, BOUNCE_HEIGHT, breaking_platform_collision);

            // Check for overlap
            overlap = false;
            for (size_t i = 0; i < scene_bodies(scene)-1; i++) {
                body_t *body = scene_get_body(scene, i);
                if (body_get_type(body) == PLATFORM && find_collision(body_get_shape(plat), body_get_shape(body)).collided && !overlap) {
                    overlap = true;
                    scene_remove_body(scene, scene_bodies(scene) - 1);
                }
            }
        }
    }

}

// Spawns in monsters
double make_monsters(scene_t *scene, double start_y, double height, double min_distance, double max_distance) {
    double y_pos = start_y;
    while (y_pos < start_y + height) {
        y_pos += rand_range(min_distance, max_distance);
        vector_t center = {rand_range(WINDOW_MIN.x, WINDOW_MAX.x - EPSILON), y_pos};
        vector_t velo = (vector_t) {0.0, 0.0};
        rgb_color_t color = MONSTER_COLOR;
        if (rand_range(0, 1) > 0.8) {
            if (rand_range(0, 1) > 0.5){
                velo = vec_subtract(velo, MONSTER_MOVING_VEL);
            }
            else{
                velo = vec_add(velo, MONSTER_MOVING_VEL);
            }
            color = MOVING_MONSTER_COLOR;
        }
        if (y_pos < start_y + height) {
            make_monster(scene, center, MONSTER_RAD, MONSTER_MASS, velo, color, MONSTER_RESOLUTION, MONSTER_FILE_NAME);
        }
    }

    return y_pos;
}
// ===== SCREEN DYNAMICS =====

// Adds one more screen height section to the scene. Returns the elevation of the highest platform created. that is the score
double extend_scene(scene_t *scene, double start_y) {
    double max_y = make_constant_platforms(scene, start_y, WINDOW_MAX.y - WINDOW_MIN.y, PLATFORM_HEIGHT);
    make_breaking_platforms(scene, start_y, WINDOW_MAX.y - WINDOW_MIN.y, NUM_PLATFORMS_PER_SCREEN);
    make_monsters(scene, start_y, WINDOW_MAX.y - WINDOW_MIN.y, MONSTER_MIN, MONSTER_MAX);
    // levels(scene, start_y, WINDOW_MAX.y - WINDOW_MIN.y, PLATFORM_HEIGHT);


    double y = WINDOW_MAX.y * 0.75;
    double x_min = WINDOW_MIN.x + MONSTER_RAD;
    double x_max = WINDOW_MAX.x - MONSTER_RAD;
    double x = rand_range(x_min, x_max);
    vector_t vec = {x, y};

    return max_y;
}

// moves the screen according to the sprite position. Returns current elevation of the screen view.
double move_screen(scene_t *scene, double highest_plat_elevation, size_t level_index) {
    level_info_t level_info;
    // DEBUGGING LEVEL STUFF
    if (level_index <LEVEL_COUNT) {
        level_info = LEVEL_INFO[level_index];
    }
    else {
        level_info = levels(highest_plat_elevation);
    }

    body_t *sprite = scene_get_body(scene, 1);
    double y_change = body_get_centroid(sprite).y - SPRITE_MAX_HEIGHT;
    double elevation = -body_get_centroid(scene_get_body(scene, 0)).y;

    if (y_change > 0) {
        // Add enough objects to the scene to keep the game going.
        while (highest_plat_elevation < elevation + (WINDOW_MAX.y - WINDOW_MIN.y) + y_change) {
            highest_plat_elevation = extend_scene(scene, highest_plat_elevation - elevation) + elevation;
        }

        // Now we shift everything.
        elevation += y_change;
        for (size_t i = 0; i < scene_bodies(scene); i++) {
            body_t *body = scene_get_body(scene, i);
            vector_t center = body_get_centroid(body);
            center = vec_add(center, (vector_t) {0.0, - y_change});
            // Deletes bodies that exit the screen.
            body_type_t type = body_get_type(body);
            if (center.y <  WINDOW_MIN.y && (type != BASE) && (type != INDICATOR)) {
                scene_remove_body(scene, i);
            }
            else {
                body_set_centroid(body, center);
            }
        }
    }  

    return highest_plat_elevation;
}

// PERHAPS SHOULD BE A COLLISION HANDLER WITH LEFT/RIGHT WALLS BUT THIS WILL DO
void wrap(scene_t *scene) {
    body_t *sprite = scene_get_body(scene, 1);
    vector_t curr_centroid = body_get_centroid(sprite);

    if (curr_centroid.x < WINDOW_MIN.x) {
        vector_t new_centroid = {WINDOW_MAX.x, curr_centroid.y};
        body_set_centroid(sprite, new_centroid);
    }

    if (curr_centroid.x > WINDOW_MAX.x) {
        vector_t new_centroid = {(WINDOW_MIN.x), curr_centroid.y};
        body_set_centroid(sprite, new_centroid);
    }
}

void blocks_wrap(scene_t *scene) {
    for (int i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        if (body_get_type(body) == PLATFORM || body_get_type(body) == MONSTER) {
            vector_t curr_centroid = body_get_centroid(body);
            if (curr_centroid.x < WINDOW_MIN.x || curr_centroid.x > WINDOW_MAX.x) {
                vector_t v = body_get_velocity(body);
                body_set_velocity(body, (vector_t) {-v.x, v.y});
            }
        }
    }
}

// ===== BODY ACTIONS =====
// shoots a bullet from player 
void defend(scene_t *scene) {
    vector_t location = body_get_centroid(scene_get_body(scene, 1));
    body_t *shot = make_shot(scene, location, SHOT_SIZE, SHOT_MASS, DEFEND_COLOR, DEFEND_SHOT_VELOCITY, SHOT_RESOLUTION);
    size_t size = scene_bodies(scene);
    for (size_t i = 0; i < size; i++) {
        body_t *body = scene_get_body(scene, i);
        if (body_get_type(body) == MONSTER) {
            create_destructive_collision(scene, body, shot);
        }
    }
}


// ===== START AND END STATE =====
// Resets the screen.
scene_t *reset(scene_t *old_scene) {
    list_t *centroids = list_init(INDICATOR_SIZE, free);
    if (scene_bodies(old_scene) != 0) {
        body_t *old_base = scene_get_body(old_scene, 0);
        for (size_t i = 0; i < scene_bodies(old_scene); i++) {
            body_t *curr_body = scene_get_body(old_scene, i);
            if (body_get_type(curr_body) == INDICATOR) {
                vector_t *cent = malloc(sizeof(vector_t));
                *cent = body_get_centroid(curr_body);
                cent->y += -body_get_centroid(old_base).y;
                list_add(centroids, cent);
            }
        }
    }
    
    scene_free(old_scene);
    scene_t *scene = scene_init();
    body_t *base = make_base(scene, (vector_t) {(WINDOW_MAX.x - WINDOW_MIN.x)/2, WINDOW_MIN.y}, BASE_MASS, BASE_SIZE, BASE_COLOR);
    vector_t start = {0.5 * WINDOW_MAX.x, WINDOW_MAX.y * 0.5};
    body_t *sprite = make_sprite(scene, start, SPRITE_RAD, SPRITE_MASS, SPRITE_COLOR, SPRITE_RESOLUTION, ACC, SPRITE_IMAGE, SPRITE_JET_IMAGE);
    // Make sprite jump up at start so player has time to move
    make_indicators(scene, centroids);
    make_scoretiles(scene, centroids);
    list_free(centroids);

    return scene;
}


// makes the start screen 
void start_screen(scene_t *scene) {
    // counter sprite movement: 
    body_t *sprite = scene_get_body(scene, 1);
    list_t *bodies = list_init(1, NULL);
    list_add(bodies, sprite);
    double *acc = malloc(sizeof(double));
    *acc = - ACC;
    param_t *param = param_init(bodies, acc, free);
    scene_add_bodies_force_creator(scene, accelerate_down, param, bodies, param_free);

    // make start screen:
    vector_t center = vec_add(WINDOW_MIN, vec_multiply(0.5, WINDOW_MAX));
    list_t *screen_points = make_shape_rectangle(WINDOW_MAX.x - WINDOW_MIN.x, WINDOW_MAX.y - WINDOW_MIN.y, center);
    body_t *screen = body_init_with_info(screen_points, INFINITY, PLATFORM_COLOR, START_SCREEN);
    list_t *images = list_init(1, free);

    image_t *image = malloc(sizeof(image_t));
    *image = START_SCREEN_IMAGE;
    list_add(images, image);
    body_set_skin(screen, skin_init(make_costumes_from_images(images), true));

    scene_add_body(scene, screen);
}

scene_t *death_screen(scene_t *scene){
    scene = reset(scene); 
    for (size_t i = 0; i < scene_bodies(scene); i++){
        scene_remove_body(scene, 0);
    }
    vector_t center = vec_add(WINDOW_MIN, vec_multiply(0.5, WINDOW_MAX));
    list_t *screen_points = make_shape_rectangle(WINDOW_MAX.x - WINDOW_MIN.x, WINDOW_MAX.y - WINDOW_MIN.y, center);
    body_t *screen = body_init_with_info(screen_points, INFINITY, PLATFORM_COLOR, DEATH_SCREEN);
    
    list_t *images = list_init(1, free);
    image_t *image = malloc(sizeof(image_t));
    *image = DEATH_SCREEN_IMAGE;
    list_add(images, image);
    body_set_skin(screen, skin_init(make_costumes_from_images(images), true));
    
    scene_add_body(scene, screen);
    return scene;
}



// Death case
scene_t *death(scene_t *scene) {
    body_t *base = scene_get_body(scene, 0);
    double distance = SPRITE_MAX_HEIGHT;
    vector_t cent = {(WINDOW_MAX.x - (0.5 * INDICATOR_WIDTH)), distance};
    body_t *indicator = make_indicator(scene, cent, INDICATOR_MASS, INDICATOR_WIDTH, INDICATOR_HEIGHT, INDICATOR_COLOR);
    body_t *scoretile = make_scoretile(scene, cent, SCORETILE_MASS, SCORETILE_WIDTH, SCORETILE_HEIGHT, SCORETILE_COLOR);
    scene = reset(scene);
    return scene;
}

bool dead(scene_t *scene){
    for (size_t i = 0; i < scene_bodies(scene); i++){
        body_t *body = scene_get_body(scene, i);
        if (body_get_type(body) == DEATH_SCREEN) {
            return true;
        }
    }
    return false;
}

bool start(scene_t *scene){
    for (size_t i = 0; i < scene_bodies(scene); i++){
        body_t *body = scene_get_body(scene, i);
        if (body_get_type(body) == START_SCREEN){
            return true;
        }
    }
    return false;
}


// base block spawned in at the base of every game
// write a function that makes a "platform" once the player dies
// "game score platforms" will need to be stored somehow in a list
// all game score platforms will need be generated when the sprite passes it


// handles key input from player
void on_key(scene_t *scene, char key, key_event_type_t type, double held_time) {
    body_t *sprite = scene_get_body(scene, 1);
    vector_t vel = body_get_velocity(sprite);
    vector_t curr_centroid = body_get_centroid(sprite);
    if (type == KEY_PRESSED) {
        switch (key) {
            case LEFT_ARROW:                
                body_set_velocity(sprite, (vector_t) {-X_VELO.x, vel.y});
                break;

            case RIGHT_ARROW:
                body_set_velocity(sprite, (vector_t) {X_VELO.x, vel.y});
                break;

            case SDLK_SPACE:
                defend(scene);
                break;

            case SDLK_s: 
                body_set_type(sprite, SPRITE_DEAD);
                break;
        }
    }
    else if (type == KEY_RELEASED) {
        switch (key) {
            case LEFT_ARROW:
            if (vel.x < 0) {
                body_set_velocity(sprite, (vector_t) {0.0, vel.y});
            }
            break;

            case RIGHT_ARROW:
            if (vel.x > 0) {
                body_set_velocity(sprite,  (vector_t) {0.0, vel.y});
            }
            break;  
        }
    }
}

// int main to test code periodically, update as necessary 
int main() {
    sdl_init(WINDOW_MIN, WINDOW_MAX);
    srand(time(NULL));

    scene_t *scene = scene_init();
    scene = reset(scene);
    sdl_on_key(on_key);
    // start_screen(scene);

    // DEBUGGING LEVELS
    size_t level_index = LEVEL_COUNT;
    if (argc == 2) {
        level_index = atoi(argv[1]);
    }

    body_t *sprite = scene_get_body(scene, 1);
    // Make a first platform and make this one the start point.
    vector_t first_plat_centroid = {body_get_centroid(sprite).x, rand_range(PLATFORM_HEIGHT, body_get_centroid(sprite).y * 0.5)};
    make_platform(scene, first_plat_centroid, PLATFORM_WIDTH, PLATFORM_HEIGHT, PLATFORM_MASS, VEC_ZERO, PLATFORM_COLOR, PLATFORM_IMAGE, ACC, BOUNCE_HEIGHT, platform_collision);


    double highest_plat_elevation = extend_scene(scene, first_plat_centroid.y, LEVEL_INFO[0]);

    // double highest_plat_elevation = 0;
    start_screen(scene);

    while (!sdl_is_done(scene)) {

        double dt = time_since_last_tick();

        if (!dead(scene)){
            wrap(scene);
            blocks_wrap(scene);

        if ((body_get_type(scene_get_body(scene, 1)) != SPRITE && body_get_type(scene_get_body(scene, 1)) != SPRITE_INVUL) || body_get_centroid(sprite).y < WINDOW_MIN.y) {
            scene = death(scene);
            vector_t first_plat_centroid = {body_get_centroid(sprite).x, rand_range(PLATFORM_HEIGHT, body_get_centroid(sprite).y * 0.5)};
            make_platform(scene, first_plat_centroid, PLATFORM_WIDTH, PLATFORM_HEIGHT, PLATFORM_MASS, VEC_ZERO, PLATFORM_COLOR, ACC, BOUNCE_HEIGHT, platform_collision);
            highest_plat_elevation = extend_scene(scene, first_plat_centroid.y);
            sprite = scene_get_body(scene, 1); 
        }

        highest_plat_elevation = move_screen(scene, highest_plat_elevation);
            if ((body_get_type(scene_get_body(scene, 1)) != SPRITE && body_get_type(scene_get_body(scene, 1)) != SPRITE_INVUL) || body_get_centroid(sprite).y < WINDOW_MIN.y) {
                scene = death(scene);
                vector_t first_plat_centroid = {body_get_centroid(sprite).x, rand_range(PLATFORM_HEIGHT, body_get_centroid(sprite).y * 0.5)};
                make_platform(scene, first_plat_centroid, PLATFORM_WIDTH, PLATFORM_HEIGHT, PLATFORM_MASS, VEC_ZERO, PLATFORM_COLOR, PLATFORM_IMAGE, ACC, BOUNCE_HEIGHT, platform_collision);
                highest_plat_elevation = extend_scene(scene, first_plat_centroid.y, LEVEL_INFO[0]);
                sprite = scene_get_body(scene, 1); 
            }

            highest_plat_elevation = move_screen(scene, highest_plat_elevation, level_index);

        }
        scene_tick(scene, dt);
        
        sdl_render_scene(scene);
        sdl_draw_sprite(SPRITE_FILE_NAME, sprite, WINDOW_MAX);

    }

    scene_free(scene);
    return 0;
}