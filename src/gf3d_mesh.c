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

void gf3d_mesh_init(Uint32 mesh_max){
    if (mesh_manager.meshCount !=0 ){
        slog("already init");
        return;
    }
    if(mesh_max==0){
        slog("invalid");
        return;
    }
    mesh_manager.meshList = gfc_allocate_array(sizeof(Mesh), mesh_max);
    mesh_manager_device = gf3d_vgraphics_get_default_logical_device();
    mesh_manager.meshCount = mesh_max;
    atexit(gf3d_mesh_close);
}

void gf3d_mesh_close(){
    int i;

    for(i = 0; i++ < mesh_manager.meshCount; i++){
        gf3d_mesh_delete(&mesh_manager.meshList[i]);
    }
    free(mesh_manager.meshList)
    memset(&mesh_manager,0,sizeof(MeshManager));

}


void gf3d_mesh_free(Mesh *mesh)
{
    if (!mesh)return;
    mesh->_refCount--;
}


/**
 * @brief get a new empty model
 * @return NULL on error, or an empty model
 */
Mesh *gf3d_mesh_new(){

    int i;
    for (i=0; i < mesh_manager.meshCount; i++)
    {
        if(mesh_manager.meshList[i]._refCount == 0){
            mesh_manager.meshList[i]._refCount = 1;
            mesh_manager.meshList[i].primitives = gfc_list_new();
            if (mesh_manager.meshList[i].primitives == NULL){
                mesh_manager.meshList[i]._refCount = 0;
                slog("can't allocate more memory for mesh");
                return NULL;
            }
            mesh_manager.meshList[i]._refCount+= 1;
            return &mesh_manager.meshList[i];
        }
    }
}

void gf3d_mesh_delete(Mesh *mesh){

    if (!mesh){
        return;
    }
    MeshPrimitive *prim;
    int i, c;

    c = gfc_list_count(mesh->primitives);
    for (i=0; i < c; i++){
        prim = gfc_list_nth(mesh->primitives, i);
        if(!prim)continue;
        gf3d_mesh_primitive_free(prim);
    }
    gfc_list_delete(mesh->primitives);
    free(mesh);

}

gf3d_mesh_obj_buffer_create(){
    int i, c;
    MeshPrimitive *prim;
    if (!mesh) return 0;
    prim =  gf3d_mesh_primitive_new();
    if (!prim){
        slog("failed");
    }
}

void gf3d_primitive_buffer_create(){

    int i,c;
    MeshPrimitive *prim;

    if(!mesh)return 0;
    c = gfc_list_nth(mesh->primitives, 1);
    for (i=0,i<c;i++){
        if (!prim) continue;
    }

}

void gf3d_mesh_buffer_create(Mesh *mesh){

// face buffer

    // vertex buffer

}



void gf3d_mesh_primitive_free(MeshPrimitives *prim){
    if (!prim)  {
        return;
    }
    if (prim->vertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(gf2d_sprite.device, prim->buffer, NULL);
    }
    if (prim->vertexBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gf2d_sprite.device, prim->bufferMemory, NULL);
    }
    if (prim->faceBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(gf2d_sprite.device, prim->buffer, NULL);
    }
    if (prim->vertexBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gf2d_sprite.device, prim->bufferMemory, NULL);
    }

    if (prim->objData){
        gf3d_obj_free(prim->objData);
    }
    memset(prim,0,sizeof(MeshPrimitive));
}


Mesh *gf3d_mesh_get_by_filename(gf3d_mesh_get_by_filename){

    int i;
    for (i = 0; i < mesh_manager.mesh_max; i++)
    {
        if (mesh_manager.mesh_list[i]._refCount == 0)continue;
        if (gfc_strlcmp(mesh_manager.mesh_list[i].filename,filename) == 0)
        {
            return &mesh_manager.mesh_list[i];
        }
    }
    return NULL;

}

Mesh *gf3d_mesh_load_obj(const char *filename){

    Mesh *mesh;
    MeshPrimitive *prim;


    ObjData *obj;
    if(!filename) {
        slog("bad file name");
        return NULL;
    }

    mesh = gf3d_mesh_get_by_filename(filename);

    if(mesh){
       mesh->_refCount++;
       return mesh;
    }

    obj = gf3d_obj_load_from_file(filename);

    if (!obj)
    {
        return NULL;
    }

    mesh = gf3d_mesh_new();
    if (!mesh)
    {
        return NULL;
    }
    gfc_line_cpy(mesh->filename,filename);

    prim = gf3d_mesh_primitive_new();
    prim->objData = obj;
    gf3d_mesh_create_vertex_buffer_from_vertices(primitive);

    gfc_list_append(mesh->primitives,primitive);
    memcpy(&mesh->bounds,&obj->bounds,sizeof(GFC_Box)); // FIX

    return mesh;


}

/**
 * @brief make an exact, but separate copy of the input mesh
 * @param in the mesh to duplicate
 * @return NULL on error, or a copy of in
 */
Mesh *gf3d_mesh_copy(Mesh *in){

    Mesh *mesh;
    MeshPrimative *prim;
    ObjData *obj;



}

/**
 * @brief move all of the vertices of the mesh by offset at the buffer level
 * @param in the mesh to move
 * @param offset how much to move it
 * @param rotation apply this rotation to the vertices and normals
 */

