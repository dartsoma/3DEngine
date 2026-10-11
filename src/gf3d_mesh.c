
#include "gf3d_mesh.h"

#include "gf3d_buffers.h"
#include "gf3d_obj_load.h"
#include "gf3d_vgraphics.h"
#include "simple_logger.h"

#define MESH_ATTRIBUTE_COUNT 3

typedef struct {

  Uint32 meshCount;
  Mesh *meshList;
  VkDevice device;
  VkVertexInputAttributeDescription attributeDescriptions[MESH_ATTRIBUTE_COUNT];
  VkVertexInputBindingDescription bindingDescription;
} MeshManager;

static MeshManager mesh_manager = {0};

Mesh *gf3d_mesh_get_by_filename(const char *filename);

void gf3d_mesh_init(Uint32 mesh_max) {
  if (mesh_manager.meshCount != 0) {
    slog("already init");
    return;
  }
  if (mesh_max == 0) {
    slog("invalid");
    return;
  }

  // Binding and Attributes
  mesh_manager.attributeDescriptions[0].binding = 0;
  mesh_manager.attributeDescriptions[0].location = 0;
  mesh_manager.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  mesh_manager.attributeDescriptions[0].offset = offsetof(Vertex, vertex);

  mesh_manager.attributeDescriptions[1].binding = 0;
  mesh_manager.attributeDescriptions[1].location = 1;
  mesh_manager.attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  mesh_manager.attributeDescriptions[1].offset = offsetof(Vertex, normal);

  mesh_manager.attributeDescriptions[2].binding = 0;
  mesh_manager.attributeDescriptions[2].location = 2;
  mesh_manager.attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
  mesh_manager.attributeDescriptions[2].offset = offsetof(Vertex, texel);

  mesh_manager.bindingDescription.binding = 0;
  mesh_manager.bindingDescription.stride = sizeof(Vertex);
  mesh_manager.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  mesh_manager.meshList = gfc_allocate_array(sizeof(Mesh), mesh_max);
  mesh_manager.device = gf3d_vgraphics_get_default_logical_device();
  mesh_manager.meshCount = mesh_max;
  atexit(gf3d_mesh_close);
}

void gf3d_mesh_close() {
  int i;

  // check for a mesh system to close

  for (i = 0; i < mesh_manager.meshCount; i++) {
    // full deletes all
    gf3d_mesh_delete(&mesh_manager.meshList[i]);
  }
  free(mesh_manager.meshList);
  memset(&mesh_manager, 0, sizeof(MeshManager));
}

Mesh *gf3d_mesh_new() {

  int i;
  for (i = 0; i < mesh_manager.meshCount; i++) {
    if (mesh_manager.meshList[i]._refCount == 0) {
      mesh_manager.meshList[i]._refCount = 1;
      mesh_manager.meshList[i].primitives = gfc_list_new();
      if (mesh_manager.meshList[i].primitives == NULL) {
        mesh_manager.meshList[i]._refCount = 0;
        slog("can't allocate more memory for mesh");
        return NULL;
      }
      mesh_manager.meshList[i]._refCount = 1;
      return &mesh_manager.meshList[i];
    }
  }
  return NULL;
}

// rids of a reference
void gf3d_mesh_free(Mesh *mesh) {
  if (!mesh)
    return;
  if (mesh->_refCount == 0)
    return;

  mesh->_refCount--;
}

// full destroys the mesh and frees primitives
void gf3d_mesh_delete(Mesh *mesh) {
  MeshPrimitive *prim;
  int i, c;

  if (!mesh) {
    return;
  }

  c = gfc_list_count(mesh->primitives);

  for (i = 0; i < c; i++) {
    prim = gfc_list_nth(mesh->primitives, i);
    if (!prim)
      continue;
    gf3d_mesh_primitive_free(prim);
  }
  gfc_list_delete(mesh->primitives);
  mesh->primitives = NULL;

  memset(mesh, 0, sizeof(Mesh));
}

MeshPrimitive *gf3d_mesh_primitive_new() {
  MeshPrimitive *prim;
  prim = gfc_allocate_array(sizeof(MeshPrimitive), 1);

  return prim;
}

void gf3d_mesh_primitive_free(MeshPrimitive *prim) {
  if (!prim) {
    return;
  }
  if (prim->vertexBuffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(mesh_manager.device, prim->vertexBuffer, NULL);
  }
  if (prim->vertexBufferMemory != VK_NULL_HANDLE) {
    vkFreeMemory(mesh_manager.device, prim->vertexBufferMemory, NULL);
  }
  if (prim->faceBuffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(mesh_manager.device, prim->faceBuffer, NULL);
  }
  if (prim->faceBufferMemory != VK_NULL_HANDLE) {
    vkFreeMemory(mesh_manager.device, prim->faceBufferMemory, NULL);
  }

  if (prim->objData) {
    gf3d_obj_free(prim->objData);
  }
  memset(prim, 0, sizeof(MeshPrimitive));
}

