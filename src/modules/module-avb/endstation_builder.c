#include "endstation_builder.h"
#include "aecp-aem.h"


// Milan v1.2 5.3.4.1 ENTITY Lock state and
// Milan v1.2 5.3.4.2 List of registered controllers
int es_builder_desc_entity(struct server *server, uint16_t type,
            uint16_t index, size_t size, void *ptr)
{

    // To register are:
    // Unsollicited notifications + list of the lockings
    // Lock/ Unlock + entity id of the ctonroller locking
    pw_log_error("Not Implemented\n")
    spa_assert(0);

    return -1;
}

// Milan v1.2 5.3.5.1 Sampling rate
int es_builder_desc_audio_unit(struct server *server, uint16_t type,
            uint16_t index, size_t size, void *ptr)
{
    pw_log_error("Not Implemented\n")
    spa_assert(0);

    return -1;
}

// Milan v1.2 5.3.7 STREAM_OUTPUT
int es_builder_desc_stream_output(struct server *server, uint16_t type,
            uint16_t index, size_t size, void *ptr)
{
    // Milan v1.2 Clause 5.3.7.2 set up structure for the SR: status
    // Milan v1.2 Clause 5.3.7.3 streaming status

    // Milan v1.2 Clause 5.3.7.4 SRP params

    // Milan v1.2 Clause 5.3.7.5 STREAMID, Stream Destination Address
    // Stream VLAN and PCP

    // Milan v1.2 Clause 5.3.7.6 PTO

    // Milan v1.2 Clause 5.3.7.7 Diagnostic conunters:
    // Stream start / Stream Stop / Media Reset / Timestamp_Uncertain 
    // Frames TX




    pw_log_error("Not Implemented\n")
    spa_assert(0);

    return -1;
}

// Milan v1.2 5.3.8 STREAM_INPUT
int es_builder_desc_stream_input(struct server *server, uint16_t type,
            uint16_t index, size_t size, void *ptr)
{
    // Milan v1.2 Clause 5.3.8.2 Bound State

    // Milan v1.2 Clause 5.3.8.3 Binding Parameeters:
    // Entity Id of the bound talker
    // Index of the source the stream input is bound to
    // Entity Id of the controller that requested the binding
    // Start/Stop state

    // Milan v1.2 Clause 5.3.8.4 Talker discover state

    // Milan v1.2 Clause 5.3.8.5 Probing / Settled state

    // Milan v1.2 Clause 5.3.8.6 Probing status and ACMP status
    // PROBING_DISABLED / PROBING_PASSIVE / PROBING_ACTIVE
    // PROBING COMPLETED

    // Milan v1.2 Clause 5.3.8.7 Started/ Stopped state

    // Milan v1.2 Clause 5.3.8.8 SRP state

    // Milan v1.2 Clause 5.3.8.9 SRP parameters
    // VLAN_ID / Stream dest MAC addr /  STREAM ID

    // Milan v1.2 Clause 5.3.8.10 Diagnostic Counter
    // Media Lock / Media Unlock / Stream interrupted / SEQ_NUM_Mismatch
    // Media reset / Timestamp Uncertain / Unspported format / Late Timestamp
    // Early Timestamp / Frames RX


    pw_log_error("Not Implemented\n")
    spa_assert(0);

    return -1;
}

// Milan v1.2 5.3.9 STREAM_OUTPUT_PORT
int es_builder_desc_stream_port_output(struct server *server, uint16_t type,
            uint16_t index, size_t size, void *ptr)
{
    // Milan V1.2 Clause 5.3.9.1 Channel mapping

    pw_log_error("Not Implemented\n")
    spa_assert(0);

    return -1;

}

// Milan v1.2 5.3.10 STREAM_INPUT_PORT
int es_builder_desc_stream_port_input(struct server *server, uint16_t type,
            uint16_t index, size_t size, void *ptr)
{
    // Milan V1.2 Clause 5.3.10.1 Channel mapping

    pw_log_error("Not Implemented\n")
    spa_assert(0);

    return -1;

}


// Milan v1.2 5.3.11 CLOCK_DOMAIN
int es_builder_desc_clock_domain(struct server *server, uint16_t type,
            uint16_t index, size_t size, void *ptr)
{
    // Milan V1.2 Clause 5.3.11.1 Clocks source
    // Milan V1.2 Clause 5.3.11.2 Diagnostic counters: Lock / Unlocked.
}

// Milan v1.2 5.3.12 Identify
int es_builder_desc_identify(struct server *server, uint16_t type,
            uint16_t index, size_t size, void *ptr)
{

    pw_log_error("Not Implemented\n")
    spa_assert(0);

    return -1;
}

int es_builder_desc_avb_interface(struct server *server, uint16_t type,
            uint16_t index, size_t size, void *ptr)
{
    // TOOD get the dynamic informations:
    // * LINK_UP
    // * LINK_DOWN
    // * GTPT_CHANGED
    // * FRAME TX
    // * FRAME  RX
    // * RX CRC ERROR

    pw_log_error("Not Implemented\n")
    spa_assert(0);

    return -1;
}


#define HELPER_ES_BUIDLER(type, callback) \
    [type] = { .build_descriptor_cb = callback  }

struct es_builder_st {
    int (*build_descriptor_cb) (struct server *server, uint16_t type, 
            uint16_t index, size_t size, void *ptr);
};

static struct es_bulder_st es_builder [] =
{
    HELPER_ES_BUIDLER(AVB_AEM_DESC_ENTITY, es_builder_desc_entity),
    HELPER_ES_BUIDLER(AVB_AEM_DESC_AVB_INTERFACE, es_builder_desc_avb_interface),
    HELPER_ES_BUIDLER(AVB_AEM_DESC_STREAM_OUTPUT, es_builder_desc_stream_outtrainput),
    HELPER_ES_BUIDLER(AVB_AEM_DESC_STREAM_INPUT, es_builder_desc_stream_input),

    HELPER_ES_BUIDLER(AVB_AEM_DESC_STREAM_PORT_OUTPUT,
         es_builder_desc_stream_port_output),

    HELPER_ES_BUIDLER(AVB_AEM_DESC_STREAM_PORT_INPUT,
        es_builder_desc_stream_port_input),


    HELPER_ES_BUIDLER(AVB_AEM_DESC_CLOCK_DOMAIN,
        es_builder_desc_clock_domain),

    HELPER_ES_BUIDLER(AVB_AEM_DESC_CONTROL, es_builder_desc_identify),
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