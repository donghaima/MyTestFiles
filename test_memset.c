#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef struct era_config_t_ {
    int log_level;
    int output_stream_delay;
    int rtp_hold_time;
    int igmp_join_latency;
    int er_cache_time;
    int igmp_ttl;
    int max_channels;
    int max_clients;

    /* Parameters for the ER policers */
    int er_pkt_tb_rate;
    int er_pkt_tb_depth;

    int er_blp_tb_rate;
    int er_blp_tb_depth;

    int client_er_tb_enable;
    int client_er_tb_rate_ratio;
    int client_er_tb_depth;
} era_config_t;

int main (void)
{
    era_config_t era_cfg;

    memset(&era_cfg, -1, sizeof(era_config_t));

    return 0;
}
