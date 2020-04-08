#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "mi_config.h"
#if defined(MI_BLE_ENABLED) && (HAVE_MSC)
#include "mible_api.h"
#include "mible_port.h"
#include "mible_trace.h"
#include "third_party/pt/pt.h"
#include "third_party/pt/pt_misc.h"
#include "common/crc32.h"
#include "common/queue.h"
#include "common/mible_beacon.h"
#include "common/mible_beacon_internal.h"
#include "common/mible_crypto.h"
#include "common/mible_rxfer.h"
#include "cryptography/mi_crypto.h"
#include "mijia_profiles/mi_service_server.h"
#include "mible_secure_auth.h"

#undef  MI_LOG_MODULE_NAME
#define MI_LOG_MODULE_NAME "secure auth"
#include "mible_log.h"

#if HAVE_MSC==0
#error "Secure auth requires Mijia Secure Chip. SHOULD add macro definition HAVE_MSC=XXX to preprocesser symbols list."
#endif


typedef struct {
    uint32_t expire_time;
    uint32_t reserved[3];
} cloud_info_t;

#define PROTOCOL_VERSION   0x0204
#define PROFILE_PIN        25
#define PAIRCODE_NUMS      6
#define RTC_TIME_DRIFT     300
#define RECORD_KEY_INFO    0x10
#define MSC_REC_LTMK       1
#define MSC_REC_MKPK       0
#define INVALID_ID         0xDEADBEEF
#define RECID_MKPK         0
#define SET_DATA_VALID(x)        (x) = 1
#define SET_DATA_INVALID(x)      (x) = 0
#define DATA_IS_VALID_P(x)       ((x) == 1)
#define DATA_IS_INVALID_P(x)     ((x) == 0)
#define IS_POWER_OF_TWO(A)       ( ((A) != 0) && ((((A) - 1) & (A)) == 0) )
#define CEIL_DIV(A, B)           (((A) + (B) - 1) / (B))
#define NORMAL_EXIT              PT_ENDED

#if TIME_PROFILE
#define TIMING_BEGIN()                                                          \
    ResetCycleCounter()

#define TIMING_END(name)                                                        \
    MI_LOG_DEBUG("%s consume time: %u us\n", (char*)name, GetCycleCounter()/(SystemCoreClock/1000000))
#else
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
    uint8_t dev_info            :1 ;
    uint8_t app_pub             :1 ;
    uint8_t dev_pub             :1 ;
    uint8_t eph_key             :1 ;
    uint8_t reserve             :1 ;
    uint8_t dev_sign            :1 ;
    uint8_t MKPK                :1 ;
    uint8_t bind_key            :1 ;

    uint8_t dev_cert            :1 ;
    uint8_t manu_cert           :1 ;
    uint8_t encrypt_reg_data    :1 ;
    uint8_t encrypt_login_data  :1 ;
    uint8_t encrypt_share_data  :1 ;
    uint8_t app_confirmation    :1 ;
    uint8_t dev_confirmation    :1 ;
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
static __ALIGN(4) session_ctx_t session_key;
static __ALIGN(4) uint8_t bind_key[16];
static __ALIGN(4) union {
    uint8_t device[64];
    uint8_t server[64];
} sign;

static union {
    uint8_t app[32];
    uint8_t dev[32];
} confirmation;

static union {
    uint8_t app[16];
    uint8_t dev[16];
} random;

static struct {
    union {
        uint8_t cipher[4];
        uint32_t crc32;
    };
    uint8_t mic[4];
} encrypt_login_data;

static msc_info_t msc_info;
static struct {
    uint8_t cipher[32];
    uint8_t mic[4];
} MKPK;
typedef struct {
    uint8_t cloud[32];
    uint8_t user[32];
} shared_token_t;

static shared_token_t shared_tokens;

static struct {
    uint8_t cipher[sizeof(shared_tokens)];
    uint8_t mic[4];
} encrypt_share_data;


static uint16_t dev_cert_len;
static uint16_t manu_cert_len;
static uint16_t root_cert_len;
static uint8_t dev_cert[512];
uint8_t cert[512];
static msc_crt_t crt;
static uint16_t IO_selected;
static uint8_t OOB[16];
static uint8_t OOB_is_avail;

static const uint8_t nonce[12] = 
{0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b};
static const uint8_t reg_info[] = "smartcfg-setup-info";
static const uint8_t log_salt[] = "smartcfg-login-salt";
static const uint8_t log_info[] = "smartcfg-login-info";
static const uint8_t share_salt[] = "smartcfg-share-salt";
static const uint8_t share_info[] = "smartcfg-share-info";
static const uint8_t userkey_info[] = "miot-shared-user-key-info";
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

