#include "mi_config.h"

mi_config_t m_config = {
        .developer_version   = MIBLE_DEVELOPER_VERSION,
        .model               = MODEL_NAME,
        .pid                 = PRODUCT_ID,
        .io                  = OOB_IO_CAP,
        .have_msc            = HAVE_MSC,
        .have_reset_button   = HAVE_RESET_BUTTON,
        .have_confirm_button = HAVE_CONFIRM_BUTTON,
        .schd_in_mainloop    = MI_SCHD_PROCESS_IN_MAIN_LOOP,
        .max_att_payload     = MAX_ATT_PAYLOAD,
        .dfu_start           = DFU_NVM_START,
        .dfu_size            = DFU_NVM_SIZE,
};
