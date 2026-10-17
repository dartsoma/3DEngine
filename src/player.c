#include "entity_component.h"
#include "entity_core.h"
#include "gfc_color.h"
#include "simple_logger.h"
#include "model.h"

#include "player.h"

Entity* player_create() {
    Entity *player;

    player = entity_create();

    player->free = player_free;
    player->update = player_update;
    player->think = player_think;
    player->draw = player_draw;

    player->tag |= EntityRender;

    RenderComponent *render = (RenderComponent*) entity_get_own_render(player);

    render->model = model_load("models/dino/dino.model");
    gfc_matrix4_identity(render->mat);
    render->color = GFC_COLOR_WHITE;

    return player;
}

void player_free(Entity *self){


return;
}

void player_think(Entity *self){

return;
}

void player_update(Entity *self){

return;
}

void player_draw(Entity *self){

    RenderComponent *render = (RenderComponent*) entity_get_own_render(self);

    gf3d_model_queue_render(render->model, render->mat, render->color);
return;
}