static struct {
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
static void reg_procedure(void);
static void admin_login_procedure(void);
static void shared_login_procedure(void);
static int monitor(pt_t *pt);

static uint8_t                    m_is_registered;
static mi_author_stat_t           m_authorization_status;
static uint32_t                   m_key_id;
static mi_schd_event_handler_t    m_user_event_handler;

static __ALIGN(4) struct {
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

uint32_t get_mi_reg_stat(void)
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

uint32_t get_mi_authorization(void)
{
    return m_authorization_status;
}

int get_mi_one_time_passwd(uint32_t refresh_interval, mi_otp_t *p_otp, mi_otp_t *p_last_otp)
{
    if (m_is_registered != true )
        return -1;
    
    if (refresh_interval < 600  || refresh_interval > 3600 || p_otp == NULL)
        return -2;

    uint8_t key[32];
    uint8_t salt[] = "mi-lock-otp-salt";
    uint8_t info[] = "mi-lock-otp-info";

    mi_crypto_hkdf_sha256(LTMK, 32,
        salt, sizeof(salt) - 1,
        info, sizeof(info) - 1,
        key,  32);

    uint32_t current = time(NULL) / refresh_interval;
    mi_crypto_hmac_sha256(key, 32, (uint8_t*)&current, sizeof(current), (uint8_t*)p_otp);

    if (p_last_otp != NULL) {
        current--;
        mi_crypto_hmac_sha256(key, 32, (uint8_t*)&current, sizeof(current), (uint8_t*)p_last_otp);
    }

    return 0;
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

uint32_t mi_scheduler_init(uint32_t interval, mi_schd_event_handler_t handler,
    mible_libs_config_t const *p_config)
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

    TRACE_ENTER(PROFILE_PIN);

    monitor(&pt_monitor);

    switch (schd_procedure & 0xF0UL) {
    case SYS_TYPE:
        sys_procedure(schd_procedure);
        break;

    case REG_TYPE:
        reg_procedure();
        break;

    case LOG_TYPE:
        admin_login_procedure();
        break;

    case SHARED_TYPE:
        shared_login_procedure();
        break;
    }

    TRACE_EXIT(PROFILE_PIN);
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
        case REG_TYPE:
            event = SCHD_EVT_REG_FAILED;
            break;

        case LOG_TYPE:
            event = SCHD_EVT_ADMIN_LOGIN_FAILED;
            break;

        case SHARED_TYPE:
            event = SCHD_EVT_SHARE_LOGIN_FAILED;
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
    schd_is_completed = true;
    switch (evt_id) {
    case SCHD_EVT_REG_SUCCESS:
        m_is_registered = true;
        break;

    case SCHD_EVT_ADMIN_LOGIN_SUCCESS:
    case SCHD_EVT_SHARE_LOGIN_SUCCESS:
        break;

    case SCHD_EVT_REG_FAILED:
    case SCHD_EVT_ADMIN_LOGIN_FAILED:
    case SCHD_EVT_SHARE_LOGIN_FAILED:
    case SCHD_EVT_TIMEOUT:
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

    // All procedures timeout time is 30 s.
    pstimer_set(&proc_timeout_timer, 30000);

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

        // It may be comleted in schd_evt_handler().
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
    uint8_t null_rand_key[16] = {0};

    PT_BEGIN(pt);

    MSC_POWER_ON();

    errno = mible_record_read(RECORD_KEY_INFO, (uint8_t*)&mi_sysinfo, sizeof(mi_sysinfo));
    if (errno != MI_SUCCESS) {
        MI_LOG_INFO("NOT found the KEYINFO.\n");
        PT_WAIT_THREAD(pt, mi_crypto_get_crt_len(&dev_cert_len, NULL, NULL));
        PT_WAIT_THREAD(pt, mi_crypto_get_crt_der(MI_DEV_CERT, dev_cert, dev_cert_len));
        errno = mi_crypto_crt_parse_der(dev_cert, dev_cert_len, NULL, &crt);
        if (errno == MI_SUCCESS) {
            /* Dev Cert SN is big endian */
            memcpy(mi_sysinfo.did+8-crt.sn.len, crt.sn.p, crt.sn.len);
        }
        mibeacon_init(NULL, false);
        event = SCHD_EVT_KEY_NOT_FOUND;
    } else {
        MI_LOG_INFO("Found the KEYINFO.\n");
        // If rand_key is null, it means LTMK need to be migrated to MKPK.
        if (memcmp(mi_sysinfo.rand_key, null_rand_key, 16) == 0) {
            PT_WAIT_UNTIL(pt, mible_rand_num_generator(mi_sysinfo.rand_key, 16) == MI_SUCCESS);
            PT_WAIT_THREAD(pt, mi_crypto_record_read(MSC_REC_LTMK, LTMK, 32));
            mi_crypto_ccm_encrypt(mi_sysinfo.rand_key,
                                      nonce, 12,
                                       NULL, 0,
                                       LTMK, 32,
                                MKPK.cipher,
                                   MKPK.mic, 4);

            PT_WAIT_THREAD(pt, mi_crypto_record_write(MSC_REC_MKPK, (const uint8_t *)&MKPK, 36));
            errno = mible_record_write(RECORD_KEY_INFO, (uint8_t*)&mi_sysinfo, sizeof(mi_sysinfo));
            if (errno == MI_SUCCESS)
                PT_WAIT_THREAD(pt, mi_crypto_record_delete(1));
        }

    PT_WAIT_THREAD(pt, mi_crypto_record_read(MSC_REC_MKPK, (uint8_t*)&MKPK, 36));

    errno = mi_crypto_ccm_decrypt(mi_sysinfo.rand_key,
                                   nonce,  12,
                                    NULL,  0,
                             MKPK.cipher,  32,
                                    LTMK,
                                MKPK.mic,  4);
        if (errno == MI_SUCCESS) {
            mibeacon_init(mi_sysinfo.beacon_key, false);
            event = SCHD_EVT_KEY_FOUND;
        } else {
            MI_LOG_ERROR("KEYINFO decrypt failed.\n");
            mibeacon_init(NULL, false);
            event = SCHD_EVT_KEY_NOT_FOUND;
        }
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
        mibeacon_init(NULL, true);
        event = SCHD_EVT_KEY_DEL_SUCC;
    } else {
        MI_LOG_INFO("KEYINFO deleted failed! errno: %d\n", errno);
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


static int msc_self_test(pt_t *pt)
{
    static int errno = 0;
    uint8_t event;
    uint8_t *p_root_cert   = cert;
    uint8_t *p_root_pubkey = dev_pub;
    uint8_t *p_manu_cert   = cert;
    uint8_t *p_manu_pubkey = dev_pub;
    uint8_t *p_dev_cert    = cert;
    uint8_t *p_dev_pubkey  = dev_pub;
    const char testpub[]= {0xB6, 0x8A, 0xFC, 0xD3, 0xB6, 0x96, 0xEA,0xA8,
    0xD3, 0x6D, 0x06, 0x18, 0x3F, 0x60, 0x7D,0x7E,
    0x5A, 0xBE, 0xF1, 0x74, 0x12, 0x23, 0xCD,0x7B,
    0x54, 0x52, 0x77, 0x07, 0x0A, 0xF5, 0xA3,0xB1,
    0xB4, 0x57, 0x91, 0x7F, 0x81, 0x8A, 0x48,0xF3,
    0x23, 0x01, 0x8C, 0xE2, 0xD5, 0x99, 0x57,0x7E,
    0xDE, 0x53, 0x38, 0x10, 0x8B, 0x1A, 0x2B,0x9F,
    0x0B, 0xCB, 0x3A, 0xE0, 0x95, 0x03, 0xD6,0x43};
    const char testmkpk[] = {0xB6, 0x8A, 0xFC, 0xD3, 0xB6, 0x96, 0xEA,0xA8,
    0xD3, 0x6D, 0x06, 0x18, 0x3F, 0x60, 0x7D,0x7E,
    0x5A, 0xBE, 0xF1, 0x74, 0x12, 0x23, 0xCD,0x7B,
    0x54, 0x52, 0x77, 0x07, 0x0A, 0xF5, 0xA3,0xB1,
    0x54, 0x52, 0x77, 0x07};
    uint8_t tmpmkpk[36];

    PT_BEGIN(pt);

    MSC_POWER_ON();
    PT_WAIT_THREAD(pt, mi_crypto_get_crt_len(&dev_cert_len, &manu_cert_len, &root_cert_len));
    
    /* read mijia root certificate */
    PT_WAIT_THREAD(pt, mi_crypto_get_crt_der(MI_ROOT_CERT, p_root_cert, root_cert_len));
    errno = mi_crypto_crt_parse_der(p_root_cert, root_cert_len, NULL, &crt);
    if (errno != MI_SUCCESS)
        goto EXCEPTION_EXIT;

    memcpy(p_root_pubkey, crt.pub.p, 64);

    /* read mijia manufacturer certificate */
    PT_WAIT_THREAD(pt, mi_crypto_get_crt_der(MI_MANU_CERT, p_manu_cert, manu_cert_len));
    errno = mi_crypto_crt_parse_der(p_manu_cert, manu_cert_len, NULL, &crt);
    if (errno != MI_SUCCESS)
        goto EXCEPTION_EXIT;

    mi_crypto_sha256(crt.tbs.p, crt.tbs.len, sha);
    PT_WAIT_THREAD(pt, errno = mi_crypto_ecc_verify(P256R1, p_root_pubkey, sha, crt.sig));
    if (errno != NORMAL_EXIT) {
        MI_LOG_ERROR("Manufacturer cert is invalid.\n");
        goto EXCEPTION_EXIT;
    }
    memcpy(p_manu_pubkey, crt.pub.p, 64);

    /* read device certificate */
    PT_WAIT_THREAD(pt, mi_crypto_get_crt_der(MI_DEV_CERT, p_dev_cert, dev_cert_len));
    errno = mi_crypto_crt_parse_der(p_dev_cert, dev_cert_len, NULL, &crt);
    if (errno != MI_SUCCESS)
        goto EXCEPTION_EXIT;

    mi_crypto_sha256(crt.tbs.p, crt.tbs.len, sha);
    PT_WAIT_THREAD(pt, errno = mi_crypto_ecc_verify(P256R1, p_manu_pubkey, sha, crt.sig));
    if (errno != NORMAL_EXIT) {
        MI_LOG_ERROR("Device cert is invalid.\n");
        goto EXCEPTION_EXIT;
    }
    memcpy(p_dev_pubkey, crt.pub.p, 64);

    /* verify device signature */
    PT_WAIT_THREAD(pt, mi_crypto_ecc_sign(P256R1, sha, sign.device));
    PT_WAIT_THREAD(pt, errno = mi_crypto_ecc_verify(P256R1, p_dev_pubkey, sha, sign.device));
    if (errno != NORMAL_EXIT) {
        MI_LOG_ERROR("Device private key is invalid.\n");
        goto EXCEPTION_EXIT;
    }
    
    PT_WAIT_THREAD(pt, mi_crypto_ecc_keypair_gen(P256R1, dev_pub));

    memcpy(app_pub, testpub, 64);
    PT_WAIT_THREAD(pt, errno = mi_crypto_ecc_shared_secret_compute(P256R1, app_pub, eph_key));
    if (errno != PT_ENDED) {
        MI_LOG_ERROR("App pubkey is invaild.\n", errno);
        goto EXCEPTION_EXIT;
    }
    
    PT_WAIT_THREAD(pt, mi_crypto_record_write(0, (const uint8_t *)testmkpk, 32+4));
    PT_WAIT_THREAD(pt, mi_crypto_record_read(0, tmpmkpk, 32+4));
    if (memcmp(tmpmkpk, testmkpk, 36) != 0) {
        MI_LOG_ERROR("WR/RD is abnormal.\n");
        goto EXCEPTION_EXIT;
    }

#if (HAVE_MSC==2)
    PT_WAIT_THREAD(pt, mi_crypto_record_delete(0));
    PT_WAIT_THREAD(pt, mi_crypto_record_read(0, tmpmkpk, 32+4));
    {
        char empty[36] = {0};
        if (memcmp(tmpmkpk, empty, 36) != 0) {
            MI_LOG_ERROR("DEL is abnormal.\n");
            goto EXCEPTION_EXIT;
        }
    }
#endif

    errno = MI_SUCCESS;

EXCEPTION_EXIT:

    if (errno == MI_SUCCESS) {
        MI_LOG_INFO("MSC self test: "MI_LOG_COLOR_GREEN"PASS\n");
        event = SCHD_EVT_MSC_SELF_TEST_PASS;
    } else {
        MI_LOG_INFO("MSC self test: "MI_LOG_COLOR_RED"FAIL\n");
        event = SCHD_EVT_MSC_SELF_TEST_FAIL;
    }

    enqueue(&monitor_queue, &event);
    
    MSC_POWER_OFF();

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

    case SYS_MSC_SELF_TEST:
        if (need_processing.pt1)
            need_processing.pt1 = PT_SCHEDULE(msc_self_test(&pt1));
        break;
    }
}


static int reg_msc(pt_t *pt)
{
    int stat = 0;
    const uint8_t null_did[8] = {0};

    PT_BEGIN(pt);

    MSC_POWER_ON();

    TIMING_BEGIN();
    PT_WAIT_THREAD(pt, mi_crypto_ecc_keypair_gen(P256R1, dev_pub));
    TIMING_END("keygen");
    SET_DATA_VALID(signal.dev_pub);

#if DEBUG_MIBLE
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"DEV_PUB:\n"MI_LOG_COLOR_DEFAULT);
    MI_LOG_HEXDUMP(dev_pub, 64);
#endif

    PT_WAIT_THREAD(pt, msc_get_chip_info(&msc_info));

    // current protocol only support input number as OOB value
    dev_info.base_IO = IO_selected;
    dev_info.cipher_suit = msc_info.sw_ver;
    dev_info.protocol_version = PROTOCOL_VERSION;
    SET_DATA_VALID(signal.dev_info);

#if DEBUG_MIBLE
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"MSC Info:\n"MI_LOG_COLOR_DEFAULT);
    MI_LOG_HEXDUMP(&dev_info, 12);
#endif

    PT_WAIT_THREAD(pt, mi_crypto_get_crt_len(&dev_cert_len, &manu_cert_len, NULL));
    PT_WAIT_THREAD(pt, mi_crypto_get_crt_der(MI_DEV_CERT, dev_cert, dev_cert_len));
    SET_DATA_VALID(signal.dev_cert);

    if (memcmp(mi_sysinfo.did, null_did, 8) == 0) {
        stat = mi_crypto_crt_parse_der(dev_cert, dev_cert_len, NULL, &crt);
        if (stat == MI_SUCCESS) {
            /* Dev Cert SN is big endian */
            memcpy(mi_sysinfo.did+8-crt.sn.len, crt.sn.p, crt.sn.len);
        }
    }

    PT_WAIT_THREAD(pt, mi_crypto_get_crt_der(MI_MANU_CERT, cert, manu_cert_len));
    SET_DATA_VALID(signal.manu_cert);

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.app_pub));
    TIMING_BEGIN();
    PT_WAIT_THREAD(pt, stat = mi_crypto_ecc_shared_secret_compute(P256R1, app_pub, eph_key));
    TIMING_END("ecdh");
    if (stat != PT_ENDED) {
        MI_LOG_ERROR("App pubkey is invalid. stat %d\n", stat);
        goto EXIT;
    }

#if DEBUG_MIBLE
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"ECDHE:\n"MI_LOG_COLOR_DEFAULT);
    MI_LOG_HEXDUMP(eph_key, 32);
