#include <stdint.h>
#include <stdarg.h>
//#include <string.h>
#include <time.h>

#include "mi_config.h"
#if defined(MI_MESH_ENABLED)
#include "mible_api.h"
#include "mible_port.h"
#include "mible_log.h"
#include "mible_trace.h"
#include "third_party/pt/pt.h"
#include "third_party/pt/pt_misc.h"
#include "common/crc32.h"
#include "common/queue.h"
#include "common/tlv.h"

#include "common/mible_beacon.h"
#include "common/mible_crypto.h"
#include "common/mible_rxfer.h"
#include "cryptography/mi_crypto.h"
#include "mijia_profiles/mi_service_server.h"
#include "mible_mesh_auth.h"
#include "common/mible_beacon_internal.h"

#undef  MI_LOG_MODULE_NAME
#define MI_LOG_MODULE_NAME "mesh auth"
#include "mible_log.h"
#if MI_API_ENABLE

typedef struct {
    uint32_t expire_time;
    uint32_t reserved[3];
} shared_key_t;

#define PROTOCOL_VERSION   0x0201
#define PROFILE_PIN        25
#define PAIRCODE_NUMS      6
#define RTC_TIME_DRIFT     300
#define RECORD_KEY_INFO    0x10
#define INVALID_ID         0xDEADBEEF
#define RECID_MKPK         0
#define SET_DATA_VALID(x)        (x) = 1
#define SET_DATA_INVALID(x)      (x) = 0
#define DATA_IS_VALID_P(x)       ((x) == 1)
#define DATA_IS_INVALID_P(x)     ((x) == 0)
#define IS_POWER_OF_TWO(A)       ( ((A) != 0) && ((((A) - 1) & (A)) == 0) )
#define CEIL_DIV(A, B)           (((A) + (B) - 1) / (B))

#define TLV8_DEVKEY          1
#define TLV8_PROV_DATA       2
#define TLV8_APPKEY_LIST     3
#define TLV8_BIND_LIST       4

#if TIME_PROFILE
#define TIMING_INIT()                                                           \
    InitCycleCounter()

#define TIMING_BEGIN()                                                          \
    ResetCycleCounter()

#define TIMING_END(name)                                                        \
    MI_LOG_DEBUG("%s consume time: %u us\n", (char*)name, GetCycleCounter()/(SystemCoreClock/1000000))
#else
#define TIMING_INIT()
#define TIMING_BEGIN()
#define TIMING_END(name)
#endif

#if (HAVE_MSC)
static pstimer_t msc_timer;
#define MSC_POWER_ON()                                                          \
do {                                                                            \
    PT_WAIT_THREAD(pt, msc_power_on());                                         \
} while(0)

#define MSC_POWER_OFF()                                                         \
do {                                                                            \
    PT_WAIT_THREAD(pt, msc_power_off());                                        \
} while(0)
#else
#define MSC_POWER_ON()
#define MSC_POWER_OFF()
#endif /* HAVE_MSC */

static struct {
    uint8_t dev_info   :1 ;
    uint8_t app_pub    :1 ;
    uint8_t dev_pub    :1 ;
    uint8_t eph_key    :1 ;
    uint8_t dev_sha    :1 ;
    uint8_t dev_sign   :1 ;
    uint8_t MKPK       :1 ;
    uint8_t session_key:1 ;

    uint8_t dev_cert   :1 ;
    uint8_t manu_cert  :1 ;
    uint8_t encrypt_mesh_config:1 ;
    uint8_t encrypt_login_data :1 ;
    uint8_t encrypt_share_data :1 ;
    uint8_t server_cert :1;
    uint8_t server_sign :1;
    uint8_t server_sha  :1;
} signal;

static struct {
    uint8_t  base_IO;
    uint8_t  future_IO;
    uint8_t  resevered[6];
    uint16_t cipher_suit;
    uint16_t protocol_version;
} dev_info;

static __ALIGN(4) uint8_t app_pub[64];
static __ALIGN(4) uint8_t dev_pub[64];
static __ALIGN(4) uint8_t sha[32];
static __ALIGN(4) uint8_t eph_key[32];
static __ALIGN(4) uint8_t LTMK[32];
static __ALIGN(4) uint8_t GATT_LTMK[32];
static __ALIGN(4) session_ctx_t session_key;
static __ALIGN(4) union {
    uint8_t device[64];
    uint8_t server[64];
} sign;

struct {
    union {
        uint8_t cipher[4];
        uint32_t crc32;
    };
    uint8_t mic[4];
} encrypt_login_data;

static session_ctx_t cloud_key;
static struct {
    uint8_t cipher[32];
    uint8_t mic[4];
} MKPK;

static uint8_t manu_sn[8] = {[0] = 0};

static uint16_t mesh_config_len;
static uint16_t server_cert_len;
static uint16_t dev_cert_len;
static uint16_t manu_cert_len;
static uint8_t buffer[128];
static uint8_t xfer_buffer[512];
static msc_crt_t crt;
static uint16_t IO_selected;
static uint8_t OOB[16];
static uint8_t OOB_is_avail;

