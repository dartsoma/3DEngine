#ifndef __GF3D_MESH_H__
#define __GF3D_MESH_H__

#include <stdalign.h>
#include <vulkan/vulkan.h>

#include "gf3d_pipeline.h"
#include "gfc_list.h"
#include "gfc_matrix.h"
#include "gfc_primitives.h"
#include "gfc_text.h"
#include "gfc_vector.h"

// forward declaration:
typedef struct ObjData_S ObjData;

// absolute basics of the mesh information sent to the graphics card
typedef struct {
  GFC_Matrix4 model;
  GFC_Matrix4 view;
  GFC_Matrix4 proj;
  GFC_Vector4D color;
  GFC_Vector4D camera;
} MeshUBO;

typedef struct {
  GFC_Vector3D vertex;
  GFC_Vector3D normal;
  GFC_Vector2D texel;
} Vertex;

typedef struct {
  Uint16 verts[3];
} Face;

typedef struct {
  Uint32 vertexCount;
  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  Uint32 faceCount;
  VkBuffer faceBuffer;
  VkDeviceMemory faceBufferMemory;
  ObjData *objData;
} MeshPrimitive;

typedef struct {
  GFC_TextLine filename;
  Uint32 _refCount;
  GFC_List *primitives;
  GFC_Box bounds;
} Mesh;

void gf3d_mesh_init(Uint32 mesh_max);

void gf3d_mesh_close();

Mesh *gf3d_mesh_new();

// rids of a reference
void gf3d_mesh_free(Mesh *mesh);

void gf3d_mesh_delete(Mesh *mesh);

MeshPrimitive *gf3d_mesh_primitive_new();

void gf3d_mesh_primitive_free(MeshPrimitive *prim);

Uint8 gf3d_primitive_buffer_create(MeshPrimitive *prim);

Mesh *gf3d_mesh_load_obj(const char *filename);

VkVertexInputAttributeDescription *gf3d_mesh_get_attribute_descriptions();
VkVertexInputBindingDescription *gf3d_mesh_get_bind_description();

void gf3d_mesh_queue_render(Mesh *mesh, Pipeline *pipe, void *uboData,
                            Texture *texture);

#endif