#endif

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.bind_key));

#if (PRINT_BINDKEY   == 1)
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"BINDKEY:\n"MI_LOG_COLOR_DEFAULT);
    MI_LOG_HEXDUMP(bind_key, 16);
#endif
    
    TIMING_BEGIN();
    uint8_t tmp[48];
    mi_crypto_sha256(eph_key, sizeof(eph_key), tmp);
    memcpy(&tmp[32], bind_key, sizeof(bind_key));
    mi_crypto_sha256(tmp, sizeof(tmp), sha);
    TIMING_END("SHA eSK");

#if PRINT_SHA256
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"SHA256:\n"MI_LOG_COLOR_DEFAULT);
    MI_LOG_HEXDUMP(sha, 32);
#endif

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

EXIT:
    MSC_POWER_OFF();
    
    PT_END(pt);
}

static uint8_t certs_is_sent = 0;
static uint8_t buffer[sizeof(dev_info) + sizeof(dev_pub)];
static int reg_ble(pt_t *pt)
{
    PT_BEGIN(pt);

    certs_is_sent = 0;

    format_rx_cb(&rxfer_auth, app_pub, sizeof(app_pub));
    PT_SPAWN(pt, &pt_rxfer_rx, rxfer_rx_thd(&pt_rxfer_rx, &rxfer_auth, ECC_PUBKEY));
    SET_DATA_VALID(signal.app_pub);
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"app_pub recv ""@ schd_ticks %d\n", schd_ticks);
    
    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.dev_info) && DATA_IS_VALID_P(signal.dev_pub));

    memcpy(buffer, &dev_info, sizeof(dev_info));
    memcpy(buffer+sizeof(dev_info), dev_pub, sizeof(dev_pub));
    format_tx_cb(&rxfer_auth, buffer, sizeof(dev_info) + sizeof(dev_pub));
    PT_SPAWN(pt, &pt_rxfer_tx, rxfer_tx_thd(&pt_rxfer_tx, &rxfer_auth, ECC_PUBKEY));
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"dev_info, dev_pub sent ""@ schd_ticks %d\n", schd_ticks);

    format_rx_cb(&rxfer_auth, bind_key, sizeof(bind_key));
    PT_SPAWN(pt, &pt_rxfer_rx, rxfer_rx_thd(&pt_rxfer_rx, &rxfer_auth, BIND_KEY));
    SET_DATA_VALID(signal.bind_key);
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"bindkey recv ""@ schd_ticks %d\n", schd_ticks);

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.dev_cert));

    format_tx_cb(&rxfer_auth, dev_cert, dev_cert_len);
    PT_SPAWN(pt, &pt_rxfer_tx, rxfer_tx_thd(&pt_rxfer_tx, &rxfer_auth, DEV_CERT));
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"dev_cert sent ""@ schd_ticks %d\n", schd_ticks);
    
    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.manu_cert));

    format_tx_cb(&rxfer_auth, cert, manu_cert_len);
    PT_SPAWN(pt, &pt_rxfer_tx, rxfer_tx_thd(&pt_rxfer_tx, &rxfer_auth, DEV_MANU_CERT));
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"manu_cert sent ""@ schd_ticks %d\n", schd_ticks);

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.dev_sign));

    format_tx_cb(&rxfer_auth, sign.device, sizeof(sign));
    PT_SPAWN(pt, &pt_rxfer_tx, rxfer_tx_thd(&pt_rxfer_tx, &rxfer_auth, DEV_SIGNATURE));
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"dev_sign sent ""@ schd_ticks %d\n", schd_ticks);

    certs_is_sent = 1;

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.dev_confirmation));

    format_tx_cb(&rxfer_auth, confirmation.dev, sizeof(confirmation));
    PT_SPAWN(pt, &pt_rxfer_tx, rxfer_tx_thd(&pt_rxfer_tx, &rxfer_auth, DEV_CONFIRMATION));
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"dev_confirmation sent ""@ schd_ticks %d\n", schd_ticks);

    format_rx_cb(&rxfer_auth, confirmation.app, sizeof(confirmation));
    PT_SPAWN(pt, &pt_rxfer_rx, rxfer_rx_thd(&pt_rxfer_rx, &rxfer_auth, APP_CONFIRMATION));
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"app_confirmation received ""@ schd_ticks %d\n", schd_ticks);

    format_tx_cb(&rxfer_auth, random.dev, sizeof(random));
    PT_SPAWN(pt, &pt_rxfer_tx, rxfer_tx_thd(&pt_rxfer_tx, &rxfer_auth, DEV_RANDOM));
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"dev_random sent ""@ schd_ticks %d\n", schd_ticks);

    format_rx_cb(&rxfer_auth, random.app, sizeof(random));
    PT_SPAWN(pt, &pt_rxfer_rx, rxfer_rx_thd(&pt_rxfer_rx, &rxfer_auth, APP_RANDOM));
    SET_DATA_VALID(signal.app_confirmation);
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"app_random received ""@ schd_ticks %d\n", schd_ticks);

    PT_END(pt);
}