static const uint8_t reg_info[] = "miot-mesh-auth-info";
static const uint8_t ltmk_salt[] = "gatt-ltmk";
static const uint8_t ltmk_info[] = "miot-mesh-ltmk-info";
static const uint8_t log_salt[] = "miot-mesh-login-salt";
static const uint8_t log_info[] = "miot-mesh-login-info";
static const uint8_t cloud_salt[] = "smartcfg-cloud-salt";
static const uint8_t cloud_info[] = "smartcfg-cloud-info";
static const uint8_t nonce[12] = {
0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b
};
static void * schd_tick_timer;
static bool   schd_need_exec;
static bool   schd_is_completed;
static uint32_t schd_procedure;
static queue_t monitor_queue;
static uint8_t monitor_queue_buf[1];
static queue_t proc_queue;
static uint32_t proc_queue_buf[1];

uint32_t schd_ticks;
uint32_t tick_interval = 10;

struct {
    uint8_t pt0 :1;
    uint8_t pt1 :1;
    uint8_t pt2 :1;
    uint8_t pt3 :1;
    uint8_t reserve: 4;
} need_processing;
static pt_t pt0, pt1, pt2, pt3, pt_monitor;
static pt_t pt_rxfer_tx, pt_rxfer_rx;

extern rxfer_cb_t rxfer_auth;

static void schd_tick_timer_handler(void * p_context);
static void sys_procedure(uint32_t type);
static void mesh_reg_procedure(void);
static void admin_login_procedure(void);
static int monitor(pt_t *pt);

static uint8_t                    m_is_registered;
static mi_author_stat_t           m_authorization_status;
static uint32_t                   m_key_id;
static mi_schd_event_handler_t    m_user_event_handler;

static __ALIGN(4)  struct {
    uint8_t did[8];
    uint8_t beacon_key[16];
    uint8_t cloud_key[16];
    uint8_t rand_key[16];
} mi_sysinfo;

const unsigned char root_pub[] = {
0xBE,0xF5,0x8B,0x02,0xDA,0xE3,0xFF,0xF8,0x54,0x1A,0xA0,0x44,0x8F,0xBA,0xC4,0x4D,
0xB7,0xC6,0x9A,0x2F,0xA8,0xF0,0xB1,0xB6,0xFF,0x7A,0xD9,0x51,0xDB,0x66,0x28,0xFA,
0xD7,0xF0,0x20,0xEA,0x39,0xA2,0xEE,0x86,0x7F,0xDD,0x78,0x3F,0xDC,0x2F,0xB0,0x86,
0x09,0x5C,0xC2,0x85,0x04,0x13,0xA2,0x80,0x2C,0x62,0x7D,0xBD,0xC7,0x15,0xF4,0xF9,
};


int get_mi_device_id(uint8_t did[8])
{
    uint8_t null_did[8] = {0};
    if (memcmp(mi_sysinfo.did, null_did, 8) == 0) {
        return -1;
    } else {
        memcpy(did, mi_sysinfo.did, 8);
        return 0;
    }
}

uint8_t get_mi_reg_stat(void)
{
    return m_is_registered;
}

uint32_t get_mi_key_id(void)
{
    return m_authorization_status ? m_key_id : INVALID_ID;
}

void set_mi_unauthorization(void)
{
    m_authorization_status = UNAUTHORIZATION;
}

uint8_t get_mi_authorization(void)
{
    return m_authorization_status;
}

int mi_schd_oob_rsp(uint8_t const * const in, uint8_t ilen)
{
    if (IO_selected == 0)
        return -1;

    if (IO_selected != 0 && in == NULL)
        return -2;

    if (ilen > 16)
        return -3;

    if (in != NULL) {
        memcpy(OOB, in, ilen);
        memset(OOB + ilen, 0, 16 - ilen);
    }

    OOB_is_avail = true;

    return 0;
}

static int get_mi_mesh_static_oob(uint8_t *p_out, uint8_t len)
{
    if (get_mi_reg_stat() == false && len != 16)
        return -1;

    uint8_t hash[32];
    mi_crypto_sha256(LTMK, 32, hash);
    memcpy(p_out, hash, len);
    return 0;
}

uint32_t mi_scheduler_init(uint32_t interval, mi_schd_event_handler_t handler,
    mible_libs_config_t const * p_config)
{
    int32_t errno;
    tick_interval = interval;

#if (HAVE_MSC)
    msc_timer.tick_interval_ms = interval;
    msc_timer.p_curr_tick      = &schd_ticks;
#endif

    errno = mible_timer_create(&schd_tick_timer, schd_tick_timer_handler, MIBLE_TIMER_REPEATED);
    MI_ERR_CHECK(errno);

    mible_record_create(RECORD_KEY_INFO, sizeof(mi_sysinfo));

    if (handler != NULL)
        m_user_event_handler = handler;

#if (HAVE_MSC)
    if (p_config != NULL) {
        errno = mi_msc_config(p_config->msc_onoff, p_config->p_msc_iic_config, &msc_timer);
        MI_ERR_CHECK(errno);
    }
#else
    errno = mi_crypto_init();
    MI_ERR_CHECK(errno);
#endif

    queue_init(&proc_queue,
            proc_queue_buf,
            sizeof(proc_queue_buf) / sizeof(proc_queue_buf[0]),
            sizeof(proc_queue_buf[0]) );

#if TIME_PROFILE
    InitCycleCounter();
    EnableCycleCounter();
    TRACE_INIT(PROFILE_PIN);
#endif
    return 0;
}

