#include "jel.h"
#include <SDL2/SDL.h>

// Game stuff
static int running = 1;
static int create = 0; // Create dots
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
#define FPS 30

// Component Stuff
JEL_COMPONENT_DEFINE(Position, int, x, int, y)
JEL_COMPONENT_CREATE(Position, int, x, int, y)

JEL_COMPONENT_DEFINE(Physics, int, x_vel, int, y_vel)
JEL_COMPONENT_CREATE(Physics, int, x_vel, int, y_vel)

// Prefab
struct JEL_Prefab *cannon_ball;

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
    }
  }

  JEL_query_destroy(q);
}

void draw_dots(void)
{
  struct JEL_Query *q;
  JEL_QUERY(q, Position);

  SDL_SetRenderDrawColor(renderer, 28, 72, 128, 255);
  
  for (JEL_ComponentInt i = 0; i < q->tables_num; ++i) {
    struct PositionFragment *position;
    JEL_FRAGMENT_GET(position, q->tables[i], Position);  

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
        }
        break;
      }
    }
  }
}

void update(void)
{
  if (create) {
    JEL_prefab_generate(cannon_ball);
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

  window = SDL_CreateWindow("Cannon", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480, 320, 0);
  renderer = SDL_CreateRenderer(window, -1, 0);

  JEL_COMPONENT_REGISTER(Position);
  JEL_COMPONENT_REGISTER(Physics);

  /*
  uint8_t *data = malloc(sizeof(JEL_Entity) + 4 * sizeof(int));
  int *temp = (int *) (data + sizeof(JEL_Entity));
  *temp = 0;
  *(++temp) = 160;
  *(++temp) = 32;
  *(++temp) = 0;
  */
  JEL_Type prefab_type;
  JEL_type_init(prefab_type);
  JEL_TYPE_ADD(prefab_type, Position, Physics);
  cannon_ball = JEL_prefab_create(
    prefab_type,
    JEL_data_create(5,
      sizeof(JEL_Entity), sizeof(int), sizeof(int), sizeof(int), sizeof(int),
      0,                  0,           160,         32,          0
    )
  );

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

  JEL_prefab_destroy(cannon_ball);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();
  JEL_quit();

  return 0;
}