static int reg_auth(pt_t *pt)
{

    uint32_t errno;
    uint8_t event;

    PT_BEGIN(pt);
    
    IO_selected = (schd_procedure >> 8) & (uint16_t)OOB_IO_CAP;
    if (OOB_IO_CAP != 0 && IO_selected == 0) {
        MI_LOG_ERROR("Select invalid OOB 0x%04X.\n", schd_procedure >> 8);
        PT_WAIT_UNTIL(pt, opcode_send(ERR_INVALID_OOB) == MI_SUCCESS);
        event = SCHD_EVT_REG_FAILED;
        goto EXIT;
    }

    PT_WAIT_UNTIL(pt, certs_is_sent);

    OOB_is_avail = false;
    event = SCHD_EVT_OOB_REQUEST;
    enqueue(&monitor_queue, &event);
    PT_WAIT_UNTIL(pt, OOB_is_avail == true);

    mible_rand_num_generator(random.dev, 16);
    memcpy(confirmation.dev, random.dev, sizeof(random));
    memcpy(confirmation.dev + sizeof(random), OOB, sizeof(OOB));

    TIMING_BEGIN();
    mi_crypto_hmac_sha256(eph_key, sizeof(eph_key), confirmation.dev, sizeof(confirmation), confirmation.dev);
    TIMING_END("HMAC");
    SET_DATA_VALID(signal.dev_confirmation);

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.app_confirmation));
    uint8_t validation[32];
    memcpy(validation, random.app, sizeof(random));
    memcpy(validation+sizeof(random), OOB, sizeof(OOB));
    mi_crypto_hmac_sha256(eph_key, sizeof(eph_key), validation, sizeof(validation), validation);

    if (memcmp(validation, confirmation.app, sizeof(confirmation)) != 0) {
        MI_LOG_ERROR("Confirmation failed.\n");
        PT_WAIT_UNTIL(pt, opcode_send(REG_FAILED) == MI_SUCCESS);
        event = SCHD_EVT_REG_FAILED;
        goto EXIT;
    }

    PT_WAIT_UNTIL(pt, opcode_recv() == REG_VERIFY_SUCC || opcode_recv() == REG_VERIFY_FAIL);
    if (opcode_recv() == REG_VERIFY_FAIL) {
        MI_LOG_ERROR("Authenticated failed.\n");
        PT_WAIT_UNTIL(pt, opcode_send(REG_FAILED) == MI_SUCCESS);
        event = SCHD_EVT_REG_FAILED;
        goto EXIT;
    }

    struct {
        uint8_t LTMK[32];
        uint8_t beacon_key[16];
        uint8_t cloud_key[16];
    } keys;

    TIMING_BEGIN();
    mi_crypto_hkdf_sha256(   eph_key,         sizeof(eph_key),
                                 OOB,         sizeof(OOB),
                    (void *)reg_info,         sizeof(reg_info)-1,
                     (uint8_t*)&keys,         sizeof(keys));
    TIMING_END("HKDF KEYS");

    memcpy(LTMK,                  keys.LTMK,        32);
    memcpy(mi_sysinfo.beacon_key, keys.beacon_key,  16);
    memcpy(mi_sysinfo.cloud_key,  keys.cloud_key,   16);