void mi_schd_process()
{
    if (schd_need_exec == false)
        return;

	//TRACE_ENTER(PROFILE_PIN);
	
    monitor(&pt_monitor);

    switch (schd_procedure & 0xF0UL) {
    case SYS_TYPE:
        sys_procedure(schd_procedure);
        break;

    case MESH_REG_TYPE:
        mesh_reg_procedure();
        break;

    case MESH_LOGIN_TYPE:
        admin_login_procedure();
        break;
    }

    //TRACE_EXIT(PROFILE_PIN);
}

uint32_t mi_scheduler_start(uint32_t procedure)
{
    int32_t errno;

    switch (schd_procedure) {
    case SCHD_IDLE:
        schd_procedure    = procedure;
        schd_ticks        = 0;
        schd_need_exec    = true;
        schd_is_completed = false;
        break;

    case SYS_DEV_INFO_GET:
        enqueue(&proc_queue, &procedure);
        return MI_SUCCESS;

    default:
        return MI_ERR_BUSY;
    }

    PT_INIT(&pt0);
    PT_INIT(&pt1);
    PT_INIT(&pt2);
    PT_INIT(&pt3);
    PT_INIT(&pt_monitor);

    memset(&signal, 0, sizeof(signal));
    memset(&need_processing, 0xFF, sizeof(need_processing));

    errno = mible_timer_stop(schd_tick_timer);
    MI_ERR_CHECK(errno);
    errno = mible_timer_start(schd_tick_timer, tick_interval, &schd_procedure);
    MI_ERR_CHECK(errno);

    mi_schd_process();

    return errno;
}

uint32_t mi_scheduler_stop(void)
{
    queue_flush(&proc_queue);
    queue_flush(&monitor_queue);
    if (!schd_is_completed) {
        uint8_t event;
        switch (schd_procedure & 0xF0UL) {
        case MESH_REG_TYPE:
            event = SCHD_EVT_MESH_REG_FAILED;
            break;

        case MESH_LOGIN_TYPE:
            event = SCHD_EVT_ADMIN_LOGIN_FAILED;
            break;
        }
        enqueue(&monitor_queue, &event);
    }
    return 0;
}

static void schd_tick_timer_handler(void * p_context)
{
    schd_ticks++;

#if (MI_SCHD_PROCESS_IN_MAIN_LOOP==0)
    mi_schd_process();
#endif
}

static void schd_evt_handler(uint8_t evt_id)
{

    schd_evt_t event = {.id = (schd_evt_id_t)evt_id};
    tlv8_t *p_tlv;
    appkey_list_t appkey_list;
    model_bind_list_t bind_list;
    schd_is_completed = true;
    switch (evt_id) {
    case SCHD_EVT_MESH_REG_SUCCESS:
        //MI_HEXDUMP(xfer_buffer, mesh_config_len);
        tlv8_decode(xfer_buffer, mesh_config_len, TLV8_DEVKEY, &p_tlv);
        event.data.mesh_config.p_devkey = (void*)p_tlv->value;

        if (sizeof(provisionin_data_t) != 25)
            MI_LOG_ERROR("unpack prov data type is used\n");
        tlv8_decode(xfer_buffer, mesh_config_len, TLV8_PROV_DATA, &p_tlv);
        event.data.mesh_config.p_prov_data = (void*)p_tlv->value;

        tlv8_decode(xfer_buffer, mesh_config_len, TLV8_APPKEY_LIST, &p_tlv);
        appkey_list.head = (void*)p_tlv->value;
        appkey_list.size = p_tlv->len / sizeof(appkey_item_t);
        event.data.mesh_config.p_appkey_list = &appkey_list;

        tlv8_decode(xfer_buffer, mesh_config_len, TLV8_BIND_LIST, &p_tlv);
        bind_list.head = (void*)p_tlv->value;
        bind_list.size = p_tlv->len / sizeof(bind_item_t);
        event.data.mesh_config.p_bind_list = &bind_list;

        m_is_registered = true;
        break;

    case SCHD_EVT_KEY_NOT_FOUND:
        m_is_registered = false;
        break;

    case SCHD_EVT_KEY_FOUND:
        m_is_registered = true;
        break;

    case SCHD_EVT_KEY_DEL_FAIL:
        break;

    case SCHD_EVT_KEY_DEL_SUCC:
        m_is_registered = false;
        break;

    case SCHD_EVT_OOB_REQUEST:
        schd_is_completed = false;
        event.data.IO_capability = IO_selected;
        break;

    case SCHD_EVT_INTERNAL:
        return;

    default:
        break;
    }

    if (m_user_event_handler != NULL)
        m_user_event_handler(&event);
}