void gf3d_mesh_move_vertices(Mesh *in, GFC_Vector3D offset,GFC_Vector3D rotation){

    int i,c;
    MeshPrimitive *prim;
    if (!in)return;
    c = gfc_list_get_count(in->prim);
    for (i = 0; i < c;i++)
    {
        prim = gfc_list_get_nth(in->primitives,i);
        if (!prim)continue;
        if (!in->objData)return;
        gf3d_obj_move(in->objData,offset,rotation);
        gf3d_mesh_primitive_delete_buffers(in);
        gf3d_mesh_create_vertex_buffer_from_vertices(in);
    }

}

/**
 * @brief allocate a zero initialized mesh primitive
 * @return NULL on error or the primitive
 */
MeshPrimitive *gf3d_mesh_primitive_new(){
    MeshPrimitive *out = NULL;
    out = gfc_allocate_array(sizeof(MeshPrimitive),1);

    return out;
}


/**
 * @brief get the input attribute descriptions for mesh based rendering
 * @param count (optional, output) the number of attributes
 * @return a pointer to a vertex input attribute description array
 */
VkVertexInputAttributeDescription * gf3d_mesh_get_attribute_descriptions(Uint32 *count);

/**
 * @brief get the binding description for mesh based rendering
 * @return vertex input binding descriptions compatible with mesh data
 */
VkVertexInputBindingDescription * gf3d_mesh_get_bind_description();

/**
 * @brief free a mesh that has been loaded from memory
 */

/**
 * @brief needs to be called once at the beginning of each render frame
 */


/**
 * @brief called to submit all draw commands to the mesh pipelines
 */

/**
 * @brief get the current command buffer for the mesh system
 */
VkCommandBuffer gf3d_mesh_get_model_command_buffer(){
    if (!mesh_manager.pipe)return VK_NULL_HANDLE;
    return mesh_manager.pipe->commandBuffer;
}


/**
 * @brief queue up a render for the current draw frame
 * @param mesh the mesh to render
 * @param pipe the pipeline to use
 * @param uboData the data to use to draw the mesh
 * @param texture texture data to use
 */
void gf3d_mesh_queue_render(Mesh *mesh,Pipeline *pipe,void *uboData,Texture *texture){


    int i,c;
    MeshPrimative *prim;
    if(!mesh) return;
    if(!texture) return;
    if(!uboData) return;
    if(!pipe) return;
    c = gfc_list_count(mesh->primitives);
    for (i=0; i<c; i++){
        prim = gfc_list_nth(i, mesh->primitives);

    }
}


/**
 * @brief adds a mesh to the render pass rendered as an outline highlight
 * @note: must be called within the render pass
 * @param mesh the mesh to render
 * @param com the command pool to use to handle the request we are rendering with
 */
void gf3d_mesh_render(Mesh *mesh,VkCommandBuffer commandBuffer, VkDescriptorSet * descriptorSet){

   gf3d_mesh_render_generic(mesh,mesh_manager.pipe, descriptorSet);
}

/**
 * @brief render a mesh through a given pipeline
 */
void gf3d_mesh_render_generic(Mesh *mesh,Pipeline *pipe,VkDescriptorSet * descriptorSet){

    int i,c;
    MeshPrimitive *prim;
    if (!mesh)
    {
        slog("cannot render a NULL mesh");
        return;
    }
    if (!pipe)
    {
        slog("cannot render with NULL pipe");
        return;
    }
    if (!descriptorSet)
    {
        slog("cannot render with NULL descriptor set");
        return;
    }
    c = gfc_list_get_count(mesh->primitives);
    for (i = 0; i < c; i++)
    {
        prim = gfc_list_get_nth(mesh->primitives,i);
        if (!prim)continue;
        gf3d_pipeline_call_render(
            pipe,
            descriptorSet,
            prim->vertexBuffer,
            prim->faceCount * 3,
            prim->faceBuffer);
    }


}




/**
 * @brief create a mesh's internal buffers based on vertices
 * @param primitive the mesh primitive to populate
 * @note the primitive must have the objData set and it must have be organizes in buffer order
 */
void gf3d_mesh_create_vertex_buffer_from_vertices(MeshPrimitive *primitive){

    void *data = NULL;
    VkDevice device = gf3d_vgraphics_get_default_logical_device();
    Vertex *vertices;
    Uint32 vcount;
    Face *faces;
    Uint32 fcount;
    size_t bufferSize;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    if (!mesh)
    {
        slog("no mesh primitive provided");
        return;
    }

    vertices = mesh->objData->faceVertices;
    vcount = mesh->objData->face_vert_count;
    faces = mesh->objData->outFace;
    fcount = mesh->objData->face_count;
    bufferSize = sizeof(Vertex) * vcount;


    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mesh->vertexBuffer, &mesh->vertexBufferMemory);

    gf3d_buffer_copy(stagingBuffer, mesh->vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);

    mesh->vertexCount = vcount;

    gf3d_mesh_setup_face_buffers(mesh,faces,fcount);

}

/**
 * @brief get the pipeline that is used to render basic 3d meshes
 * @return NULL on error or the pipeline in question
 */
Pipeline *gf3d_mesh_get_pipeline() {

   return mesh_manager.pipe;

}

/**
 * @brief given a model matrix and basic color, build the meshUBO needed to render a model
 * @param modelMat the model Matrix
 * @param colorMod the color for the UBO
 */


#endif