#if DEBUG_MIBLE
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"LTMK:\n"MI_LOG_COLOR_DEFAULT);
    MI_LOG_HEXDUMP(LTMK, 32);
#endif

    PT_WAIT_UNTIL(pt, mible_rand_num_generator(mi_sysinfo.rand_key, 16) == MI_SUCCESS);

    mi_crypto_ccm_encrypt(mi_sysinfo.rand_key,
                              nonce, 12,
                               NULL, 0,
                               LTMK, 32,
                        MKPK.cipher,
                           MKPK.mic, 4);
    SET_DATA_VALID(signal.MKPK);
    PT_WAIT_UNTIL(pt, DATA_IS_INVALID_P(signal.MKPK));

    errno = mible_record_write(RECORD_KEY_INFO, (uint8_t*)&mi_sysinfo, sizeof(mi_sysinfo));
    if (errno != MI_SUCCESS) {
        MI_LOG_INFO(MI_LOG_COLOR_RED"KEYINFO STORE FAILED: %d\n", errno);
        PT_WAIT_UNTIL(pt, opcode_send(REG_FAILED) == MI_SUCCESS);
        event = SCHD_EVT_REG_FAILED;
     } else {
        mibeacon_init(mi_sysinfo.beacon_key, true);
        PT_WAIT_UNTIL(pt, opcode_send(REG_SUCCESS) == MI_SUCCESS);
        MI_LOG_INFO("REG SUCCESS: %d\n", schd_ticks);
        event = SCHD_EVT_REG_SUCCESS;
    }