static int monitor(pt_t *pt)
{
    static pstimer_t proc_timeout_timer;
    uint8_t event;

    PT_BEGIN(pt);

    proc_timeout_timer.tick_interval_ms = tick_interval;
    proc_timeout_timer.p_curr_tick      = &schd_ticks;

    // All procedures timeout time is 20 s.
    pstimer_set(&proc_timeout_timer, 20000);
    queue_init(&monitor_queue,
            monitor_queue_buf,
            sizeof(monitor_queue_buf) / sizeof(monitor_queue_buf[0]),
            sizeof(monitor_queue_buf[0]) );

    while (1) {
        if (pstimer_expired(&proc_timeout_timer, NULL) == true) {
            event = SCHD_EVT_TIMEOUT;
            enqueue(&monitor_queue, &event);
        }

        if (!schd_is_completed && dequeue(&monitor_queue, &event) != MI_ERR_NOT_FOUND)
            schd_evt_handler(event);

        // It may be completed in schd_evt_handler().
        if (schd_is_completed) {
            MI_LOG_WARNING("OPERATION 0x%08X completed.\n", schd_procedure);
            memset(&need_processing, 0, sizeof(need_processing));
            MSC_POWER_OFF();
            schd_procedure = 0;
            schd_need_exec = false;
            mible_timer_stop(schd_tick_timer);

            uint32_t proc;
            if (dequeue(&proc_queue, &proc) != MI_ERR_NOT_FOUND)
                mi_scheduler_start(proc);
        }

        PT_YIELD(pt);
    }

    PT_END(pt);
}

static int psm_restore(pt_t *pt)
{
    uint8_t errno;
    uint8_t event;

    PT_BEGIN(pt);

    MSC_POWER_ON();

    errno = mible_record_read(RECORD_KEY_INFO, (uint8_t*)&mi_sysinfo, sizeof(mi_sysinfo));
    if (errno == MI_SUCCESS) {
        MI_LOG_INFO("Found the KEYINFO.\n");
        mibeacon_init(mi_sysinfo.beacon_key, false);
        PT_WAIT_THREAD(pt, mi_crypto_record_read(RECID_MKPK, (uint8_t*)&MKPK, 36));
        errno = mi_crypto_ccm_decrypt(mi_sysinfo.rand_key,
                                       nonce,  12,
                                        NULL,  0,
                                 MKPK.cipher,  32,
                                        LTMK,
                                    MKPK.mic,  4);
        if (errno == MI_SUCCESS) {
            mi_crypto_hkdf_sha256(LTMK,           sizeof(LTMK),
                            (void *)ltmk_salt,    sizeof(ltmk_salt)-1,
                            (void *)ltmk_info,    sizeof(ltmk_info)-1,
                            (void *)GATT_LTMK,    sizeof(GATT_LTMK));
            event = SCHD_EVT_KEY_FOUND;
        } else {
            event = SCHD_EVT_KEY_NOT_FOUND;
        }
    } else {
        mibeacon_init(NULL, false);
        PT_WAIT_THREAD(pt, mi_crypto_get_crt_len(&dev_cert_len, NULL, NULL));
        PT_WAIT_THREAD(pt, mi_crypto_get_crt_der(MI_DEV_CERT, xfer_buffer, dev_cert_len));
        errno = mi_crypto_crt_parse_der(xfer_buffer, dev_cert_len, NULL, &crt);
        if (errno == MI_SUCCESS) {
            /* Dev Cert SN is big endian */
            memcpy(mi_sysinfo.did+8-crt.sn.len, crt.sn.p, crt.sn.len);
        }
        PT_YIELD(pt);

        event = SCHD_EVT_KEY_NOT_FOUND;
    }

    enqueue(&monitor_queue, &event);

    MSC_POWER_OFF();

    PT_END(pt);
}

static int psm_delete(pt_t *pt)
{
    uint8_t errno;
    uint8_t event;
    PT_BEGIN(pt);

    errno = mible_record_delete(RECORD_KEY_INFO);
    if (errno == MI_SUCCESS) {
        MI_LOG_INFO("KEYINFO has been deleted! \n" );
        MSC_POWER_ON();
        PT_WAIT_THREAD(pt, mi_crypto_record_delete(RECID_MKPK));
        MSC_POWER_OFF();
        mibeacon_init(NULL, false);
        event = SCHD_EVT_KEY_DEL_SUCC;
    } else {
        MI_LOG_INFO("KEYINFO deleted failed! errno: %d\n", errno);
        PT_YIELD(pt);
        event = SCHD_EVT_KEY_DEL_FAIL;
    }

    enqueue(&monitor_queue, &event);

    PT_END(pt);
}

static int dev_info_rsp(pt_t *pt)
{
    PT_BEGIN(pt);

    dev_info.base_IO = OOB_IO_CAP;
    dev_info.cipher_suit = 0;   // ECC p256r1
    dev_info.protocol_version = PROTOCOL_VERSION;
    format_tx_cb(&rxfer_auth, &dev_info, sizeof(dev_info));
    PT_SPAWN(pt, &pt_rxfer_tx, rxfer_tx_thd(&pt_rxfer_tx, &rxfer_auth, PASS_THROUGH));
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"dev_info sent ""@ schd_ticks %d\n", schd_ticks);

    enqueue(&monitor_queue, (uint8_t []) {SCHD_EVT_INTERNAL});

    PT_END(pt);
}

static void sys_procedure(uint32_t type)
{
    switch (type) {
    case SYS_KEY_RESTORE:
        if (need_processing.pt1)
            need_processing.pt1 = PT_SCHEDULE(psm_restore(&pt1));
        break;

    case SYS_KEY_DELETE:
        if (need_processing.pt1)
            need_processing.pt1 = PT_SCHEDULE(psm_delete(&pt1));
        break;
        
    case SYS_DEV_INFO_GET:
        if (need_processing.pt1)
            need_processing.pt1 = PT_SCHEDULE(dev_info_rsp(&pt1));
        break;
    }
}

