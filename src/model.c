#include "model.h"
#include "simple_logger.h"

typedef struct {
  Model *modelList;
  Uint32 modelCount;
  Pipeline *pipe;
  Uint32 chainLength;
  Texture *defaultTexture;
  VkDevice device;
} ModelManager;

ModelManager model_manager = {0};

void model_system_init(Uint32 max_models) {

  // set default texture
  gf3d_mesh_init(max_models);

  if (max_models == 0) {
    slog("bad max count");
    return;
  }

  model_manager.chainLength = gf3d_swapchain_get_chain_length();
  model_manager.modelList =
      (Model *)gfc_allocate_array(sizeof(Model), max_models);
  model_manager.modelCount = max_models;
  model_manager.device = gf3d_vgraphics_get_default_logical_device();
  model_manager.pipe = gf3d_pipeline_create_from_config(
      gf3d_vgraphics_get_default_logical_device(), "config/model_pipeline.cfg",
      gf3d_vgraphics_get_view_extent(), max_models,
      gf3d_mesh_get_bind_description(), gf3d_mesh_get_attribute_descriptions(),
      3, // attribute count
      sizeof(ModelUBO), VK_INDEX_TYPE_UINT32);
  model_manager.defaultTexture = gf3d_texture_load("images/default.png");

  if (!model_manager.pipe) {
    slog("failed pipeline");
    exit(1);
  }

  atexit(model_close);
}

void model_close() {
  int i;
  for (i = 0; i < model_manager.modelCount; i++) {
    model_delete(&model_manager.modelList[i]);
  }
  if (model_manager.modelList) {
    free(model_manager.modelList);
  }
  memset(&model_manager, 0, sizeof(ModelManager));
  slog("model manager closed");
}

Model *model_new() {

  int i;
  for (i = 0; i < model_manager.modelCount; i++) {
    if (model_manager.modelList[i]._refCount == 0 &&
        strlen(model_manager.modelList[i].filename) == 0) {

      model_delete(&model_manager.modelList[i]);
      model_manager.modelList[i]._refCount = 1;
      return &model_manager.modelList[i];
    }
  }
  for (i = 0; i < model_manager.modelCount; i++) {
    if (!model_manager.modelList[i]._refCount) {
      model_delete(&model_manager.modelList[i]);
      model_manager.modelList[i]._refCount = 1;
      return &model_manager.modelList[i];
    }
  }
  slog("unable to make a new model, out of space");
  return NULL;
}

void model_free(Model *model) {
  if (!model)
    return;
  model->_refCount--;
}

void model_delete(Model *model) {

  gf3d_mesh_free(model->mesh);
  gf3d_texture_free(model->texture);
  memset(model, 0, sizeof(Model));
}

Model *model_get_by_filename(const char *filename) {
  int c;
  if (!filename)
    return NULL;

  for (c = 0; c < model_manager.modelCount; c++) {
    if (model_manager.modelList[c]._refCount == 0)
      continue;
    if (!model_manager.modelList[c].filename)
      continue;

    if (gfc_strlcmp(model_manager.modelList[c].filename, filename) == 0)
      return &model_manager.modelList[c];
  }

  return NULL;
}

void gf3d_model_reset_pipes() {

  Uint32 bufferFrame = gf3d_vgraphics_get_current_buffer_frame();

  gf3d_pipeline_reset_frame(model_manager.pipe, bufferFrame);
}

/**
 * @brief called to submit all draw commands to the mesh pipelines
 */
void gf3d_model_submit_pipe_commands() {

  gf3d_pipeline_submit_commands(model_manager.pipe);
}

/**
 * @brief get the current command buffer for the mesh system
 */
VkCommandBuffer gf3d_model_get_model_command_buffer() {

  if (!model_manager.pipe)
    return VK_NULL_HANDLE;
  return model_manager.pipe->commandBuffer;
}

/**
 * @brief queue up a render for the current draw frame
 * @param mesh the mesh to render
 * @param pipe the pipeline to use
 * @param uboData the data to use to draw the mesh
 * @param texture texture data to use
 */

void gf3d_model_queue_render(Model *model, GFC_Matrix4 mat,
                             GFC_Color colormod) {
  if (!model)
    return;

  ModelUBO ubo = model_get_ubo(mat, colormod);
  gf3d_mesh_queue_render(model->mesh, model_manager.pipe, &ubo, model->texture);
}

Model *model_load(const char *filename) {
  const char *str = NULL;
  const char *str2 = NULL;
  SJson *json, *data;
  Mesh *mesh;
  Model *model;
  Texture *texture = NULL;

  if (!filename) {
    return NULL;
  }
  model = model_get_by_filename(filename);
  if (model) {
    model->_refCount++;
    return model;
  }

  json = sj_load(filename);
  if (!json) {
    slog("failed load model %s", filename);
    return NULL;
  }

  data = sj_object_get_value(json, "model");
  if (!data) {
    slog("no model info in file %s", filename);
    sj_free(json);
    return NULL;
  }

  str = sj_object_get_string(data, "obj");
  if (!str) {
    slog("no object data in file %s", filename);
    sj_free(json);
    return NULL;
  }
  mesh = gf3d_mesh_load_obj(str);
  if (!mesh) {
    slog("failed to get mesh in %s", str);
    sj_free(json);
    return NULL;
  }

  str2 = sj_object_get_string(data, "texture");
  if (str2) {
    texture = gf3d_texture_load(str2);
    slog("set pathed texture");
  }
  model = model_new();

  if (!model) {
    if (texture) {
      gf3d_texture_free(texture);
    }
    model_free(model);
    return model;
  }

  if (!texture) {
    texture = model_manager.defaultTexture;
    slog("set default texture");
  }

  model->texture = texture;
  model->mesh = mesh;
  sj_free(json);
  return model;
}
ModelUBO model_get_ubo(GFC_Matrix4 modelMat, GFC_Color colormod) {
  ModelUBO ubo = {0};
  GFC_Matrix4 view;
  GFC_Matrix4 projection;
  // gfc_matrix4_copy(ubo.model,modelMat);
  memcpy(ubo.model, modelMat, sizeof(GFC_Matrix4));

  gf3d_vgraphics_get_view(&view);

  memcpy(ubo.view, view, sizeof(GFC_Matrix4));

  gf3d_vgraphics_get_projection_matrix(&projection);

  memcpy(ubo.proj, projection, sizeof(GFC_Matrix4));

  ubo.color = gfc_color_to_vector4f(colormod);
  /*
    gfc_matrix4_slog(ubo.model);
    slog("");
    gfc_matrix4_slog(ubo.view);
    slog("");
    gfc_matrix4_slog(ubo.proj);
    slog("");
  */
  return ubo;
}

Pipeline *model_get_pipeline() { return model_manager.pipe; }
