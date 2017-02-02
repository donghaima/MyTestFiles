#include <stdio.h>
#include <stdint.h>

/**
 * app_msg_type_t
 * @brief
 * Type enumeration of the RTCP APP messages that are utilized in RCC
 */
typedef enum app_msg_type_t_ {
    /**@brief 
     * Previous Payload Data of Decoder: sent from server to client */
    PPDD = 0,
    /**@brief 
     * Packet Lost Indication Information: sent from client to server */
    PLII,
    /**@brief 
     * New Channel Start Information: sent from client to server*/
    NCSI,
} app_msg_type_t;

/**
 * rcc_tlv_type_t
 * @brief
 * Type enumeration of RCC TLVs encoded in RTCP APP messages
 */
typedef enum rcc_tlv_type_t_ {
    PADDING = 0,
    START_SEQ_NUM,
    START_RTP_TIMESTAMP,
    DT_EARLIEST_TIME_TO_JOIN,
    DT_REPAIR_END_TIME,
    TS_RAP,
    MIN_RCC_FILL,
    MAX_RCC_FILL,
    ACT_RCC_FILL,
    ACT_RCC_FILL_AT_JOIN,
    ER_HOLDOFF_TIME,
    FIRST_MCAST_PKT_SEQ_NUM,
} rcc_tlv_type_t;

/* RCC TLV data structure */
typedef struct rcc_tlv_t_ {
    app_msg_type_t app_type;

    union {
        struct {
            uint32_t start_seq_number;
            uint32_t start_rtp_time;
            uint32_t dt_earliest_join;
            uint32_t dt_repair_end;
            uint32_t act_rcc_fill;
            uint32_t act_rcc_fill_at_join;
            uint32_t er_holdoff_time;
            uint32_t first_mcast_pkt_seq_num;
            uint16_t tsrap_len;
            uint8_t  tsrap[0];
        } ppdd;
        
        struct {
            uint32_t min_rcc_fill;
            uint32_t max_rcc_fill;
        } plii;
        
        struct {
            uint32_t first_mcast_seq_number;
        } ncsi;
    };
} rcc_tlv_t;


int main (void)
{
    rcc_tlv_t my_tlv = {0};

    my_tlv.app_type = PPDD;
    my_tlv.ppdd.start_seq_number = 12345;
    printf("APP type=%u, data=%u\n", 
           my_tlv.app_type, my_tlv.ppdd.start_seq_number);

    my_tlv.app_type = PLII;
    my_tlv.plii.min_rcc_fill = 2300;
    printf("APP type=%u, data=%u\n", 
           my_tlv.app_type, my_tlv.plii.min_rcc_fill);

    my_tlv.app_type = NCSI;
    my_tlv.ncsi.first_mcast_seq_number = 54321;
    printf("APP type=%u, data=%u\n", 
           my_tlv.app_type, my_tlv.ncsi.first_mcast_seq_number);
}