EXIT:
    enqueue(&monitor_queue, &event);

    PT_END(pt);
}

static void reg_procedure()
{
    if (m_is_registered == true) {
        opcode_send(ERR_REGISTERED);
        enqueue(&monitor_queue, (uint8_t []) {SCHD_EVT_REG_FAILED});
        return;
    }

    if (need_processing.pt0)
        need_processing.pt0 = PT_SCHEDULE(reg_auth(&pt0));

    if (need_processing.pt0 && need_processing.pt1)
        need_processing.pt1 = PT_SCHEDULE(reg_msc(&pt1));

    if (need_processing.pt0 && need_processing.pt2)
        need_processing.pt2 = PT_SCHEDULE(reg_ble(&pt2));
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
        goto EXIT;
    }

#if DEBUG_MIBLE
    MI_LOG_INFO(MI_LOG_COLOR_BLUE"LTMK:\n"MI_LOG_COLOR_DEFAULT);
    MI_LOG_HEXDUMP(LTMK, 32);
#endif
    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.eph_key));
    memcpy(buffer, eph_key, sizeof(eph_key));
    memcpy(buffer+sizeof(eph_key), LTMK, sizeof(LTMK));
    mi_crypto_hkdf_sha256( buffer,         sizeof(eph_key) + sizeof(LTMK),
                         log_salt,         sizeof(log_salt)-1,
                         log_info,         sizeof(log_info)-1,
           (uint8_t*)&session_key,         sizeof(session_key));

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.encrypt_login_data));

    errno = mi_crypto_ccm_decrypt(session_key.app_key,
                                   nonce,  12,
                                    NULL,  0,
               encrypt_login_data.cipher,  sizeof(encrypt_login_data.cipher),
        (void*)&encrypt_login_data.crc32,
                  encrypt_login_data.mic,  4);

    if (errno != MI_SUCCESS) {
        MI_LOG_ERROR("ADMIN LOGIN DECRYPT FAIL. (invalid LTMK) \n");
        PT_WAIT_UNTIL(pt, opcode_send(LOG_INVALID_LTMK) == MI_SUCCESS);
        event = SCHD_EVT_ADMIN_LOGIN_FAILED;
        goto EXIT;
    }

    crc32 = soft_crc32(dev_pub, sizeof(dev_pub), 0);

    if (crc32 == encrypt_login_data.crc32) {
        MI_LOG_INFO("ADMIN LOG SUCCESS: %d\n", schd_ticks);
        m_key_id = 0;
        mi_session_init(&session_key);
        m_authorization_status = ADMIN_AUTHORIZATION;
        PT_WAIT_UNTIL(pt, opcode_send(LOG_SUCCESS) == MI_SUCCESS);
        event = SCHD_EVT_ADMIN_LOGIN_SUCCESS;
    } else {
        MI_LOG_ERROR("ADMIN LOG FAILED. %d\n", errno);
        PT_WAIT_UNTIL(pt, opcode_send(LOG_FAILED) == MI_SUCCESS);
        event = SCHD_EVT_ADMIN_LOGIN_FAILED;
    }

