/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __MI_DFU_AUTH_H__
#define __MI_DFU_AUTH_H__

/* Includes ------------------------------------------------------------------*/

#include "mible_type.h"
#include "mible_dfu_main.h"
#include "cryptography/mi_crypto_backend_mbedtls.h"

/* Exported types ------------------------------------------------------------*/

typedef struct {
    uint8_t    tag[512];
    uint8_t    server_der_crt[512];
    uint8_t    developer_der_crt[512];
    msc_crt_t  server_crt;
    msc_crt_t  developer_crt;
    uint8_t    pack_sig[64];
    uint8_t    pack_sha[32];
} dfu_ctx_t;

/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

int mible_dfu_auth(dfu_ctx_t * p_ctx, mible_dfu_init_t * p_meta, uint32_t dfu_pack_size);

#endif
