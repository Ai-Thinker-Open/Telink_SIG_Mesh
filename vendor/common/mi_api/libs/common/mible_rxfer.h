#ifndef __MIBLE_RXFER_H__
#define __MIBLE_RXFER_H__
#include <stdint.h>
#include "third_party/pt/pt.h"
#include "third_party/pt/pt_misc.h"
#ifdef __cplusplus
extern "C" {
#endif

#define PASS_THROUGH     0x00
#define DEV_CERT         0x01
#define DEV_MANU_CERT    0x02
#define ECC_PUBKEY       0x03
#define DEV_SIGNATURE    0x04
#define DEV_LOGIN_INFO   0x05
#define DEV_SHARE_INFO   0x06
#define SERVER_CERT      0x07
#define SERVER_SIGN      0x08
#define MESH_CONFIG      0x09
#define APP_CONFIRMATION 0x0A
#define APP_RANDOM       0x0B
#define DEV_CONFIRMATION 0x0C
#define DEV_RANDOM       0x0D
#define BIND_KEY         0x0E
#define WIFI_CONFIG      0x0F

typedef struct {
    uint16_t   conn_handle;
    uint16_t    srv_handle;
    uint16_t  value_handle;
    uint16_t     data_size;
    uint8_t         *pdata;

    pstimer_t     *p_timer;

    uint8_t          state;
    uint8_t       cmd_type;
    uint8_t            cmd;
    uint8_t       ack_type;
    uint8_t            ack;

    uint16_t       recv_sn;
    uint16_t        max_sn;

    uint8_t  last_rx_bytes;
    uint8_t   lost_pkt_cnt;
} rxfer_cb_t;

int format_rx_cb(rxfer_cb_t *pxfer, void *p_rxd, uint16_t rxd_bytes);
int format_tx_cb(rxfer_cb_t *pxfer, void *p_txd, uint16_t txd_bytes);
int rxfer_init(rxfer_cb_t *pxfer, pstimer_t *p_timer);
void rxfer_features_get(uint8_t *p_simul_retran, uint8_t *p_dmtu);
void rxfer_process(rxfer_cb_t *pxfer, uint8_t *pdata, uint8_t len);

PT_THREAD(rxfer_rx_thd(pt_t *pt, rxfer_cb_t *pxfer, uint8_t data_type, ...));
PT_THREAD(rxfer_tx_thd(pt_t *pt, rxfer_cb_t *pxfer, uint8_t data_type));
PT_THREAD(rxfer_mng_thd(pt_t *pt, rxfer_cb_t *pxfer, uint8_t cmd, ...));

#ifdef __cplusplus
}
#endif

/** @} */

#endif  /* __MIBLE_RXFER_H__ */ 
