#include "memory_manager.h" //

void MEMORY_MANAGER::operate()
{
    uint64_t addr;
    for (uint32_t i=0; i<DRAM_CHANNELS; i++){
        for (uint32_t j=0; j<DRAM_WQ_SIZE; j++){
            if (WQ[i].entry[j].address){
                addr = WQ[i].entry[j].address;
                if ((addr >> (ADDR_WIDTH-3)) == 0){
                    DRAM_WQ[i].add_queue(&WQ[i].entry[j]);
                }
                else {
                    NVM_WQ[i].add_queue(&WQ[i].entry[j]);
                }
            }
    }
        for (uint32_t j=0; j<DRAM_RQ_SIZE; j++){
            if (RQ[i].entry[j].address){
                addr = RQ[i].entry[j].address;
                if ((addr >> (ADDR_WIDTH-3)) == 0){
                    DRAM_RQ[i].add_queue(&RQ[i].entry[j]);
                }
                else {
                    NVM_RQ[i].add_queue(&RQ[i].entry[j]);
                }
            }
    }
    }
}

int MEMORY_MANAGER::add_rq(PACKET *packet)
{
    // simply return read requests with dummy response before the warmup
    if (all_warmup_complete < NUM_CPUS) {
        if (packet->instruction) 
            upper_level_icache[packet->cpu]->return_data(packet);
        if (packet->is_data)
            upper_level_dcache[packet->cpu]->return_data(packet);

        return -1;
    }

    // check for the latest wirtebacks in the write queue
    uint32_t channel = dram_get_channel(packet->address);
    int wq_index = check_dram_queue(&WQ[channel], packet);
    if (wq_index != -1) {
        
        // no need to check fill level
        //if (packet->fill_level < fill_level) {

            packet->data = WQ[channel].entry[wq_index].data;
            if (packet->instruction) 
                upper_level_icache[packet->cpu]->return_data(packet);
            if (packet->is_data) 
                upper_level_dcache[packet->cpu]->return_data(packet);
        //}

        DP ( if (packet->cpu) {
        cout << "[" << NAME << "_RQ] " << __func__ << " instr_id: " << packet->instr_id << " found recent writebacks";
        cout << hex << " read: " << packet->address << " writeback: " << WQ[channel].entry[wq_index].address << dec << endl; });

        ACCESS[1]++;
        HIT[1]++;

        WQ[channel].FORWARD++;
        RQ[channel].ACCESS++;
        //assert(0);

        return -1;
    }

    // check for duplicates in the read queue
    int index = check_dram_queue(&RQ[channel], packet);
    if (index != -1)
        return index; // merged index

    // search for the empty index
    for (index=0; index<DRAM_RQ_SIZE; index++) {
        if (RQ[channel].entry[index].address == 0) {
            
            RQ[channel].entry[index] = *packet;
            RQ[channel].occupancy++;

#ifdef DEBUG_PRINT
            uint32_t channel = dram_get_channel(packet->address),
                     rank = dram_get_rank(packet->address),
                     bank = dram_get_bank(packet->address),
                     row = dram_get_row(packet->address),
                     column = dram_get_column(packet->address); 
#endif

            DP ( if(warmup_complete[packet->cpu]) {
            cout << "[" << NAME << "_RQ] " <<  __func__ << " instr_id: " << packet->instr_id << " address: " << hex << packet->address;
            cout << " full_addr: " << packet->full_addr << dec << " ch: " << channel;
            cout << " rank: " << rank << " bank: " << bank << " row: " << row << " col: " << column;
            cout << " occupancy: " << RQ[channel].occupancy << " current: " << current_core_cycle[packet->cpu] << " event: " << packet->event_cycle << endl; });

            break;
        }
    }

    //update_schedule_cycle(&RQ[channel]);

    return -1;
}

int MEMORY_MANAGER::add_wq(PACKET *packet)
{
    // simply drop write requests before the warmup
    if (all_warmup_complete < NUM_CPUS)
        return -1;

    // check for duplicates in the write queue
    uint32_t channel = dram_get_channel(packet->address);
    int index = check_dram_queue(&WQ[channel], packet);
    if (index != -1)
        return index; // merged index

    // search for the empty index
    for (index=0; index<DRAM_WQ_SIZE; index++) {
        if (WQ[channel].entry[index].address == 0) {
            
            WQ[channel].entry[index] = *packet;
            WQ[channel].occupancy++;

#ifdef DEBUG_PRINT
            uint32_t channel = dram_get_channel(packet->address),
                     rank = dram_get_rank(packet->address),
                     bank = dram_get_bank(packet->address),
                     row = dram_get_row(packet->address),
                     column = dram_get_column(packet->address); 
#endif

            DP ( if(warmup_complete[packet->cpu]) {
            cout << "[" << NAME << "_WQ] " <<  __func__ << " instr_id: " << packet->instr_id << " address: " << hex << packet->address;
            cout << " full_addr: " << packet->full_addr << dec << " ch: " << channel;
            cout << " rank: " << rank << " bank: " << bank << " row: " << row << " col: " << column;
            cout << " occupancy: " << WQ[channel].occupancy << " current: " << current_core_cycle[packet->cpu] << " event: " << packet->event_cycle << endl; });

            break;
        }
    }

    //update_schedule_cycle(&WQ[channel]);

    return -1;
}