EXIT:
    enqueue(&monitor_queue, &event);

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


static int verify_shared_tokens(shared_token_t * p_tokens, uint8_t * p_LTMK)
{
    uint32_t errno;
    uint8_t adata[9];
    uint8_t user_token[32];
    uint8_t user_key[32];
    struct {
        uint8_t       nonce[12];
        cloud_info_t  info;
        uint8_t       mic[4];
    } cloud_token;

    memcpy(&cloud_token, p_tokens->cloud, sizeof(cloud_token));

    // derive shared user key
    mi_crypto_hkdf_sha256(    p_LTMK,         32,
               &cloud_token.nonce[4],         8,
                        userkey_info,         sizeof(userkey_info)-1,
                 (uint8_t*)&user_key,         32);

    // calculate user token
    mi_crypto_hmac_sha256(user_key, 32, (void*)&cloud_token, sizeof(cloud_token), user_token);

    if (memcmp(user_token, p_tokens->user, sizeof(user_token)) != 0){
        MI_LOG_ERROR("Invaild user token.\n");
        return 3;
    }

    memcpy(adata, mi_sysinfo.did, 8);
    adata[8] = 0x01;

    errno = mi_crypto_ccm_decrypt(mi_sysinfo.cloud_key,
                      cloud_token.nonce, 12,
                                  adata,  9,
            (uint8_t*)&cloud_token.info, 16,
            (uint8_t*)&cloud_token.info,
                        cloud_token.mic,  4);

    if (errno != 0) {
        MI_LOG_ERROR("Invaild virtual key:%d\n", errno);
        return 2;
    }

    time_t curr_time = time(NULL);
//    MI_LOG_INFO("Local  UTC %s", ctime(&curr_time));
//    MI_LOG_INFO("Expire UTC %s", ctime(&cloud_token.info.expire_time));
    
    if (curr_time - RTC_TIME_DRIFT >= cloud_token.info.expire_time) {
        MI_LOG_ERROR("virtual key expired.\n");
        return 1;
    } else {
        memcpy(&m_key_id, &cloud_token.nonce[8], 4);
        return 0;
    }
}

static int shared_msc(pt_t *pt)
{
    PT_BEGIN(pt);

    MSC_POWER_ON();
    PT_WAIT_THREAD(pt, mi_crypto_ecc_keypair_gen(P256R1, dev_pub));
    SET_DATA_VALID(signal.dev_pub);

    if (schd_procedure == SHARED_LOG_START_W_CERT ||
        schd_procedure == SHARED_LOG_START_2FA_W_CERT) {
        PT_WAIT_THREAD(pt, mi_crypto_get_crt_len(&dev_cert_len, &manu_cert_len, NULL));
        PT_WAIT_THREAD(pt, mi_crypto_get_crt_der(MI_DEV_CERT, dev_cert, dev_cert_len));
        SET_DATA_VALID(signal.dev_cert);

        PT_WAIT_THREAD(pt, mi_crypto_get_crt_der(MI_MANU_CERT, cert, manu_cert_len));
        SET_DATA_VALID(signal.manu_cert);
    }

    mi_crypto_sha256(dev_pub, sizeof(dev_pub), sha);

    PT_WAIT_THREAD(pt, mi_crypto_ecc_sign(P256R1, sha, sign.device));
    SET_DATA_VALID(signal.dev_sign);

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.app_pub));
    PT_WAIT_THREAD(pt, mi_crypto_ecc_shared_secret_compute(P256R1, app_pub, eph_key));
    SET_DATA_VALID(signal.eph_key);

    MSC_POWER_OFF();

    PT_END(pt);
}

