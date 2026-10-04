#ifndef __MODEL_H__
#define __MODEL_H__



#include "simple_json.h"

#include "gfc_types.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_text.h"

#include "gf3d_pipeline.h"
#include "gf3d_texture.h"

typedef struct {

    GFC_Matrix4 view;
    GFC_Matrix4 model;
    GFC_Matrix4 proj;
    GFC_Vector4 color;
} ubo;

} ModelUBO;

typedef struct {


    int _refCount;
    Mesh *mesh;  // GPU HANDLE FOR MESH DATA: refer to gf3d_mesh
    Texture *texture; // texture mem
    GFC_TextLine filename;

} Model;

#endif
