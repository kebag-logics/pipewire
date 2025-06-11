#include "endstation_builder.h"
#include "aecp-aem.h"



int es_builder_entity_model(struct server *server, uint16_t type, 
            uint16_t index, size_t size, void *ptr)
{

    // To register are:
    // Unsollicited notifications + list of the lockings
    // Lock/ Unlock + entity id of the ctonroller locking
}


#define HELPER_ES_BUIDLER(type, callback) \
    [type] = { .build_descriptor_cb = callback  }

struct es_builder_st {
    int (*build_descriptor_cb) (struct server *server, uint16_t type, 
            uint16_t index, size_t size, void *ptr);
};

static struct es_bulder_st es_builder [] = 
{
    HELPER_ES_BUIDLER(AVB_AEM_DESC_ENTITY, )
};


int endstation_builder_add_descriptor(struct server *server, uint16_t type,
     uint16_t index, size_t size, void *ptr)
{
    // TODO
    // 1. create a builder for each of the different type of descriptor
    // 2. Then add it to the server
    // 3. For streams, make sure to get the information of the MRP on a per
    // stream basis.


}