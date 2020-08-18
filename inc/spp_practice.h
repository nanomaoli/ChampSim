#ifndef SPP_H
#define SPP_H

// SPP functional knobs
#define LOOKAHEAD_ON
#define FILTER_ON
#define GHR_ON
#define SPP_SANITY_CHECK

// signature table
#define st_set 1                                     //question: why need this macro definition
#define st_way 256
#define st_tag_length 16
#define st_tag_mask ((1 << st_tag_length) - 1)
#define sig_shift_length 3
#define sig_length 12
#define sig_mask ((1 << sig_length) - 1)
#define delta_length 7

// Pattern table parameters
#define pt_set 512
#define pt_way 4
#define c_sig_length 4
#define c_delta_length 4
#define c_sig_max ((1 << c_sig_length) - 1)
#define c_delta_max ((1 << c_delta_length) - 1)

// Prefetch filter parameters
#define quotient_bit  10
#define remainder_bit 6
#define hash_bit (quotient_bit + remainder_bit + 1)
#define filter_set (1 << quotient_bit)
#define fill_threshold 90
#define pf_threshold 25

// Global register parameters
#define global_counter_bit 10
#define global_counter_max ((1 << global_counter_bit) - 1) 
#define max_ghr_entry 8

enum filter_request {SPP_L2C_PREFETCH, SPP_LLC_PREFETCH, L2C_DEMAND, L2C_EVICT}; // Request type for prefetch filter
uint64_t get_hash(uint64_t key);

class signature_table {
    public:
    bool valid[st_set][st_way];
    uint32_t tag[st_set][st_way], last_offset[st_set][st_way], signature[st_set][st_way], lru[st_set][st_way];

    signature_table() {
        for (uint32_t set = 0; set < 1; set++){
            for (uint32_t way = 0; way < 256; way++){
                valid[set][way] = 0;
                tag[set][way] = 0;
                last_offset[set][way] = 0;
                sig[set][way] = 0;
                lru[set][way] = way;
            }
        }
    }
    void update_signature_table(uint64_t page, uint32_t page_offset, uint32_t &last_sig, uint32_t &present_sig, int32_t &delta);
};

class pattern_table{
    public:
    int delta[pt_set][pt_way];
    uint32_t c_delta[pt_set][pt_way], c_sig[pt_set];

    pattern_table(){
        for(uint32_t set = 0; set < pt_set; set++){
            for(uint32_t way = 0; way < pt_way; way++){
                delta[set][way] = 0;
                c_delta[set][way] = 0;
            }
            c_sig[set] = 0;
        }
    }
    void update_pattern_table(uint32_t last_sig, int present_delta);
    void read_pattern_table(uint32_t present_sig, int *prefetch_delta, uint32_t *confidence_q, uint32_t &lookahead_way, uint32_t &lookahead_conf, uint32_t &pf_q_tail, uint32_t &depth);

};

class prefetch_filter{
    public:
        uint64_t remainder_tag[filter_set];
        bool valid[filter_set], useful[filter_set];

        prefetch_filter(){
            for(uint32_t set = 0; set < filter_set; set++){
                remainder_tag[set] = 0;
                valid[set] = 0;
                useful[set] = 0;
            }
        }
        bool check(uint64_t pf_addr, filter_request filter_request);
};

class global_register{
    public:
    uint64_t pf_useful, pf_issued, global_accuracy;

    uint8_t  valid[max_ghr_entry];
    uint32_t sig[max_ghr_entry], confidence[max_ghr_entry], offset[max_ghr_entry];
    int      delta[max_ghr_entry];

    global_register(){
        pf_useful = 0;
        pf_issued = 0;
        global_accuracy = 0;

        for (uint32_t i = 0; i < max_ghr_entry; i++) {
            valid[i] = 0;
            sig[i] = 0;
            confidence[i] = 0;
            offset[i] = 0;
            delta[i] = 0;
        }
    }
    void update_entry(uint32_t pf_sig, uint32_t pf_confidence, uint32_t pf_offset, int pf_delta);
    uint32_t check_entry(uint32_t page_offset);
};

#endif