static int mesh_reg_msc(pt_t *pt)
{
    int stat = 0;

    PT_BEGIN(pt);

    MSC_POWER_ON();

//    TIMING_INIT();
    PT_WAIT_THREAD(pt, stat = mi_crypto_get_crt_len(&dev_cert_len, &manu_cert_len, NULL));
    if (stat < 0) {
        MI_LOG_ERROR("not found mijia certs\n");
        PT_EXIT(pt);
    }

    PT_WAIT_THREAD(pt, mi_crypto_get_crt_der(MI_MANU_CERT, xfer_buffer, manu_cert_len));
    mi_crypto_crt_parse_der(xfer_buffer, manu_cert_len, NULL, &crt);

    if (crt.sn.p[0] == 1 && crt.sn.len == 1) {
        /* dummy mesh manu cert */
        memset(manu_sn, 0, sizeof(manu_sn));
    } else {
        /* Cert SN big endian convert to little endian  */
        for (int i = 0; i < crt.sn.len; i++) manu_sn[crt.sn.len-1-i] = crt.sn.p[i];
    }

    PT_WAIT_THREAD(pt, mi_crypto_get_crt_der(MI_DEV_CERT, xfer_buffer, dev_cert_len));
    mi_crypto_crt_parse_der(xfer_buffer, dev_cert_len, NULL, &crt);
    memcpy(mi_sysinfo.did+8-crt.sn.len, crt.sn.p, crt.sn.len);

    MI_LOG_DEBUG("device id: ");
    MI_LOG_HEXDUMP(mi_sysinfo.did, 8);
    SET_DATA_VALID(signal.dev_cert);

    TIMING_BEGIN();
    PT_WAIT_THREAD(pt, stat = mi_crypto_ecc_keypair_gen(P256R1, dev_pub));
    TIMING_END("key pair gen");
    MI_ERR_TEST(stat, PT_ENDED);
    SET_DATA_VALID(signal.dev_pub);

#if DEBUG_MIBLE
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"DEV_PUB:\n"MI_LOG_COLOR_DEFAULT);
    MI_LOG_HEXDUMP(dev_pub, 64);
#endif

    dev_info.base_IO = IO_selected;
    dev_info.cipher_suit = 0;   // ECC p256r1
    dev_info.protocol_version = PROTOCOL_VERSION;
    SET_DATA_VALID(signal.dev_info);

#if DEBUG_MIBLE
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"MSC Info:\n"MI_LOG_COLOR_DEFAULT);
    MI_LOG_HEXDUMP(&dev_info, 12);
#endif

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.server_sha));
    TIMING_BEGIN();
    PT_WAIT_THREAD(pt, stat = mi_crypto_ecc_verify(P256R1, (uint8_t*)root_pub, sha, crt.sig));
    TIMING_END("verify cert");
    if (stat != PT_ENDED) {
        MI_LOG_ERROR("Server cert is invalid.\n");
        PT_EXIT(pt);
    }

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.app_pub));
    TIMING_BEGIN();
    PT_WAIT_THREAD(pt, mi_crypto_ecc_shared_secret_compute(P256R1, app_pub, eph_key));
    TIMING_END("ecdh");
    SET_DATA_VALID(signal.eph_key);

#if DEBUG_MIBLE
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"ECDHE:\n"MI_LOG_COLOR_DEFAULT);
    MI_LOG_HEXDUMP(eph_key, 32);
#endif

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.server_sign) && DATA_IS_VALID_P(signal.dev_sha));

    uint8_t pub[64];
    memcpy(pub, crt.pub.p, 64);
    TIMING_BEGIN();
    PT_WAIT_THREAD(pt, stat = mi_crypto_ecc_verify(P256R1, pub, sha, sign.server));
    TIMING_END("verify server sign");
    if (stat != PT_ENDED) {
        MI_LOG_ERROR("Server sign is invalid.\n");
        PT_EXIT(pt);
    }
    TIMING_BEGIN();
    PT_WAIT_THREAD(pt, mi_crypto_ecc_sign(P256R1, sha, sign.device));
    TIMING_END("ecdsa sign");
    SET_DATA_VALID(signal.dev_sign);

#if DEBUG_MIBLE
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"Sign:\n"MI_LOG_COLOR_DEFAULT);
    MI_LOG_HEXDUMP(sign.device, 64);
#endif

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.MKPK));

    PT_WAIT_THREAD(pt, mi_crypto_record_write(RECID_MKPK, (const uint8_t *)&MKPK, 32+4));
    SET_DATA_INVALID(signal.MKPK);

    MSC_POWER_OFF();
    
    PT_END(pt);
}

