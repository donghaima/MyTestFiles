#include <stdio.h>
#include <stdint.h>

typedef uint8_t boolean;

/* RCC configuration object */
typedef struct rcc_config_t_ {
    boolean rcc_enable;
    //rcc_mode_t rcc_mode;
    boolean rcc_ignore_min_backfill;
    uint32_t e_factor;
    uint32_t igmp_join_varibility;
    uint32_t rcc_overlap_loss;
    uint32_t dp_cache_lag;
    uint32_t max_idr_penalty;
} rcc_config_t;

typedef uint32_t (*rcc_buff_size_func_t)(rcc_config_t *rcc_cfg);



static rcc_buff_size_func_t sb_calc_buff_size (rcc_config_t *rcc_cfg)
{
    printf("From sb_calc_buff_size(): e=%u\n", rcc_cfg->e_factor);

    return (rcc_cfg->e_factor);
}


static rcc_buff_size_func_t bc_calc_buff_size (rcc_config_t *rcc_cfg)
{
    printf("From bc_calc_buff_size(): e=%u\n", rcc_cfg->e_factor);

    return (rcc_cfg->e_factor*2);
}


static uint32_t my_calc_buff_size (rcc_config_t *rcc_cfg)
{
    printf("From my_calc_buff_size(): e=%u\n", rcc_cfg->e_factor);

    return (rcc_cfg->e_factor*3);
}


uint32_t (*rcc_buff_size_func)(rcc_config_t *rcc_cfg) = NULL;

int main (void)
{
    uint32_t buf_size;

    rcc_buff_size_func_t rcc_calc_buff_size_1 = sb_calc_buff_size;
    rcc_buff_size_func_t rcc_calc_buff_size_2 = bc_calc_buff_size;

    rcc_buff_size_func = my_calc_buff_size;


    rcc_config_t my_rcc_cfg = {0};
    my_rcc_cfg.e_factor = 94;
    buf_size = rcc_calc_buff_size_1(&my_rcc_cfg);
    printf("Returned buff_size=%u\n", buf_size);

    my_rcc_cfg.e_factor = 54;
    buf_size = rcc_calc_buff_size_2(&my_rcc_cfg);
    printf("Returned buff_size=%u\n", buf_size);

    my_rcc_cfg.e_factor = 23;    
    buf_size = rcc_buff_size_func(&my_rcc_cfg);
    printf("Returned buff_size=%u\n", buf_size);

    return 0;
}