int MEMORY_MANAGER::add_pq(PACKET *packet)
{
    return -1;
}

void MEMORY_MANAGER::return_data(PACKET *packet)
{

}

int MEMORY_MANAGER::check_dram_queue(PACKET_QUEUE *queue, PACKET *packet)
{
    // search write queue
    for (uint32_t index=0; index<queue->SIZE; index++) {
        if (queue->entry[index].address == packet->address) {
            
            DP ( if (warmup_complete[packet->cpu]) {
            cout << "[" << queue->NAME << "] " << __func__ << " same entry instr_id: " << packet->instr_id << " prior_id: " << queue->entry[index].instr_id;
            cout << " address: " << hex << packet->address << " full_addr: " << packet->full_addr << dec << endl; });

            return index;
        }
    }

    DP ( if (warmup_complete[packet->cpu]) {
    cout << "[" << queue->NAME << "] " << __func__ << " new address: " << hex << packet->address;
    cout << " full_addr: " << packet->full_addr << dec << endl; });

    DP ( if (warmup_complete[packet->cpu] && (queue->occupancy == queue->SIZE)) {
    cout << "[" << queue->NAME << "] " << __func__ << " mshr is full";
    cout << " instr_id: " << packet->instr_id << " mshr occupancy: " << queue->occupancy;
    cout << " address: " << hex << packet->address;
    cout << " full_addr: " << packet->full_addr << dec;
    cout << " cycle: " << current_core_cycle[packet->cpu] << endl; });

    return -1;
}

uint32_t MEMORY_MANAGER::dram_get_channel(uint64_t address)
{
    if (LOG2_DRAM_CHANNELS == 0)
        return 0;

    int shift = 0;

    return (uint32_t) (address >> shift) & (DRAM_CHANNELS - 1);
}

uint32_t MEMORY_MANAGER::dram_get_bank(uint64_t address)
{
    if (LOG2_DRAM_BANKS == 0)
        return 0;

    int shift = LOG2_DRAM_CHANNELS;

    return (uint32_t) (address >> shift) & (DRAM_BANKS - 1);
}

uint32_t MEMORY_MANAGER::dram_get_column(uint64_t address)
{
    if (LOG2_DRAM_COLUMNS == 0)
        return 0;

    int shift = LOG2_DRAM_BANKS + LOG2_DRAM_CHANNELS;

    return (uint32_t) (address >> shift) & (DRAM_COLUMNS - 1);
}

uint32_t MEMORY_MANAGER::dram_get_rank(uint64_t address)
{
    if (LOG2_DRAM_RANKS == 0)
        return 0;

    int shift = LOG2_DRAM_COLUMNS + LOG2_DRAM_BANKS + LOG2_DRAM_CHANNELS;

    return (uint32_t) (address >> shift) & (DRAM_RANKS - 1);
}

uint32_t MEMORY_MANAGER::dram_get_row(uint64_t address)
{
    if (LOG2_DRAM_ROWS == 0)
        return 0;

    int shift = LOG2_DRAM_RANKS + LOG2_DRAM_COLUMNS + LOG2_DRAM_BANKS + LOG2_DRAM_CHANNELS;

    return (uint32_t) (address >> shift) & (DRAM_ROWS - 1);
}

uint32_t MEMORY_MANAGER::get_occupancy(uint8_t queue_type, uint64_t address)
{
    uint32_t channel = dram_get_channel(address);
    if (queue_type == 1)
        return RQ[channel].occupancy;
    else if (queue_type == 2)
        return WQ[channel].occupancy;

    return 0;
}

uint32_t MEMORY_MANAGER::get_size(uint8_t queue_type, uint64_t address)
{
    uint32_t channel = dram_get_channel(address);
    if (queue_type == 1)
        return RQ[channel].SIZE;
    else if (queue_type == 2)
        return WQ[channel].SIZE;

    return 0;
}
