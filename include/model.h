#ifndef __MODEL_H__
#define __MODEL_H__

#include "simple_json.h"

#include "gf3d_camera.h"
#include "gf3d_commands.h"
#include "gf3d_mesh.h"
#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"
#include "gf3d_texture.h"
#include "gf3d_vgraphics.h"
#include "gfc_color.h"
#include "gfc_matrix.h"
#include "gfc_text.h"
#include "gfc_types.h"
#include "gfc_vector.h"

typedef struct {

  GFC_Matrix4 view;
  GFC_Matrix4 model;
  GFC_Matrix4 proj;
  GFC_Vector4D color;
} ModelUBO;

typedef struct {

  int _refCount;
  Mesh *mesh;       // GPU HANDLE FOR MESH DATA: refer to gf3d_mesh
  Texture *texture; // texture mem
  GFC_TextLine filename;

} Model;

void model_system_init(Uint32 max_models);

void model_close();

Model *model_new();

void model_free(Model *model);

void model_delete(Model *model);

Model *model_get_by_filename(const char *filename);

void gf3d_model_reset_pipes();

void gf3d_model_submit_pipe_commands();

VkCommandBuffer gf3d_model_get_model_command_buffer();

void gf3d_model_queue_render(Model *model, GFC_Matrix4 mat, GFC_Color colorMod);

Model *model_load(const char *filename);

ModelUBO model_get_ubo(GFC_Matrix4 modelMat, GFC_Color colormod);

Pipeline *model_get_pipeline();

#endif
