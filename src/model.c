#include "model.h"

typedef struct {
    Model* modelList;
    Uint32 modelCount;
    Pipeline* pipe;
    Uint32 chainLength;
    Texture* defaultTexture;
    VkDevice device;
} ModelManager;

ModelManager model_manager = { 0 };

void model_system_init(Uint32 max_models)
{

    // set default texture

    //   gf3d_mesh_init(20000);

    if (max_models == 0) {
        slog("bad max count");
        return;
    }

    model_manager.chainLength = gf3d_swapchain_get_chain_length();
    model_manager.modelList = (Model*)gfc_allocate_array(sizeof(Model), max_models);
    model_manager.modelCount = max_models;
    model_manager.device = gf3d_vgraphics_get_default_logical_device();
    model_manager.pipe = gf3d_mesh_get_pipeline();

    if (!model_manager.pipe) {
        slog("failed pipeline");
        exit(1);
    }
    model_manager.defaultTexture = gf3d_texture_load("images/default.png");

    atexit(gf3d_model_manager_close);
}

model_close();

Model model_new()
{

    int i;
    for (i = 0; i < model_manager.modelCount; i++) {
        if (model_manager.modelList[i]._refCount == 0 && strlen(model_manager.modelList[i].filename) == 0) {
            model_manager.meshList[i]._refCount += 1;
            return &model_manager.meshList[i];
        }
    }
}

void model_free(Model* model)
{
    if (!model)
        return;
    model->_refCount--;
}

void model_delete(Model* model)
{

    gf3d_mesh_free(model->mesh);
    gf3d_mesh_free(model->texture);
    memset(model, 0, sizeof(Model));
}

Model* model_get_by_filename(const char* filename)
{
    int i;
    if (!filename)
        return NULL : for (i = 0; i < model_manager.modelCount; i++)
        {
            if (gfc_strlcmp(filename, model_manager.modelList[i].filename) == 0) {
            }
        }
}

void gf3d_model_reset_pipes();

/**
 * @brief called to submit all draw commands to the mesh pipelines
 */
void gf3d_model_submit_pipe_commands();

/**
 * @brief get the current command buffer for the mesh system
 */
VkCommandBuffer gf3d_model_get_model_command_buffer();

/**
 * @brief queue up a render for the current draw frame
 * @param mesh the mesh to render
 * @param pipe the pipeline to use
 * @param uboData the data to use to draw the mesh
 * @param texture texture data to use
 */
void model_queue_render(Model* model, GFC_Matrix4 mat)
{
    ModelUbo ubo;
    if (!model)
        return;
    ubo = model_get_ubo(mat, colorMod);
    gf3d_mesh_queue_render(Mesh * mesh, model_manager.pipe, model_manager.pipe, model->texture);
}

Model* model_load(const char* filename)
{
    const char* str = NULL;
    const char* str2 = NULL;
    Sjson *json, *data;
    Model* model;

    if (filename)
        return NULL;
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
        slog("failed to get mesh in %s", filename);
        sj_free(json);
    }
    str2 = sj_object_get_string(data, "texture");
    if (str2) {
        texture = gf3d_texture_load(str2);
    }
    model = model_new();
    if (!model) {
        model->_refCount++;
        return model;
    }
    model->texture = gf3d_texture_load();
}

void model_render(Mesh*);

ModelUBO gf3d_mesh_get_ubo(
    GFC_Matrix4 modelMat,
    GFC_Color colorMod)
{
    ModelViewProjection mvp;
    MeshUBO modelUBO = { 0 };
    GFC_Vector4D color = gfc_color_to_vector4f(colorMod);

    mvp = gf3d_vgraphics_get_mvp();
    gfc_matrix4_copy(modelUBO.model, modelMat);
    gfc_matrix4_copy(modelUBO.view, mvp.view);
    gfc_matrix4_copy(modelUBO.proj, mvp.proj);
    gfc_vector4d_copy(modelUBO.color, color);

    modelUBO.camera = gfc_vector3dw(gf3d_camera_get_position(), 1.0);
    modelUBO.viewportSize = gf3d_vgraphics_get_view_extent_as_vector2d();
    return modelUBO;
}