static int mesh_reg_ble(pt_t *pt)
{
    uint8_t *ptr;

    PT_BEGIN(pt);

    /* Stage 1: Send device certificate and pubkey */

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.dev_cert));
    format_tx_cb(&rxfer_auth, xfer_buffer, dev_cert_len);
    PT_SPAWN(pt, &pt_rxfer_tx, rxfer_tx_thd(&pt_rxfer_tx, &rxfer_auth, DEV_CERT));
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"schd_tick %3d: dev_cert sent.\n", schd_ticks);
    
    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.dev_info));
    ptr = buffer;
    memcpy(ptr, &dev_info, sizeof(dev_info));
    ptr += sizeof(dev_info);
    memcpy(ptr, manu_sn, sizeof(manu_sn));
    ptr += sizeof(manu_sn);
    memcpy(ptr, dev_pub, sizeof(dev_pub));
    ptr += sizeof(dev_pub);
    format_tx_cb(&rxfer_auth, buffer, ptr - buffer);
    PT_SPAWN(pt, &pt_rxfer_tx, rxfer_tx_thd(&pt_rxfer_tx, &rxfer_auth, ECC_PUBKEY));
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"schd_tick %3d: dev_info, manu_sn, dev_pub sent. \n", schd_ticks);

    /* Stage 2: Receive Server certificate and signature */

    format_rx_cb(&rxfer_auth, xfer_buffer, sizeof(xfer_buffer));
    PT_SPAWN(pt, &pt_rxfer_rx, rxfer_rx_thd(&pt_rxfer_rx, &rxfer_auth, SERVER_CERT, &server_cert_len));
    SET_DATA_VALID(signal.server_cert);
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"schd_tick %3d: server_cert %d bytes received.\n", schd_ticks, server_cert_len);

    format_rx_cb(&rxfer_auth, app_pub, sizeof(app_pub));
    PT_SPAWN(pt, &pt_rxfer_rx, rxfer_rx_thd(&pt_rxfer_rx, &rxfer_auth, ECC_PUBKEY));
    SET_DATA_VALID(signal.app_pub);
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"schd_tick %3d: server_pub received.\n", schd_ticks);

    format_rx_cb(&rxfer_auth, sign.server, sizeof(sign));
    PT_SPAWN(pt, &pt_rxfer_rx, rxfer_rx_thd(&pt_rxfer_rx, &rxfer_auth, SERVER_SIGN));
    SET_DATA_VALID(signal.server_sign);
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"schd_tick %3d: server_sign received.\n", schd_ticks);

    /* Stage 3: Send device signature */
    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.dev_sign));
    format_tx_cb(&rxfer_auth, sign.device, sizeof(sign));
    PT_SPAWN(pt, &pt_rxfer_tx, rxfer_tx_thd(&pt_rxfer_tx, &rxfer_auth, DEV_SIGNATURE));
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"schd_tick %3d: dev_sign sent.\n", schd_ticks);

    /* Stage 4: Receive mesh configuration */
    format_rx_cb(&rxfer_auth, xfer_buffer, sizeof(xfer_buffer));
    PT_SPAWN(pt, &pt_rxfer_rx, rxfer_rx_thd(&pt_rxfer_rx, &rxfer_auth, MESH_CONFIG, &mesh_config_len));
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"schd_tick %3d: enc mesh config %d bytes received.\n", schd_ticks, mesh_config_len);
    // CCM MIC length is 4
    mesh_config_len -= 4;
    SET_DATA_VALID(signal.encrypt_mesh_config);
    PT_END(pt); 
}

static int mesh_reg_auth(pt_t *pt)
{

    uint32_t errno;
    uint8_t event;

    PT_BEGIN(pt);

    IO_selected = (schd_procedure >> 8) & (uint16_t)OOB_IO_CAP;
    if (OOB_IO_CAP != 0 && IO_selected == 0) {
        MI_LOG_ERROR("Select invalid OOB 0x%04X.\n", schd_procedure >> 8);
        PT_WAIT_UNTIL(pt, opcode_send(ERR_INVALID_OOB) == MI_SUCCESS);
        enqueue(&monitor_queue, (uint8_t []){SCHD_EVT_MESH_REG_FAILED});
        PT_EXIT(pt);
    }

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.server_cert));
    errno = mi_crypto_crt_parse_der(xfer_buffer, server_cert_len, NULL, &crt);
    if (MI_SUCCESS == errno) {
        if (memcmp("Mijia Cloud", crt.sub_o.p, crt.sub_o.len) != 0){
            MI_LOG_ERROR("Invalid OU.\n");
            PT_WAIT_UNTIL(pt, opcode_send(MESH_REG_S_CERT_INVAILD) == MI_SUCCESS);
            enqueue(&monitor_queue, (uint8_t []) {SCHD_EVT_MESH_REG_FAILED});
            PT_EXIT(pt);
        }
        TIMING_BEGIN();
        mi_crypto_sha256(crt.tbs.p, crt.tbs.len, sha);
        TIMING_END("SHA tbs");
        SET_DATA_VALID(signal.server_sha);
    } else {
        MI_LOG_ERROR("Parse server cert failed. %d\n", errno);
        PT_WAIT_UNTIL(pt, opcode_send(MESH_REG_S_CERT_INVAILD) == MI_SUCCESS);
        enqueue(&monitor_queue, (uint8_t []) {SCHD_EVT_MESH_REG_FAILED});
        PT_EXIT(pt);
    }

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.eph_key));

    if (IO_selected == 0) {
        MI_LOG_ERROR("Select NO OOB mode.\n");
        mi_crypto_hkdf_sha256(eph_key,         sizeof(eph_key),
                                 NULL,         0,
                     (void *)reg_info,         sizeof(reg_info)-1,
                                 LTMK,         32);
    } else {
        PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.server_sign));
        OOB_is_avail = false;
        event = SCHD_EVT_OOB_REQUEST;
        enqueue(&monitor_queue, &event);
        PT_WAIT_UNTIL(pt, OOB_is_avail == true);

        mi_crypto_hkdf_sha256(   eph_key,         sizeof(eph_key),
                                     OOB,         16,
                        (void *)reg_info,         sizeof(reg_info)-1,
                                    LTMK,         32);
    }

