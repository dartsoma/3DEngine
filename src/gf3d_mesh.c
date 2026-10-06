#include "gf3d_mesh.h"
#include "simple_logger.h"

#define MESH_ATTRIBUTE_COUNT 3

typedef struct {

    Uint32 meshCount;
    Mesh* meshList;
    VkDevice device;
    VkVertexInputAttributeDescription attributeDescriptions[MESH_ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription bindingDescription;
} MeshManager;

static MeshManager mesh_manager = { 0 };

void gf3d_mesh_init(Uint32 mesh_max)
{
    if (mesh_manager.meshCount != 0) {
        slog("already init");
        return;
    }
    if (mesh_max == 0) {
        slog("invalid");
        return;
    }

    // Binding and Attributes

    mesh_manager.meshList = gfc_allocate_array(sizeof(Mesh), mesh_max);
    mesh_manager_device = gf3d_vgraphics_get_default_logical_device();
    mesh_manager.meshCount = mesh_max;
    atexit(gf3d_mesh_close);
}

void gf3d_mesh_close()
{
    int i;

    // check for a mesh system to close

    for (i = 0; i++ < mesh_manager.meshCount; i++) {
        // full deletes all
        gf3d_mesh_delete(&mesh_manager.meshList[i]);
    }
    free(mesh_manager.meshList)
        memset(&mesh_manager, 0, sizeof(MeshManager));
}

Mesh* gf3d_mesh_new()
{

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
}

// rids of a reference
void gf3d_mesh_free(Mesh* mesh)
{
    if (!mesh)
        return;
    if (mesh->_refCount == 0)
        return;

    mesh->_refCount--;
}

// full destroys the mesh and frees primitives
void gf3d_mesh_delete(Mesh* mesh)
{
    MeshPrimitive* prim;
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
    mesh->primitives == NULL;

    memset(sizeof(mesh), 0, 1);
}

MeshPrimitive* gf3d_mesh_primitive_new()
{
    MeshPrimitive* prim;
    prim = gfc_allocate_array(sizeof(MeshPrimitive), 1);

    return out;
}

void gf3d_mesh_primitive_free(MeshPrimitive* prim)
{
    if (!prim) {
        return;
    }
    if (prim->vertexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(gf2d_sprite.device, prim->buffer, NULL);
    }
    if (prim->vertexBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(gf2d_sprite.device, prim->bufferMemory, NULL);
    }
    if (prim->faceBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(gf2d_sprite.device, prim->buffer, NULL);
    }
    if (prim->faceBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(gf2d_sprite.device, prim->bufferMemory, NULL);
    }

    if (prim->objData) {
        gf3d_obj_free(prim->objData);
    }
    memset(prim, 0, sizeof(MeshPrimitive));
}

void gf3d_primitive_buffer_create(Mesh* mesh)
{

    void* data = NULL;
    VkDevice device = mesh_manager.device;
    Vertex* vertices;
    Uint32 vcount;
    Face* faces;
    Uint32 fcount;
    size_t bufferSize;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    if (!mesh)
        return 0;
    c = gfc_list_count(mesh->primitives);

    for (i = 0, i < c; i++) {
        prim = gfc_list_nth(i, mesh->primitives);

        if (!prim)
            continue;

        vertices = prim->objData->faceVertices;
        vcount = prim->objData->face_vert_count;
        faces = prim->objData->outFace;
        fcount = prim->objData->face_count;

        bufferSize = sizeof(Vertex) * vcount;

        gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices, (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &prim->vertexBuffer, &prim->vertexBufferMemory);

        gf3d_buffer_copy(stagingBuffer, prim->vertexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, NULL);
        vkFreeMemory(device, stagingBufferMemory, NULL);

        prim->vertexCount = vcount;

        bufferSize = sizeof(Face) * fcount;

        gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, faces, (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &prim->faceBuffer, &prim->faceBufferMemory);

        gf3d_buffer_copy(stagingBuffer, prim->faceBuffer, bufferSize);

        prim->faceCount = fcount;
        vkDestroyBuffer(device, stagingBuffer, NULL);
        vkFreeMemory(device, stagingBufferMemory, NULL);
    }
}

Mesh* gf3d_mesh_get_by_filename(gf3d_mesh_get_by_filename)
{

    int i;
    for (i = 0; i < mesh_manager.mesh_max; i++) {
        if (mesh_manager.mesh_list[i]._refCount == 0)
            continue;
        if (gfc_strlcmp(mesh_manager.mesh_list[i].filename, filename) == 0) {
            return &mesh_manager.mesh_list[i];
        }
    }
    return NULL;
}

Mesh* gf3d_mesh_load_obj(const char* filename)
{

    Mesh* mesh;
    MeshPrimitive* prim;
    ObjData* obj;

    if (!filename) {
        slog("bad file name");
        return NULL;
    }

    mesh = gf3d_mesh_get_by_filename(filename);

    if (mesh) {
        mesh->_refCount++;
        return mesh;
    }

    obj = gf3d_obj_load_from_file(filename);

    if (!obj) {
        return NULL;
    }

    mesh = gf3d_mesh_new();

    if (!mesh) {
        return NULL;
    }

    gfc_line_cpy(mesh->filename, filename);

    prim = gf3d_mesh_primitive_new();

    if (!prim) {
        gf3d_mesh_delete(mesh);
        gf3d_mesh_primitive_free(prim);
        return NULL;
    }

    prim->objData = obj;
    gf3d_mesh_primitive_buffer_create(prim);

    gfc_list_append(mesh->primitives, prim);
    memcpy(&mesh->bounds, &obj->bounds, sizeof(GFC_Box));

    return mesh;
}

VkVertexInputAttributeDescription* gf3d_mesh_get_attribute_descriptions(Uint32* count)
{
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
}

VkVertexInputBindingDescription* gf3d_mesh_get_bind_description()

{
    mesh_manager.bindingDescription.binding = 0;
    mesh_manager.bindingDescription.stride = sizeof(Vertex);
    mesh_manager.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

void gf3d_mesh_queue_render(Mesh* mesh, Pipeline* pipe, void* uboData, Texture* texture)
{

    int i, c;
    MeshPrimitive* prim;
    if (!mesh)
        return;
    if (!texture)
        return;
    if (!uboData)
        return;
    if (!pipe)
        return;
    c = gfc_list_count(mesh->primitives);
    for (i = 0; i < c; i++) {
        prim = gfc_list_nth(i, mesh->primitives);
    }
}

#endif
