#include "jel.h"
#include <SDL2/SDL.h>

// Game stuff
static int running = 1;
static int bounce = 0; // Make dots with physics
static int create = 0; // Create dots
static int mouse_x, mouse_y;
static int window_w, window_h;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
#define FPS 30

// Component Stuff
JEL_COMPONENT_DEFINE(Position, int, x, int, y)
JEL_COMPONENT_CREATE(Position, int, x, int, y)

JEL_COMPONENT_DEFINE(Physics, int, x_vel, int, y_vel)
JEL_COMPONENT_CREATE(Physics, int, x_vel, int, y_vel)

// Systems
void physics(void)
{
  struct JEL_Query *q;
  JEL_QUERY(q, Position, Physics);

  for (JEL_ComponentInt i = 0; i < q->tables_num; ++i) {
    struct PositionFragment *position;
    struct PhysicsFragment *physics;
    JEL_FRAGMENT_GET(position, q->tables[i], Position);
    JEL_FRAGMENT_GET(physics, q->tables[i], Physics);

    for (JEL_EntityInt j = 0; j < q->tables[i]->num; ++j) {
      position->x[j] += physics->x_vel[j];
      position->y[j] += physics->y_vel[j];

      if (position->x[j] < 0 || position->x[j] > window_w) {
        physics->x_vel[j] = -physics->x_vel[j];
      }
      if (position->y[j] < 0 || position->y[j] > window_h) {
        physics->y_vel[j] = -physics->y_vel[j];
      }
    }
  }

  JEL_query_destroy(q);
}

void draw_dots(void)
{
  struct JEL_Query *q;
  JEL_QUERY(q, Position);

  for (JEL_ComponentInt i = 0; i < q->tables_num; ++i) {
    struct PositionFragment *position;
    JEL_FRAGMENT_GET(position, q->tables[i], Position);
    
    SDL_SetRenderDrawColor(renderer, 28, 72, 128, 255);

    for (JEL_EntityInt j = 0; j < q->tables[i]->num; ++j) {
      SDL_Rect r = {position->x[j], position->y[j], 16, 16};
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
      case SDL_QUIT: {
        running = 0;
        break;
      }
      case SDL_MOUSEBUTTONDOWN: {
        switch (e.button.button) {
          case SDL_BUTTON_LEFT: {
            create = 1;
            break;
          }
        }
        break;
      }
      case SDL_MOUSEBUTTONUP: {
        switch (e.button.button) {
          case SDL_BUTTON_LEFT: {
            create = 0;
            break;
          }
          case SDL_BUTTON_RIGHT: {
            bounce = !bounce;
            break;
          }
        }
        break;
      }
    }
  }

  SDL_GetMouseState(&mouse_x, &mouse_y);
  SDL_GetWindowSize(window, &window_w, &window_h);
}

void update(void)
{
  if (create) {
    JEL_Entity e = JEL_entity_create();
    JEL_ENTITY_ADD(e, Position);
    JEL_ENTITY_SET(e, Position, x, mouse_x);
    JEL_ENTITY_SET(e, Position, y, mouse_y);

    if (bounce) {
      JEL_ENTITY_ADD(e, Physics);
      JEL_ENTITY_SET(e, Physics, x_vel, 8);
      JEL_ENTITY_SET(e, Physics, y_vel, -8);
    }
  }

  physics();
}

void draw(void)
{
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);

  draw_dots();

  SDL_RenderPresent(renderer);
}

// Main
int main(int argc, char *args[])
{
  JEL_init();
  SDL_Init(SDL_INIT_EVERYTHING);

  window = SDL_CreateWindow("Dots", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480, 320, SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, 0);

  JEL_COMPONENT_REGISTER(Position);
  JEL_COMPONENT_REGISTER(Physics);

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

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();
  JEL_quit();

  return 0;
}