#if DEBUG_MIBLE
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"LTMK:\n"MI_LOG_COLOR_DEFAULT);
    MI_LOG_HEXDUMP(LTMK, 32);
    PT_YIELD(pt);
#endif

    TIMING_BEGIN();
    mi_crypto_sha256(LTMK, sizeof(LTMK), sha);
    TIMING_END("SHA LTMK");
    SET_DATA_VALID(signal.dev_sha);

#if DEBUG_MIBLE
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"SHA256:\n"MI_LOG_COLOR_DEFAULT);
    MI_LOG_HEXDUMP(sha, 32);
#endif

    PT_WAIT_UNTIL(pt, (opcode_recv() & 0xFFUL) != MESH_REG_START);
    if (opcode_recv() != MESH_REG_VERIFY_SUCC) {
        MI_LOG_ERROR("Authenticated failed.\n");
        enqueue(&monitor_queue, (uint8_t []) {SCHD_EVT_MESH_REG_FAILED});
        PT_EXIT(pt);
    }

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.encrypt_mesh_config));
    uint8_t key[16];
    get_mi_mesh_static_oob(key, 16);
    errno = mi_crypto_ccm_decrypt(key,
                                  key, 8,
                                  NULL, 0,
                                  xfer_buffer, mesh_config_len,
                                  xfer_buffer,
                                  xfer_buffer + mesh_config_len, 4);
    if (errno == MI_SUCCESS) {
        PT_WAIT_UNTIL(pt, mible_rand_num_generator(mi_sysinfo.rand_key, 16) == MI_SUCCESS);
        mi_crypto_ccm_encrypt(mi_sysinfo.rand_key,
                              nonce, 12,
                              NULL, 0,
                              LTMK, 32,
                              MKPK.cipher,
                              MKPK.mic, 4);
        SET_DATA_VALID(signal.MKPK);

        mi_crypto_hkdf_sha256(LTMK,           sizeof(LTMK),
                        (void *)ltmk_salt,    sizeof(ltmk_salt)-1,
                        (void *)ltmk_info,    sizeof(ltmk_info)-1,
                        (void *)GATT_LTMK,    sizeof(GATT_LTMK));

        mi_crypto_hkdf_sha256(LTMK,           sizeof(LTMK),
                  (void *)cloud_salt,         sizeof(cloud_salt)-1,
                  (void *)cloud_info,         sizeof(cloud_info)-1,
                  (void *)&cloud_key,         sizeof(cloud_key));

        memcpy(mi_sysinfo.beacon_key, cloud_key.app_key, 16);
        memcpy(mi_sysinfo.cloud_key,  cloud_key.dev_key, 16);
        // Make sure the MKPK has been stored.
        PT_WAIT_UNTIL(pt, DATA_IS_INVALID_P(signal.MKPK));
        errno = mible_record_write(RECORD_KEY_INFO, (uint8_t*)&mi_sysinfo, sizeof(mi_sysinfo));
        if (errno == MI_SUCCESS) {
            mibeacon_init(mi_sysinfo.beacon_key, true);
            MI_LOG_INFO(MI_LOG_COLOR_RED"schd_tick %3d: REG SUCCESS.\n", schd_ticks);
            PT_WAIT_UNTIL(pt, opcode_send(MESH_REG_SUCCESS) == MI_SUCCESS);
            event = SCHD_EVT_MESH_REG_SUCCESS;
            enqueue(&monitor_queue, &event);
        } else {
            MI_LOG_INFO(MI_LOG_COLOR_RED"KEYINFO STORE FAILED: %d\n", errno);
            PT_WAIT_UNTIL(pt, opcode_send(MESH_REG_FAILED) == MI_SUCCESS);
            event = SCHD_EVT_MESH_REG_FAILED;
            enqueue(&monitor_queue, &event);
        }
    } else {
        MI_LOG_INFO(MI_LOG_COLOR_RED"DECRYPT MESH CONFIG FAILED: %d\n", errno);
        PT_WAIT_UNTIL(pt, opcode_send(MESH_REG_FAILED) == MI_SUCCESS);
        event = SCHD_EVT_MESH_REG_FAILED;
        enqueue(&monitor_queue, &event);
    }

    PT_END(pt);
}

static void mesh_reg_procedure()
{
    if (m_is_registered == true) {
        opcode_send(ERR_REGISTERED);
        enqueue(&monitor_queue, (uint8_t []) {SCHD_EVT_MESH_REG_FAILED});
        return;
    }

    if (need_processing.pt0)
        need_processing.pt0 = PT_SCHEDULE(mesh_reg_auth(&pt0));

    if (need_processing.pt0 && need_processing.pt1)
        need_processing.pt1 = PT_SCHEDULE(mesh_reg_msc(&pt1));

    if (need_processing.pt0 && need_processing.pt2)
        need_processing.pt2 = PT_SCHEDULE(mesh_reg_ble(&pt2));
}