static int shared_ble(pt_t *pt)
{
    PT_BEGIN(pt);
    
    format_rx_cb(&rxfer_auth, app_pub, sizeof(app_pub));
    PT_SPAWN(pt, &pt_rxfer_rx, rxfer_rx_thd(&pt_rxfer_rx, &rxfer_auth, ECC_PUBKEY));
    SET_DATA_VALID(signal.app_pub);

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.dev_pub));
    format_tx_cb(&rxfer_auth, dev_pub, sizeof(dev_pub));
    PT_SPAWN(pt, &pt_rxfer_tx, rxfer_tx_thd(&pt_rxfer_tx, &rxfer_auth, ECC_PUBKEY));

    if (schd_procedure == SHARED_LOG_START_W_CERT ||
        schd_procedure == SHARED_LOG_START_2FA_W_CERT ) {
        PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.dev_cert));
        format_tx_cb(&rxfer_auth, dev_cert, dev_cert_len);
        PT_SPAWN(pt, &pt_rxfer_tx, rxfer_tx_thd(&pt_rxfer_tx, &rxfer_auth, DEV_CERT));
        MI_LOG_INFO("dev_cert send "MI_LOG_COLOR_BLUE"@ schd_time %d\n", schd_ticks);
        
        PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.manu_cert));
        format_tx_cb(&rxfer_auth, cert, manu_cert_len);
        PT_SPAWN(pt, &pt_rxfer_tx, rxfer_tx_thd(&pt_rxfer_tx, &rxfer_auth, DEV_MANU_CERT));
        MI_LOG_INFO("manu_cert send "MI_LOG_COLOR_BLUE"@ schd_time %d\n", schd_ticks);
    }

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.dev_sign));
    format_tx_cb(&rxfer_auth, sign.device, sizeof(sign));
    PT_SPAWN(pt, &pt_rxfer_tx, rxfer_tx_thd(&pt_rxfer_tx, &rxfer_auth, DEV_SIGNATURE));
    MI_LOG_INFO("dev_sign send "MI_LOG_COLOR_BLUE"@ schd_time %d\n", schd_ticks);

    format_rx_cb(&rxfer_auth, &encrypt_share_data, sizeof(encrypt_share_data));
    PT_SPAWN(pt, &pt_rxfer_rx, rxfer_rx_thd(&pt_rxfer_rx, &rxfer_auth, DEV_SHARE_INFO));
    SET_DATA_VALID(signal.encrypt_share_data);
    PT_END(pt);
}

static int shared_auth(pt_t *pt)
{
    uint32_t errno;
    uint8_t event;

    PT_BEGIN(pt);

    if (m_authorization_status == SHARE_AUTHORIZATION) {
        MI_LOG_ERROR("SHARE RE-LOGIN ERROR.\n");
        PT_WAIT_UNTIL(pt, opcode_send(ERR_REPEAT_LOGIN) == MI_SUCCESS);
        event = SCHD_EVT_SHARE_LOGIN_FAILED;
        goto EXIT;
    }

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.eph_key));
    mi_crypto_hkdf_sha256(eph_key,         sizeof(eph_key),
                       share_salt,         sizeof(share_salt)-1,
                       share_info,         sizeof(share_info)-1,
          (uint8_t *)&session_key,         sizeof(session_key));

    PT_WAIT_UNTIL(pt, DATA_IS_VALID_P(signal.encrypt_share_data));

    errno = mi_crypto_ccm_decrypt(session_key.app_key,
                                   nonce,  sizeof(nonce),
                                    NULL,  0,
               encrypt_share_data.cipher,  sizeof(encrypt_share_data.cipher),
                   (void*)&shared_tokens,
                  encrypt_share_data.mic,  sizeof(encrypt_share_data.mic));

    if (errno != MI_SUCCESS ) {
        MI_LOG_ERROR("Invaild encrypt share info.\n");
        event = SCHD_EVT_SHARE_LOGIN_FAILED;
        goto EXIT;
    }
    
// verify the virtual key

    if (verify_shared_tokens(&shared_tokens, LTMK) != 0) {
        MI_LOG_ERROR("SHARED LOG FAILED: %d\n", schd_ticks);
        PT_WAIT_UNTIL(pt, opcode_send(SHARED_LOG_FAILED) == MI_SUCCESS);
        event = SCHD_EVT_SHARE_LOGIN_FAILED;
    } else {
        MI_LOG_INFO("SHARED LOG SUCCESS: %d\n", schd_ticks);
        mi_session_init(&session_key);
        m_authorization_status = SHARE_AUTHORIZATION;
        PT_WAIT_UNTIL(pt, opcode_send(SHARED_LOG_SUCCESS) == MI_SUCCESS);
        event = SCHD_EVT_SHARE_LOGIN_SUCCESS;
    }

EXIT:
    enqueue(&monitor_queue, &event);

    PT_END(pt);
}

static void shared_login_procedure()
{
    if (m_is_registered != true) {
        opcode_send(ERR_NOT_REGISTERED);
        enqueue(&monitor_queue, (uint8_t []) {SCHD_EVT_SHARE_LOGIN_FAILED});
        return;
    }

    if (need_processing.pt0)
        need_processing.pt0 = PT_SCHEDULE(shared_auth(&pt0));

    if (need_processing.pt0 && need_processing.pt1)
        need_processing.pt1 = PT_SCHEDULE(shared_msc(&pt1));

    if (need_processing.pt0 && need_processing.pt2)
        need_processing.pt2 = PT_SCHEDULE(shared_ble(&pt2));
}

#endif
