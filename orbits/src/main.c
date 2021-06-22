#include "jel.h"
#include <SDL2/SDL.h>
#include <stdlib.h>

// Game stuff
static int running = 1;
static int key_up = 0;
static int key_right = 0;
static int key_left = 0;
static int key_down = 0;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
#define FPS 120

struct JEL_Hierarchy *transform_h;
JEL_Entity entities[10];

// Component Stuff
JEL_COMPONENT_DEFINE(Position, int, x, int, y);
JEL_COMPONENT_CREATE(Position, int, x, int, y);

JEL_COMPONENT_DEFINE(Transform, int, x, int, y);
JEL_COMPONENT_CREATE(Transform, int, x, int, y);

JEL_COMPONENT_DEFINE(Orbit, int, dir);
JEL_COMPONENT_CREATE(Orbit, int, dir);

// Systems

void update_orbit(void)
{
  struct JEL_Query *q;
  JEL_QUERY(q, Orbit, Transform);

  for (JEL_ComponentInt i = 0; i < q->tables_num; ++i) {
    struct OrbitFragment *orbit;
    struct TransformFragment *transform;
    JEL_FRAGMENT_GET(orbit, q->tables[i], Orbit);
    JEL_FRAGMENT_GET(transform, q->tables[i], Transform);
    
    for (JEL_EntityInt j = 0; j < q->tables[i]->num; ++j) {
      if (orbit->dir[j] == 1) {
        if (transform->x[j] < 64) {
          ++transform->x[j];
        }
        else {
          orbit->dir[j] = 2;
        }
      }
      else if (orbit->dir[j] == 2) {
        if (transform->y[j] < 64) {
          ++transform->y[j];
        }
        else {
          orbit->dir[j] = 3;
        }
      }
      else if (orbit->dir[j] == 3) {
        if (transform->x[j] > -64) {
          --transform->x[j];
        }
        else {
          orbit->dir[j] = 4;
        }
      }
      else if (orbit->dir[j] == 4) {
        if (transform->y[j] > -64) {
          --transform->y[j];
        }
        else {
          orbit->dir[j] = 1;
        }
      }
    }
  }

  JEL_query_destroy(q);
}

void update_positions_helper(struct JEL_Node *node)
{
  if (node->parent == NULL) {
    return;
  }

  if (node->parent->entity != 0) {
    int my_x;
    int my_y;
    int parent_x;
    int parent_y;
    JEL_ENTITY_GET(node->entity, Transform, x, my_x);
    JEL_ENTITY_GET(node->entity, Transform, y, my_y);
    JEL_ENTITY_GET(node->parent->entity, Transform, x, parent_x);
    JEL_ENTITY_GET(node->parent->entity, Transform, y, parent_y);
    JEL_ENTITY_SET(node->entity, Position, x, parent_x + my_x);
    JEL_ENTITY_SET(node->entity, Position, y, parent_y + my_y);
  }
  else {
    int x;
    int y;
    JEL_ENTITY_GET(node->entity, Transform, x, x);
    JEL_ENTITY_GET(node->entity, Transform, y, y);
    JEL_ENTITY_SET(node->entity, Position, x, x);
    JEL_ENTITY_SET(node->entity, Position, y, y);
  }
}

void update_positions(void)
{
  JEL_hierarchy_iterate_down(transform_h->root, update_positions_helper);
}

void draw_points(void)
{
  struct JEL_Query *q;
  JEL_QUERY(q, Position);

  for (JEL_ComponentInt i = 0; i < q->tables_num; ++i) {
    struct PositionFragment *position;
    JEL_FRAGMENT_GET(position, q->tables[i], Position);
    
    SDL_SetRenderDrawColor(renderer, 28, 72, 128, 255);

    for (JEL_EntityInt j = 0; j < q->tables[i]->num; ++j) {
      SDL_Rect r = {position->x[j] - 8, position->y[j] - 8, 16, 16};
      SDL_RenderFillRect(renderer, &r);
    }
  }

  JEL_query_destroy(q);
}

// Input/Update/Draw
void input(void)
{
  SDL_Event e;

  while (SDL_PollEvent(&e)) {
    switch (e.type) {
      case SDL_QUIT:
        running = 0;
        break;
      case SDL_KEYDOWN:
        switch (e.key.keysym.sym) {
          case SDLK_w:
            key_up = 1;
            break;
          case SDLK_d:
            key_right = 1;
            break;
          case SDLK_s:
            key_down = 1;
            break;
          case SDLK_a:
            key_left = 1;
            break;
        }
        break;
      case SDL_KEYUP:
        switch (e.key.keysym.sym) {
          case SDLK_w:
            key_up = 0;
            break;
          case SDLK_d:
            key_right = 0;
            break;
          case SDLK_s:
            key_down = 0;
            break;
          case SDLK_a:
            key_left = 0;
            break;
        }
        break;
    }
  }
}