static int admin_msc(pt_t *pt)
{
    PT_BEGIN(pt);

    MSC_POWER_ON();

    PT_WAIT_THREAD(pt, mi_crypto_ecc_keypair_gen(P256R1, dev_pub));
    SET_DATA_VALID(signal.dev_pub);

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.app_pub));

    PT_WAIT_THREAD(pt, mi_crypto_ecc_shared_secret_compute(P256R1, app_pub, eph_key));
    SET_DATA_VALID(signal.eph_key);

    MSC_POWER_OFF();

    PT_END(pt);
}

static int admin_ble(pt_t *pt)
{
    PT_BEGIN(pt);
    
    format_rx_cb(&rxfer_auth, app_pub, sizeof(app_pub));
    PT_SPAWN(pt, &pt_rxfer_rx, rxfer_rx_thd(&pt_rxfer_rx, &rxfer_auth, ECC_PUBKEY));
    SET_DATA_VALID(signal.app_pub);

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.dev_pub));
    format_tx_cb(&rxfer_auth, dev_pub, sizeof(dev_pub));
    PT_SPAWN(pt, &pt_rxfer_tx, rxfer_tx_thd(&pt_rxfer_tx, &rxfer_auth, ECC_PUBKEY));

    format_rx_cb(&rxfer_auth, &encrypt_login_data, sizeof(encrypt_login_data));
    PT_SPAWN(pt, &pt_rxfer_rx, rxfer_rx_thd(&pt_rxfer_rx, &rxfer_auth, DEV_LOGIN_INFO));
    SET_DATA_VALID(signal.encrypt_login_data);

    PT_END(pt);
}

static int admin_auth(pt_t *pt)
{
    uint32_t errno;
    uint32_t crc32;
    uint8_t event;

    PT_BEGIN(pt);

    if (m_authorization_status == ADMIN_AUTHORIZATION) {
        MI_LOG_ERROR("ADMIN RE-LOGIN ERROR.\n");
        PT_WAIT_UNTIL(pt, opcode_send(ERR_REPEAT_LOGIN) == MI_SUCCESS);
        event = SCHD_EVT_ADMIN_LOGIN_FAILED;
        enqueue(&monitor_queue, &event);
        PT_EXIT(pt);
    }

#if DEBUG_MIBLE
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"LTMK:\n"MI_LOG_COLOR_DEFAULT);
    MI_LOG_HEXDUMP(LTMK, 32);
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"GATT LTMK:\n"MI_LOG_COLOR_DEFAULT);
    MI_LOG_HEXDUMP(LTMK, 32);
#endif

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.eph_key));
    memcpy(buffer, eph_key, sizeof(eph_key));
    memcpy(buffer+sizeof(eph_key), GATT_LTMK, sizeof(GATT_LTMK));
    mi_crypto_hkdf_sha256(buffer,         sizeof(eph_key) + sizeof(GATT_LTMK),
                (void *)log_salt,         sizeof(log_salt)-1,
                (void *)log_info,         sizeof(log_info)-1,
            (void *)&session_key,         sizeof(session_key));

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.encrypt_login_data));

    errno = mi_crypto_ccm_decrypt(session_key.app_key,
                                   nonce,  12,
                                    NULL,  0,
               encrypt_login_data.cipher,  sizeof(encrypt_login_data.cipher),
        (void*)&encrypt_login_data.crc32,
                  encrypt_login_data.mic,  4);

    if (errno != MI_SUCCESS) {
        MI_LOG_ERROR("ADMIN LOGIN DECRYPT FAIL. (invalid LTMK) \n");
        PT_WAIT_UNTIL(pt, opcode_send(MESH_ADMIN_INVALID_LTMK) == MI_SUCCESS);
        event = SCHD_EVT_ADMIN_LOGIN_FAILED;
        enqueue(&monitor_queue, &event);
        PT_EXIT(pt);
    }

    crc32 = soft_crc32(dev_pub, sizeof(dev_pub), 0);

    if (crc32 == encrypt_login_data.crc32) {
        MI_LOG_INFO("ADMIN LOG SUCCESS: %d\n", schd_ticks);
        m_key_id = 0;
        mi_session_init(&session_key);
        m_authorization_status = ADMIN_AUTHORIZATION;
        PT_WAIT_UNTIL(pt, opcode_send(MESH_ADMIN_LOGIN_SUCCESS) == MI_SUCCESS);
        event = SCHD_EVT_ADMIN_LOGIN_SUCCESS;
        enqueue(&monitor_queue, &event);
    } else {
        MI_LOG_ERROR("ADMIN LOG FAILED. %d\n", errno);
        PT_WAIT_UNTIL(pt, opcode_send(MESH_ADMIN_LOGIN_FAILED) == MI_SUCCESS);
        event = SCHD_EVT_ADMIN_LOGIN_FAILED;
        enqueue(&monitor_queue, &event);
    }

    PT_END(pt);
}

static void admin_login_procedure()
{
    if (m_is_registered != true) {
        opcode_send(ERR_NOT_REGISTERED);
        enqueue(&monitor_queue, (uint8_t []) {SCHD_EVT_ADMIN_LOGIN_FAILED});
        return;
    }

    if (need_processing.pt0)
        need_processing.pt0 = PT_SCHEDULE(admin_auth(&pt0));

    if (need_processing.pt0 && need_processing.pt1)
        need_processing.pt1 = PT_SCHEDULE(admin_msc(&pt1));

    if (need_processing.pt0 && need_processing.pt2)
        need_processing.pt2 = PT_SCHEDULE(admin_ble(&pt2));
}
#endif
#endif
