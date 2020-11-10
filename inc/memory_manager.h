#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H
#include "memory_class.h"

#define DRAM_CHANNEL_WIDTH 8 // 8B
#define DRAM_WQ_SIZE 64
#define DRAM_RQ_SIZE 64
#define NUM_MEMS 2 // now hybrid: DRAM, NVM

// DRAM
class MEMORY_MANAGER : public MEMORY {
  public:
    MEMORY *lower_level_memory[NUM_MEMS];
    const string NAME;
    int fill_level;

    // queues
    PACKET_QUEUE WQ[DRAM_CHANNELS], RQ[DRAM_CHANNELS];
    PACKET_QUEUE DRAM_WQ[DRAM_CHANNELS], DRAM_RQ[DRAM_CHANNELS], NVM_WQ[DRAM_CHANNELS], NVM_RQ[DRAM_CHANNELS];

    // constructor
    MEMORY_MANAGER(string v1) : NAME (v1) {
        for (uint32_t i=0; i<DRAM_CHANNELS; i++) {
            WQ[i].NAME = "WQ" + to_string(i);
            WQ[i].SIZE = DRAM_WQ_SIZE;
            WQ[i].entry = new PACKET [DRAM_WQ_SIZE];

            DRAM_WQ[i].NAME = "DRAM_WQ" + to_string(i);
            DRAM_WQ[i].SIZE = DRAM_WQ_SIZE;
            DRAM_WQ[i].entry = new PACKET [DRAM_WQ_SIZE];

            NVM_WQ[i].NAME = "NVM_WQ" + to_string(i);
            NVM_WQ[i].SIZE = DRAM_WQ_SIZE;
            NVM_WQ[i].entry = new PACKET [DRAM_WQ_SIZE];

            //RQ[i].NAME = "DRAM_RQ" + to_string(i);
            RQ[i].NAME = "RQ" + to_string(i);
            RQ[i].SIZE = DRAM_RQ_SIZE;
            RQ[i].entry = new PACKET [DRAM_RQ_SIZE];

            DRAM_RQ[i].NAME = "DRAM_RQ" + to_string(i);
            DRAM_RQ[i].SIZE = DRAM_RQ_SIZE;
            DRAM_RQ[i].entry = new PACKET [DRAM_RQ_SIZE];

            NVM_RQ[i].NAME = "NVM_RQ" + to_string(i);
            NVM_RQ[i].SIZE = DRAM_RQ_SIZE;
            NVM_RQ[i].entry = new PACKET [DRAM_RQ_SIZE];
        }

        //fill_level = FILL_DRAM;
        fill_level = FILL_HMMU;
    };

    // my constructor 1
    //MEMORY_CONTROLLER()

    // destructor
    ~MEMORY_MANAGER() {

    };

    // functions
    int  add_rq(PACKET *packet),
         add_wq(PACKET *packet),
         add_pq(PACKET *packet);

    void return_data(PACKET *packet),
         operate(),
         increment_WQ_FULL(uint64_t address);

    uint32_t get_occupancy(uint8_t queue_type, uint64_t address),
             get_size(uint8_t queue_type, uint64_t address);

    uint32_t dram_get_channel(uint64_t address),
             dram_get_rank   (uint64_t address),
             dram_get_bank   (uint64_t address),
             dram_get_row    (uint64_t address),
             dram_get_column (uint64_t address),
             drc_check_hit (uint64_t address, uint32_t cpu, uint32_t channel, uint32_t rank, uint32_t bank, uint32_t row);

    int check_dram_queue(PACKET_QUEUE *queue, PACKET *packet);
};

#endif