void update(void)
{
  if (key_up) {
    JEL_ENTITY_CHANGE(entities[0], Transform, y, -= 1);
  }
  if (key_right) {
    JEL_ENTITY_CHANGE(entities[0], Transform, x, += 1);
  }
  if (key_down) {
    JEL_ENTITY_CHANGE(entities[0], Transform, y, += 1);
  }
  if (key_left) {
    JEL_ENTITY_CHANGE(entities[0], Transform, x, -= 1);
  }

  update_orbit();
  update_positions();
}

void draw(void)
{
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);

  draw_points();

  SDL_RenderPresent(renderer);
}

// Main
int main(int argc, char *args[])
{
  JEL_init();
  SDL_Init(SDL_INIT_EVERYTHING);

  window = SDL_CreateWindow("Orbits", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480, 320, SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, 0);

  JEL_COMPONENT_REGISTER(Position);
  JEL_COMPONENT_REGISTER(Transform);
  JEL_COMPONENT_REGISTER(Orbit);

  // Create hierarchy and nodes
  for (int i = 0; i < 7; ++i) {
    entities[i] = JEL_entity_create();
    JEL_ENTITY_ADD(entities[i], Position, Transform, Orbit);
  }
  struct JEL_Table *temp;
  JEL_TABLE_GET(temp, JEL_EntityC, Position, Transform, Orbit);
  transform_h = JEL_hierarchy_create();
  {
    struct JEL_Node *node = JEL_node_create(entities[0]);
    JEL_ENTITY_SET(node->entity, Transform, x, 0);
    JEL_ENTITY_SET(node->entity, Transform, y, 0);
    JEL_ENTITY_SET(node->entity, Orbit, dir, 0);
    JEL_hierarchy_add(transform_h->root, node);
  }
  {
    struct JEL_Node *node = JEL_node_create(entities[1]);
    JEL_ENTITY_SET(node->entity, Transform, x, 64);
    JEL_ENTITY_SET(node->entity, Transform, y, 64);
    JEL_ENTITY_SET(node->entity, Orbit, dir, 3);
    JEL_hierarchy_add(transform_h->root->child_first, node);
  }
  {
    struct JEL_Node *node = JEL_node_create(entities[2]);
    JEL_ENTITY_SET(node->entity, Transform, x, 64);
    JEL_ENTITY_SET(node->entity, Transform, y, 64);
    JEL_ENTITY_SET(node->entity, Orbit, dir, 3);
    JEL_hierarchy_add(transform_h->root->child_first->child_first, node);
  }
  {
    struct JEL_Node *node = JEL_node_create(entities[3]);
    JEL_ENTITY_SET(node->entity, Transform, x, -64);
    JEL_ENTITY_SET(node->entity, Transform, y, 64);
    JEL_ENTITY_SET(node->entity, Orbit, dir, 1);
    JEL_hierarchy_add(transform_h->root->child_first, node);
  }
  {
    struct JEL_Node *node = JEL_node_create(entities[4]);
    JEL_ENTITY_SET(node->entity, Transform, x, 64);
    JEL_ENTITY_SET(node->entity, Transform, y, -64);
    JEL_ENTITY_SET(node->entity, Orbit, dir, 2);
    JEL_hierarchy_add(transform_h->root->child_first, node);
  }
  {
    struct JEL_Node *node = JEL_node_create(entities[5]);
    JEL_ENTITY_SET(node->entity, Transform, x, 64);
    JEL_ENTITY_SET(node->entity, Transform, y, 64);
    JEL_ENTITY_SET(node->entity, Orbit, dir, 3);
    JEL_hierarchy_add(transform_h->root->child_first->child_first->child_first, node);
  }
  {
    struct JEL_Node *node = JEL_node_create(entities[6]);
    JEL_ENTITY_SET(node->entity, Transform, x, 64);
    JEL_ENTITY_SET(node->entity, Transform, y, 64);
    JEL_ENTITY_SET(node->entity, Orbit, dir, 3);
    JEL_hierarchy_add(transform_h->root->child_first->child_first->sibling_next, node);
  }

  while (running) {
    uint32_t frame_start = SDL_GetTicks();

    input();
    update();
    draw();

    int frame_time = SDL_GetTicks() - frame_start;

    if (1000 / FPS > frame_time) {
      SDL_Delay(1000 / FPS - frame_time);
    }
  }

  JEL_hierarchy_destroy(transform_h);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();
  JEL_quit();

  return 0;
}