Uint8 gf3d_primitive_buffer_create(MeshPrimitive *prim) {

  Uint32 bufferSize = 0;
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  VkDevice device = mesh_manager.device;
  void *data = NULL;

  if (!prim) {
    slog("FAILED TO GIVE PRIM BUFFER CREATE A PRIM YOU MONSTER!");
    return 0;
  }

  if (!prim->objData) {
    slog("FAILED TO GIVE ME A PRIM WITH OBJDATA!");
    return 0;
  }

  if (!prim->objData->face_count) {
    slog("FAILED TO GIVE ME A PRIM WITH OBJ DATA WITH A FACE COUNT!");
    return 0;
  }

  bufferSize = (sizeof(Face) * prim->objData->face_count);
  gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     &stagingBuffer, &stagingBufferMemory);

  vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data) !=
      VK_SUCCESS;
  memcpy(data, prim->objData->outFace, (size_t)bufferSize);
  vkUnmapMemory(device, stagingBufferMemory);

  gf3d_buffer_create(bufferSize,
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                         VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &prim->faceBuffer,
                     &prim->faceBufferMemory);

  gf3d_buffer_copy(stagingBuffer, prim->faceBuffer, bufferSize);

  vkDestroyBuffer(device, stagingBuffer, NULL);
  vkFreeMemory(device, stagingBufferMemory, NULL);

  // Vertex BUFFERS
  bufferSize = (sizeof(Vertex) * prim->objData->face_vert_count);
  gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     &stagingBuffer, &stagingBufferMemory);

  vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, prim->objData->faceVertices, (size_t)bufferSize);
  vkUnmapMemory(device, stagingBufferMemory);

  gf3d_buffer_create(bufferSize,
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &prim->vertexBuffer,
                     &prim->vertexBufferMemory);

  gf3d_buffer_copy(stagingBuffer, prim->vertexBuffer, bufferSize);

  vkDestroyBuffer(device, stagingBuffer, NULL);
  vkFreeMemory(device, stagingBufferMemory, NULL);

  prim->vertexCount = prim->objData->face_vert_count;
  prim->faceCount = prim->objData->face_count;
  slog("vertex buffer: %p", prim->vertexBuffer);
  slog("index buffer: %p", prim->faceBuffer);
  slog("vertices: %u", prim->vertexCount);
  slog("faces: %u", prim->faceCount);
  slog("face count: %u", prim->objData->face_count);
  slog("face vert count: %u", prim->objData->face_vert_count);

  return 1;
}

Mesh *gf3d_mesh_get_by_filename(const char *filename) {

  int i;
  for (i = 0; i < mesh_manager.meshCount; i++) {
    if (mesh_manager.meshList[i]._refCount == 0)
      continue;
    if (gfc_strlcmp(mesh_manager.meshList[i].filename, filename) == 0) {
      return &mesh_manager.meshList[i];
    }
  }
  return NULL;
}

Mesh *gf3d_mesh_load_obj(const char *filename) {
  Mesh *mesh;
  MeshPrimitive *prim;
  ObjData *data;

  if (!filename)
    return NULL;

  mesh = gf3d_mesh_get_by_filename(filename);

  if (mesh) {
    slog("Found a mesh by filename!");
    mesh->_refCount++;
    return mesh;
  }

  mesh = gf3d_mesh_new();

  if (!mesh) {
    slog("Failed to allocate a new mesh. Return NULL!");
    return NULL;
  }

  slog("Mesh was created!");

  prim = gf3d_mesh_primitive_new();

  if (!prim) {
    slog("Failed to load mesh primitive! For filename: %f", filename);
    gf3d_mesh_delete(mesh);
    return NULL;
  }
  // mesh->primitives = gfc_list_new();

  slog("Mesh Prim created!");

  data = gf3d_obj_load_from_file(filename);
  if (!data) {
    gf3d_mesh_delete(mesh);
    gf3d_mesh_primitive_free(prim);
    slog("Failed to allocate objData for mesh!");
    return NULL;
  }

  slog("Data prim created!");

  prim->objData = data;

  if (!gf3d_primitive_buffer_create(prim)) {
    gf3d_mesh_delete(mesh);
    gf3d_mesh_primitive_free(prim);
    slog("Failed to make prim buffer data %s", filename);
    return NULL;
  }

  slog("Prim buffers created!");

  gfc_list_append(mesh->primitives, prim);
  // If you want a have multiple primitives do it here
  gfc_line_cpy(mesh->filename, filename);

  return mesh;
}

VkVertexInputAttributeDescription *gf3d_mesh_get_attribute_descriptions() {

  return mesh_manager.attributeDescriptions;
}

VkVertexInputBindingDescription *gf3d_mesh_get_bind_description()

{
  return &mesh_manager.bindingDescription;
}

void gf3d_mesh_queue_render(Mesh *mesh, Pipeline *pipe, void *uboData,
                            Texture *texture) {
  int c, i;
  MeshPrimitive *prim;

  if (!mesh || !pipe || !uboData || !texture) {
    slog("Something wrong in mesh queue renderer");
    return;
  }

  c = gfc_list_count(mesh->primitives);

  for (i = 0; i < c; i++) {
    prim = gfc_list_nth(mesh->primitives, i);
    if (!prim)
      continue;
    gf3d_pipeline_queue_render(pipe, prim->vertexBuffer, prim->vertexCount,
                               prim->faceBuffer, uboData, texture);
  }
}
