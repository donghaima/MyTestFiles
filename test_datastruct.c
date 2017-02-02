#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef uint8_t boolean;


/**
 * Number of TSRAPs stored per channel
 */
#define MP_TSRAPCB_MAX_CHUNKS 30

typedef struct mp_tsrap_debug_ {
    boolean valid;      /**< TRUE if get_tsrap_data() filled in this entry*/
    //cm_seq_num_t h_pkt;   /**< CM Seq NUM of TSRAP */
    uint8_t h_frame;      /**< TS Pkt index of TSRAP */
    //abs_time_t ref_time;  /**< Time tsrap was created(smoothed RTP send time)*/
    boolean is_idr;       /**< Reflects idr_frame variable of TSRAP */
    boolean is_candidate; /**< True if MP Parser considers this 
                            *   as a good TSRAP */
}mp_tsrap_debug_t;

/**
 * Need to wrap array in a struct to work with CMAPI. Keep in mind the API
 * has a max size limit when increasing the size of this array or struct
 */
typedef struct mp_tsrap_debug_arr_ {
    mp_tsrap_debug_t entries[MP_TSRAPCB_MAX_CHUNKS];
} mp_tsrap_debug_arr_t;



static void test_routine (mp_tsrap_debug_t *debug_arr)
{
    if (debug_arr) {
        memset(debug_arr, 0, sizeof(*debug_arr) * MP_TSRAPCB_MAX_CHUNKS);

        debug_arr[0].valid = 1;
        debug_arr[MP_TSRAPCB_MAX_CHUNKS-1].valid = 1;
        //debug_arr->entries[1].valid = 1;

    }
    
}


int main (void)
{
    mp_tsrap_debug_arr_t debug_arr;
    memset(&debug_arr, 0, sizeof(debug_arr));

    test_routine(&debug_arr.entries);
    test_routine(debug_arr.entries);
    test_routine(&debug_arr.entries[0]);

    //test_routine(&debug_arr);

    return 0;
}
