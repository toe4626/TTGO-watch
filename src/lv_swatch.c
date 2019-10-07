/**
 * @file lv_test_tileview.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#ifdef ESP32
#include <lvgl.h>
#include "struct_def.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "esp_wifi.h"

extern xQueueHandle g_event_queue_handle;
extern const char *get_wifi_channel();
extern const char *get_wifi_rssi();
extern const char *get_wifi_ssid();
extern const char *get_wifi_address();
extern const char *get_wifi_mac();
extern int get_batt_percentage();
extern int get_ld1_status();
extern int get_ld2_status();
extern int get_ld3_status();
extern int get_ld4_status();
extern int get_dc2_status();
extern int get_dc3_status();
extern const char *get_s7xg_model();
extern const char *get_s7xg_ver();
extern const char *get_s7xg_join();
#else

#include <lv_examples/lv_apps/lv_swatch/lv_swatch.h>

const char *get_wifi_channel()
{
    return "12";
}
const char *get_wifi_rssi()
{
    return "-90";
}
const char *get_wifi_ssid()
{
    return "Xiaomi";
}
const char *get_wifi_address()
{
    return "192.168.1.1";
}
const char *get_wifi_mac()
{
    return "ABC:DEF:GHI:JKL";
}

typedef struct {
    float vbus_vol;
    float vbus_cur;
    float batt_vol;
    float batt_cur;
    float power;
} power_data_t;


int get_batt_percentage()
{
    return 50;
}
int get_ld1_status()
{
    return 1;
} int get_ld2_status()
{
    return 1;
} int get_ld3_status()
{
    return 0;
} int get_ld4_status()
{
    return 1;
} int get_dc2_status()
{
    return 0;
} int get_dc3_status()
{
    return 1;
}

const char *get_s7xg_model()
{
    return "s78G";
}
const char *get_s7xg_ver()
{
    return "V1.08";
}
const char *get_s7xg_join()
{
    return "unjoined";
}

// #define ENABLE_BLE
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a tileview to test their functionalities
 */
LV_IMG_DECLARE(image_location);
LV_IMG_DECLARE(img_folder);
LV_IMG_DECLARE(img_placeholder);
LV_IMG_DECLARE(img_setting);
LV_IMG_DECLARE(img_wifi);
LV_IMG_DECLARE(img_folder);
LV_IMG_DECLARE(img_menu);


LV_IMG_DECLARE(img_desktop);
LV_IMG_DECLARE(img_desktop1);
LV_IMG_DECLARE(img_desktop2);
LV_IMG_DECLARE(img_desktop3);
LV_IMG_DECLARE(img_bg0);
LV_IMG_DECLARE(img_bg1);

LV_IMG_DECLARE(img_directions);
LV_IMG_DECLARE(img_direction_up);
LV_IMG_DECLARE(img_direction_down);
LV_IMG_DECLARE(img_direction_right);
LV_IMG_DECLARE(img_direction_left);
LV_IMG_DECLARE(img_step_conut);

LV_FONT_DECLARE(font_miami);
LV_FONT_DECLARE(font_miami_32);
LV_FONT_DECLARE(font_sumptuous);
LV_FONT_DECLARE(font_sumptuous_24);

LV_IMG_DECLARE(img_power);
LV_IMG_DECLARE(img_batt1);
LV_IMG_DECLARE(img_batt2);
LV_IMG_DECLARE(img_batt3);
LV_IMG_DECLARE(img_batt4);
LV_IMG_DECLARE(img_ttgo);
LV_IMG_DECLARE(img_lora);
LV_IMG_DECLARE(img_bluetooth);
LV_IMG_DECLARE(img_alipay);
LV_IMG_DECLARE(img_wechatpay);
LV_IMG_DECLARE(img_qr);

typedef lv_res_t (*lv_menu_action_t) (lv_obj_t *obj);
typedef void (*lv_menu_destory_t) (void);

typedef struct {
    const char *name;
    lv_menu_action_t callback;
} lv_lora_struct_t;

typedef struct {
    const char *name;
    lv_menu_action_t callback;
    lv_menu_destory_t destroy;
    void *src_img;
    lv_obj_t *cont;
} lv_menu_struct_t ;

typedef struct {
    const char *name;
    lv_obj_t *label;
} lv_gps_struct_t;

typedef struct {
    const char *name;
    lv_obj_t *label;
    const char *(*get_val)(void);
} lv_wifi_struct_t;

typedef struct {
    lv_obj_t *time_label;
    lv_obj_t *step_count_label;
    lv_obj_t *date_label;
    lv_obj_t *temp_label;
} lv_main_struct_t;

static lv_obj_t *menu_cont = NULL;
static lv_obj_t *main_cont = NULL;
static lv_obj_t *g_menu_win = NULL;
static lv_obj_t *g_tileview = NULL;
static int g_menu_view_width;
static int g_menu_view_height;
static int curr_index = -1;
static bool g_menu_in = false;
static int prev = -1;
#ifdef ESP32
static wifi_auth_t auth;
#endif
static lv_main_struct_t main_data;
static char buff[256];

static lv_obj_t *img_batt = NULL;
static lv_task_t *chaging_handle = NULL;
static uint8_t changin_icons = 0;
lv_task_t *monitor_handle = NULL;
static bool gps_anim_started = false;
static lv_obj_t *gps_anim_cont = NULL;

/*****************************/
static lv_obj_t *gContainer = NULL;
static lv_obj_t *gObjecter = NULL;



static lv_res_t lv_setting_backlight_action(lv_obj_t *obj, const char *txt);
static lv_res_t lv_setting_th_action(lv_obj_t *obj);
static lv_res_t lv_tileview_action(lv_obj_t *obj, lv_coord_t x, lv_coord_t y);
static lv_res_t menubtn_action(lv_obj_t *btn);
static void lv_setWinMenuHeader(const char *title, const void *img_src, lv_action_t action);

static lv_res_t lv_file_setting(lv_obj_t *par);
static lv_res_t lv_setting(lv_obj_t *par);
static lv_res_t lv_gps_setting(lv_obj_t *par);
static lv_res_t lv_wifi_setting(lv_obj_t *par);
static lv_res_t lv_motion_setting(lv_obj_t *par);
static lv_res_t lv_power_setting(lv_obj_t *par);
static lv_res_t lv_lora_setting(lv_obj_t *par);
static lv_res_t lv_ble_setting(lv_obj_t *par);
static lv_res_t lv_pay(lv_obj_t *par);


static void lv_gps_setting_destroy();
static void lv_wifi_setting_destroy();
static void lv_file_setting_destroy();
static void lv_motion_setting_destroy();
static void lv_connect_wifi(const char *password);
static void lv_power_setting_destroy(void);
static void lv_lora_setting_destroy(void);
static void lv_setting_destroy(void);
static void lv_ble_setting_destroy();
static void lv_pay_destroy();

static lv_res_t lora_Sender(lv_obj_t *obj);
static lv_res_t lora_Receiver(lv_obj_t *obj);
static lv_res_t lora_LoRaWaln(lv_obj_t *obj);
static lv_res_t lora_HardwareInfo(lv_obj_t *obj);

static void lv_menu_del();
static const void *lv_get_batt_icon();
static lv_res_t win_btn_click(lv_obj_t *btn);

static void lv_setWinBtnInvaild(bool en);
lv_res_t lv_timer_start(void (*timer_callback)(void *), uint32_t period, void *param);
static void lv_setWinMenuHeader(const char *title, const void *img_src, lv_action_t action);


static void *motion_img_src[4] = {
    &img_direction_up,
    &img_direction_down,
    &img_direction_left,
    &img_direction_right,
};

static lv_gps_struct_t gps_data[] = {
    {.name = "lat:"},
    {.name = "lng:"},
    {.name = "satellites:"},
    {.name = "date:"},
    {.name = "altitude:"},  //meters
    {.name = "course:"},
    {.name = "speed:"}      //kmph
};

static lv_wifi_struct_t wifi_data[] = {
    {.name = "SSID", .get_val = get_wifi_ssid},
    {.name = "IP", .get_val = get_wifi_address},
    {.name = "RSSI", .get_val = get_wifi_rssi},
    {.name = "CHL", .get_val = get_wifi_channel},
    {.name = "MAC", .get_val = get_wifi_mac},
};

static lv_menu_struct_t menu_data[]  = {
#ifdef ENABLE_BLE
    {.name = "Bluetooth", .callback = lv_ble_setting, .destroy = lv_ble_setting_destroy, .src_img = &img_bluetooth},
#endif

#if defined(ACSIP_S7XG_MODULE) && !defined(UBOX_GPS_MODULE)
    {.name = "GPS", .callback = lv_gps_setting, .destroy = lv_gps_setting_destroy, .src_img = &img_placeholder},
    {.name = "LoRa", .callback = lv_lora_setting, .destroy = lv_lora_setting_destroy, .src_img = &img_lora},
#elif defined(UBOX_GPS_MODULE)
    {.name = "GPS", .callback = lv_gps_setting, .destroy = lv_gps_setting_destroy, .src_img = &img_placeholder},
#endif

    {.name = "WiFi", .callback = lv_wifi_setting, .destroy = lv_wifi_setting_destroy, .src_img = &img_wifi},
    {.name = "Power", .callback = lv_power_setting, .destroy = lv_power_setting_destroy, .src_img = &img_power},
    {.name = "Setting", .callback = lv_setting, .destroy = lv_setting_destroy, .src_img = &img_setting},
    {.name = "SD Card", .callback = lv_file_setting, .destroy = lv_file_setting_destroy, .src_img = &img_folder},
    {.name = "Sensor", .callback = lv_motion_setting, .destroy = lv_motion_setting_destroy, .src_img = &img_directions},

    //pay
    {.name = "WechatPay", .callback = lv_pay, .destroy = lv_pay_destroy, .src_img = &img_wechatpay},
    {.name = "AliPay", .callback = lv_pay, .destroy = lv_pay_destroy, .src_img = &img_alipay},

};

/*********************************************************************
 *
 *                          PAY
 *
 * ******************************************************************/
static lv_res_t lv_pay(lv_obj_t *par)
{
    gContainer = lv_obj_create(par, NULL);
    lv_obj_set_size(gContainer,  g_menu_view_width, g_menu_view_height);
    lv_obj_set_style(gContainer, &lv_style_transp_fit);
    lv_obj_t *img = lv_img_create(gContainer, NULL);
    lv_img_set_src(img, &img_qr);
    lv_obj_align(img, NULL, LV_ALIGN_CENTER, 0, 0);

}

static void lv_pay_destroy()
{
    lv_obj_del(gContainer);
    gContainer = NULL;
    gObjecter = NULL;
}

/*********************************************************************
 *
 *                          BLUETOOTH
 *
 * ******************************************************************/
typedef struct {
    lv_obj_t *lmeter;
    lv_obj_t *label;
} lv_soil_t;

lv_soil_t soil_data[3];
static bool connect = false;

extern void soil_led_control();

void lv_soil_btn_cb(lv_obj_t *obj)
{
#ifdef ESP32
    soil_led_control();
#endif
}

void lv_soil_data_update(float humidity, float temperature, int soil)
{
    if (!connect)return;
    snprintf(buff, sizeof(buff), "%.2f", humidity);
    lv_label_set_text(soil_data[0].label, buff);
    lv_lmeter_set_value(soil_data[0].lmeter, (int)humidity);

    snprintf(buff, sizeof(buff), "%.2f", temperature);
    lv_label_set_text(soil_data[1].label, buff);
    lv_lmeter_set_value(soil_data[1].lmeter, (int)temperature);

    snprintf(buff, sizeof(buff), "%d%", soil);
    lv_label_set_text(soil_data[2].label, buff);
    lv_lmeter_set_value(soil_data[2].lmeter, soil);
}


void lv_soil_test_create()
{
    if (gContainer)
        lv_obj_clean(gContainer);

    /*Create a simple style with ticker line width*/
    static lv_style_t style_lmeter1;
    lv_style_copy(&style_lmeter1, &lv_style_pretty_color);
    style_lmeter1.line.width = 2;
    style_lmeter1.line.color = LV_COLOR_SILVER;
    style_lmeter1.body.main_color = LV_COLOR_HEX(0x91bfed);         /*Light blue*/
    style_lmeter1.body.grad_color = LV_COLOR_HEX(0x04386c);         /*Dark blue*/

    for (int i = 0; i < 3; i++) {
        soil_data[i].lmeter = lv_lmeter_create(gContainer, NULL);
        lv_lmeter_set_range(soil_data[i].lmeter, 0, 100);                   /*Set the range*/
        lv_lmeter_set_value(soil_data[i].lmeter, 30);                       /*Set the current value*/
        lv_lmeter_set_style(soil_data[i].lmeter, &style_lmeter1);           /*Apply the new style*/
        lv_obj_set_size(soil_data[i].lmeter, 50, 50);

        soil_data[i].label = lv_label_create(soil_data[i].lmeter, NULL);
        lv_label_set_text(soil_data[i].label, "N/A");
        lv_label_set_style(soil_data[i].label, &lv_style_pretty);
        lv_obj_align(soil_data[i].label, NULL, LV_ALIGN_CENTER, 0, 0);

        if (i == 0)
            lv_obj_align(soil_data[i].lmeter, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 10);
        else
            lv_obj_align(soil_data[i].lmeter, soil_data[i - 1].lmeter, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
    }
    lv_obj_t *scanbtn = lv_btn_create(gContainer, NULL);
    lv_obj_align(scanbtn, gContainer, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    lv_obj_set_size(scanbtn, 100, 25);
    lv_obj_t *label = lv_label_create(scanbtn, NULL);
    lv_label_set_text(label, "LED");
    lv_btn_set_action(scanbtn, LV_BTN_ACTION_PR, lv_soil_btn_cb);
    connect = true;
}


static lv_res_t ble_list_action(lv_obj_t *obj)
{
#ifdef ESP32
    lv_setWinBtnInvaild(true);
    task_event_data_t event_data;
    event_data.type = MESS_EVENT_BLE;
    event_data.ble.event = LV_BLE_CONNECT;
    event_data.ble.index = lv_list_get_btn_index(gObjecter, obj);
    xQueueSend(g_event_queue_handle, &event_data, portMAX_DELAY);
#else
    const char *dev = lv_list_get_btn_text(obj);
    int32_t index = lv_list_get_btn_index(gObjecter, obj);
    printf("connect device : %s index:%d\n", dev, index);
#endif
    return LV_RES_OK;
}

void lv_ble_device_list_add(const char *name)
{
    if (!gObjecter) {
        lv_setWinBtnInvaild(true);
        if (!name) {
            lv_obj_t *obj = lv_obj_get_child_back(gContainer, NULL);
            lv_label_set_text(obj, "No Search Device");
            lv_obj_align(obj, NULL, LV_ALIGN_CENTER, 0, -10);
            return;
        }
        lv_obj_clean(gContainer);
        gObjecter = lv_list_create(gContainer, NULL);
        lv_obj_set_size(gObjecter,  g_menu_view_width, g_menu_view_height);
        lv_obj_align(gObjecter, gContainer, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style(gObjecter, &lv_style_transp_fit);

    }
    lv_list_add(gObjecter, SYMBOL_BLUETOOTH, name, ble_list_action);
    return LV_RES_OK;
}

static lv_res_t lv_mbox_btn_callback(lv_obj_t *obj, const char *txt)
{
    lv_mbox_start_auto_close(lv_obj_get_parent(obj), 0);
}

void lv_ble_mbox_event(const char *event_txt)
{
    printf("lv_ble_mbox_event : %s\n", event_txt);
    if (!gContainer)return;
    connect = false;
    lv_obj_clean(gContainer);
    /*Create a new background style*/
    static lv_style_t style_bg;
    lv_style_copy(&style_bg, &lv_style_pretty);
    style_bg.body.main_color = LV_COLOR_MAKE(0xf5, 0x45, 0x2e);
    style_bg.body.grad_color = LV_COLOR_MAKE(0xb9, 0x1d, 0x09);
    style_bg.body.border.color = LV_COLOR_MAKE(0x3f, 0x0a, 0x03);
    style_bg.text.color = LV_COLOR_WHITE;
    style_bg.body.padding.hor = 12;
    style_bg.body.padding.ver = 8;
    style_bg.body.shadow.width = 8;

    /*Create released and pressed button styles*/
    static lv_style_t style_btn_rel;
    static lv_style_t style_btn_pr;
    lv_style_copy(&style_btn_rel, &lv_style_btn_rel);
    style_btn_rel.body.empty = 1;                    /*Draw only the border*/
    style_btn_rel.body.border.color = LV_COLOR_WHITE;
    style_btn_rel.body.border.width = 2;
    style_btn_rel.body.border.opa = LV_OPA_70;
    style_btn_rel.body.padding.hor = 12;
    style_btn_rel.body.padding.ver = 8;

    lv_style_copy(&style_btn_pr, &style_btn_rel);
    style_btn_pr.body.empty = 0;
    style_btn_pr.body.main_color = LV_COLOR_MAKE(0x5d, 0x0f, 0x04);
    style_btn_pr.body.grad_color = LV_COLOR_MAKE(0x5d, 0x0f, 0x04);

    lv_obj_t *mbox1 = lv_mbox_create(gContainer, NULL);
    lv_mbox_set_text(mbox1, event_txt);                    /*Set the text*/
    static const char *btns[] = {"\221Apply", ""}; /*Button description. '\221' lv_btnm like control char*/
    lv_mbox_add_btns(mbox1, btns, NULL);
    lv_obj_set_width(mbox1, 180);
    lv_obj_align(mbox1, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10); /*Align to the corner*/
    lv_mbox_set_style(mbox1, LV_MBOX_STYLE_BG, &style_bg);
    lv_mbox_set_style(mbox1, LV_MBOX_STYLE_BTN_REL, &style_btn_rel);
    lv_mbox_set_style(mbox1, LV_MBOX_STYLE_BTN_PR, &style_btn_pr);
    lv_mbox_set_action(mbox1, lv_mbox_btn_callback);
}

static void lv_ble_setting_destroy()
{
    connect = false;
#ifdef ESP32
    task_event_data_t event_data;
    event_data.type = MESS_EVENT_BLE;
    event_data.ble.event = LV_BLE_DISCONNECT;
    xQueueSend(g_event_queue_handle, &event_data, portMAX_DELAY);
#endif
    lv_obj_del(gContainer);
    gContainer = NULL;
    gObjecter = NULL;
}

static lv_res_t bluetooth_scan_btn_cb( lv_obj_t *obj)
{
#ifdef ESP32
    lv_setWinBtnInvaild(false);
    task_event_data_t event_data;
    event_data.type = MESS_EVENT_BLE;
    event_data.ble.event = LV_BLE_SCAN;
    xQueueSend(g_event_queue_handle, &event_data, portMAX_DELAY);
    lv_obj_clean(gContainer);
    lv_obj_t *label = lv_label_create(gContainer, NULL);
    lv_label_set_text(label, "Scaning...");
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, -10);
#else
    const char *ssid[] = {"SoilTest0", "SoilTest1"};
    for (int i = 0; i < 2; i++) {
        lv_ble_device_list_add(ssid[i]);
    }
#endif
    return LV_RES_OK;
}


static lv_res_t lv_ble_setting(lv_obj_t *par)
{
    lv_obj_t *label = NULL;
    gContainer = lv_obj_create(par, NULL);
    lv_obj_set_size(gContainer,  g_menu_view_width, g_menu_view_height);
    lv_obj_set_style(gContainer, &lv_style_transp_fit);

    // lv_soil_test_create();return;
    // for (int i = 0; i < sizeof(wifi_data) / sizeof(wifi_data[0]); ++i) {
    //     wifi_data[i].label = lv_label_create(gContainer, NULL);
    //     snprintf(buff, sizeof(buff), "%s:%s", wifi_data[i].name, wifi_data[i].get_val());
    //     lv_label_set_text(wifi_data[i].label, buff);
    //     if (!i)
    //         lv_obj_align(wifi_data[i].label, gContainer, LV_ALIGN_IN_TOP_MID, 0, 0);
    //     else
    //         lv_obj_align(wifi_data[i].label, wifi_data[i - 1].label, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    // }

    lv_obj_t *scanbtn = lv_btn_create(gContainer, NULL);
    lv_obj_align(scanbtn, gContainer, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    lv_obj_set_size(scanbtn, 100, 25);
    label = lv_label_create(scanbtn, NULL);
    lv_label_set_text(label, "Scan");
    lv_btn_set_action(scanbtn, LV_BTN_ACTION_PR, bluetooth_scan_btn_cb);
    return LV_RES_OK;
}

/*********************************************************************
 *
 *                          LORA
 *
 * ******************************************************************/


#define LV_LORA_TITLE_1 "Sender"
#define LV_LORA_TITLE_2 "Receiver"
#define LV_LORA_TITLE_3 "LoRaWaln"
#define LV_LORA_TITLE_4 "Hardware"
static const char *loraMap[] = {LV_LORA_TITLE_1, "\n",
                                LV_LORA_TITLE_2, "\n",
                                LV_LORA_TITLE_3, "\n",
                                LV_LORA_TITLE_4,
                                ""
                               };


lv_lora_struct_t lora_data [] = {
    {"Sender", lora_Sender},
    {"Receiver", lora_Receiver},
    {"LoRaWaln", lora_LoRaWaln},
    {"Hardware", lora_HardwareInfo}
};


static lv_res_t lora_HardwareInfo(lv_obj_t *obj)
{
    static lv_wifi_struct_t lora_data[] = {
        {.name = "Model", .get_val = get_s7xg_model},
        {.name = "Version", .get_val = get_s7xg_ver},
        {.name = "Join", .get_val = get_s7xg_join},
    };

    lv_obj_t *label = NULL;
    gObjecter = lv_obj_create(g_menu_win, NULL);
    lv_obj_set_size(gObjecter,  g_menu_view_width, g_menu_view_height);
    lv_obj_set_style(gObjecter, &lv_style_transp_fit);
    for (int i = 0; i < sizeof(lora_data) / sizeof(lora_data[0]); ++i) {

        lora_data[i].label = lv_label_create(gObjecter, NULL);
        snprintf(buff, sizeof(buff), "%s:%s", lora_data[i].name, lora_data[i].get_val());
        lv_label_set_text(lora_data[i].label, buff);
        if (!i)
            lv_obj_align(lora_data[i].label, gObjecter, LV_ALIGN_IN_TOP_MID, 0, 0);
        else
            lv_obj_align(lora_data[i].label, lora_data[i - 1].label, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    }
    return LV_RES_OK;
}

static lv_obj_t *ta1 = NULL;

void timer1_callback(void *a)
{
    static int i = 0;
    char bf[128];
    snprintf(bf, sizeof(bf), "lora send %d\n", ++i);
    if (lv_txt_get_encoded_length(lv_ta_get_text(ta1)) >= lv_ta_get_max_length(ta1)) {
        lv_ta_set_text(ta1, "");    /*Set an initial text*/
    }
    lv_ta_add_text(ta1, bf);
}

void lora_add_message(const char *txt)
{
    if (!txt || !ta1)return;
    if (lv_txt_get_encoded_length(lv_ta_get_text(ta1)) >= lv_ta_get_max_length(ta1)) {
        lv_ta_set_text(ta1, "");
    }
    lv_ta_add_text(ta1, txt);
}


static void lora_create_windows()
{
    static lv_style_t style_sb;
    lv_style_copy(&style_sb, &lv_style_transp_fit);

    gObjecter = lv_obj_create(g_menu_win, NULL);
    lv_obj_set_size(gObjecter,  g_menu_view_width, g_menu_view_height);
    lv_obj_set_style(gObjecter, &lv_style_transp_fit);

    /*Create a normal Text area*/
    ta1 = lv_ta_create(gObjecter, NULL);
    lv_obj_set_size(ta1, g_menu_view_width, g_menu_view_height);
    lv_obj_align(ta1, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_ta_set_style(ta1, LV_TA_STYLE_BG, &style_sb);                    /*Apply the scroll bar style*/
    lv_ta_set_cursor_type(ta1, LV_CURSOR_NONE);
    lv_ta_set_text(ta1, "");    /*Set an initial text*/
    lv_ta_set_max_length(ta1, 255);
}

static lv_res_t lora_Sender(lv_obj_t *obj)
{
    lora_create_windows();
    // lv_task_t *handle =  lv_task_create(timer1_callback, 500, LV_TASK_PRIO_LOW, NULL);
#ifdef ESP32
    task_event_data_t event_data;
    event_data.type = MESS_EVENT_LORA;
    event_data.lora.event = LVGL_S7XG_LORA_SEND;
    xQueueSend(g_event_queue_handle, &event_data, portMAX_DELAY);
#endif
    return LV_RES_OK;
}

static lv_res_t lora_Receiver(lv_obj_t *obj)
{
    lora_create_windows();
#ifdef ESP32
    task_event_data_t event_data;
    event_data.type = MESS_EVENT_LORA;
    event_data.lora.event = LVGL_S7XG_LORA_RECV;
    xQueueSend(g_event_queue_handle, &event_data, portMAX_DELAY);
#endif
    return LV_RES_OK;
}

static lv_res_t lora_LoRaWaln(lv_obj_t *obj)
{
    lora_create_windows();
    return LV_RES_OK;
}

lv_res_t lv_lora_action (struct _lv_obj_t *obj)
{
#ifdef ESP32
    task_event_data_t event_data;
    event_data.type = MESS_EVENT_LORA;
    event_data.lora.event = LVGL_S7XG_LORA_STOP;
    xQueueSend(g_event_queue_handle, &event_data, portMAX_DELAY);
#endif

    if (gObjecter)
        lv_obj_del(gObjecter);
    gObjecter = NULL;
    ta1 = NULL;
    lv_obj_set_hidden(gContainer, false);
    lv_setWinMenuHeader(NULL, SYMBOL_HOME, win_btn_click);
}

/*Called when a button is released ot long pressed*/
static lv_res_t lora_btnm_action(lv_obj_t *btnm, const char *txt)
{
    for (int i = 0; i < sizeof(lora_data) / sizeof(lora_data[0]); ++i) {
        if (strcmp(txt, lora_data[i].name)  == 0) {
            lv_obj_set_hidden(gContainer, true);
            lv_setWinMenuHeader(NULL, SYMBOL_LEFT, lv_lora_action);
            // printf("[%d] %s\n", i, lora_data[i].name);
            if (lora_data[i].callback) {
                lora_data[i].callback(NULL);
            }
            break;
        }
    }
    return LV_RES_OK; /*Return OK because the button matrix is not deleted*/
}


static lv_res_t lv_lora_setting(lv_obj_t *par)
{
    gContainer = lv_obj_create(par, NULL);
    lv_obj_set_size(gContainer,  g_menu_view_width, g_menu_view_height);
    lv_obj_set_style(gContainer, &lv_style_transp_fit);

    /*Create a default button matrix*/
    lv_obj_t *btnm1 = lv_btnm_create(gContainer, NULL);
    lv_btnm_set_map(btnm1, loraMap);
    lv_btnm_set_action(btnm1, lora_btnm_action);
    lv_obj_set_size(btnm1, 180, LV_VER_RES / 2);
    lv_obj_align(btnm1, NULL, LV_ALIGN_CENTER, 0, -20);
    return LV_RES_OK;
}

static void  lv_lora_setting_destroy(void)
{
    lv_obj_del(gContainer);
    gContainer = NULL;
}

/*********************************************************************
 *
 *                          OHTER
 *
 * ******************************************************************/

static void lv_setWinBtnInvaild(bool en)
{
    lv_win_ext_t *ext = lv_obj_get_ext_attr(g_menu_win);
    lv_obj_t *obj = NULL;
    obj = lv_obj_get_child_back(ext->header, NULL);
    obj = lv_obj_get_child_back(ext->header, obj);
    if (obj != NULL) {
        lv_obj_set_click(obj, en);
    }
}


static void lv_setWinMenuHeader(const char *title, const void *img_src, lv_action_t action)
{
    lv_win_ext_t *ext = lv_obj_get_ext_attr(g_menu_win);
    lv_obj_t *obj = NULL;
    obj = lv_obj_get_child_back(ext->header, NULL);
    if (!title)
        lv_label_set_text(ext->title, title);
    obj = lv_obj_get_child_back(ext->header, obj);
    if (obj != NULL) {
        lv_btn_set_action(obj, LV_BTN_ACTION_CLICK, action);
        obj = lv_obj_get_child_back(obj, NULL);
        lv_img_set_src(obj, img_src);
    }
}

lv_res_t lv_timer_start(void (*timer_callback)(void *), uint32_t period, void *param)
{
    lv_task_t *handle =  lv_task_create(timer_callback, period, LV_TASK_PRIO_LOW, param);
    lv_task_once(handle);
    return LV_RES_OK;
}


static lv_point_t lv_font_get_size(lv_obj_t *obj)
{
    lv_style_t *style = lv_obj_get_style(obj);
    const lv_font_t *font = style->text.font;
    lv_label_ext_t *ext = lv_obj_get_ext_attr(obj);
    lv_point_t size;
    lv_txt_flag_t flag = LV_TXT_FLAG_NONE;
    if (ext->recolor != 0) flag |= LV_TXT_FLAG_RECOLOR;
    if (ext->expand != 0) flag |= LV_TXT_FLAG_EXPAND;
    lv_txt_get_size(&size, ext->text, font, style->text.letter_space, style->text.line_space, LV_COORD_MAX, flag);
    return size;
}
/*********************************************************************
 *
 *                          BATTERY
 *
 * ******************************************************************/
void lv_update_battery_percent(int percent)
{
    static void *img_src[4] = {
        &img_batt4,
        &img_batt3,
        &img_batt2,
        &img_batt1,
    };
    if (g_menu_in) {
        int i = 0;
        if (percent > 92) {
            i = 0;
        } else if (percent > 80) {
            i = 1;
        } else if (percent > 50) {
            i = 2;
        } else {
            i = 3;
        }
        lv_img_set_src(img_batt, img_src[i]);
    }
}

static const void *lv_get_batt_icon()
{
    int percent = get_batt_percentage();
    if (percent > 98) {
        return &img_batt4;
    }
    if (percent > 80) {
        return &img_batt3;
    }
    if (percent > 50) {
        return &img_batt2;
    } else {
        return &img_batt1;
    }
}

void charging_anim_callback()
{
    static void *src_img[] = {
        &img_batt1,
        &img_batt2,
        &img_batt3,
        &img_batt4
    };
    if (g_menu_in) {
        changin_icons = changin_icons + 1 >= sizeof(src_img) / sizeof(src_img[0]) ? 0 : changin_icons + 1;
        lv_img_set_src(img_batt, src_img[changin_icons]);
    }
}

void charging_anim_stop()
{
    if (chaging_handle) {
        lv_task_del(chaging_handle);
        chaging_handle = NULL;
        changin_icons = 0;
        lv_img_set_src(img_batt, lv_get_batt_icon());
    }
}

void charging_anim_start()
{
    if (!chaging_handle) {
        chaging_handle =  lv_task_create(charging_anim_callback, 1000, LV_TASK_PRIO_LOW, NULL);
    }
}



/*********************************************************************
 *
 *                          POWER
 *
 * ******************************************************************/

typedef struct {
    int base;
    int x;
    int y;
    lv_obj_t *label;
    lv_align_t align;
    const char *txt;
    int (*get_func)(void);
} lv_power_list_t;

lv_power_list_t list[] = {
    {-1, 10, 0, NULL, LV_ALIGN_IN_TOP_MID, "Batt"}, //0
    {0, -50, 0, NULL, LV_ALIGN_OUT_LEFT_MID, "USB"},  //1
    {0, 40, 0, NULL, LV_ALIGN_OUT_RIGHT_MID, "Uint"}, //2
    {0, 0, 10, NULL, LV_ALIGN_OUT_BOTTOM_MID, "0000.00"},  //3
    {1, 0, 10, NULL, LV_ALIGN_OUT_BOTTOM_MID, "0000.00"},  //4
    {2, 0, 10, NULL, LV_ALIGN_OUT_BOTTOM_MID, "mV"},  //5
    {3, 0, 10, NULL, LV_ALIGN_OUT_BOTTOM_MID, "0000.00"},  //6
    {4, 0, 10, NULL, LV_ALIGN_OUT_BOTTOM_MID, "0000.00"},  //7
    {5, 0, 10, NULL, LV_ALIGN_OUT_BOTTOM_MID, "mA"},  //8
};

enum {
    LV_BATT_VOL_INDEX = 3,
    LV_VBUS_VOL_INDEX = 4,
    LV_BATT_CUR_INDEX = 6,
    LV_VBUS_CUR_INDEX = 7,
};

void lv_update_power_info(power_data_t *data)
{
    snprintf(buff, sizeof(buff), "%.2f", data->vbus_vol);
    lv_label_set_text(list[LV_VBUS_VOL_INDEX].label, buff);

    snprintf(buff, sizeof(buff), "%.2f", data->vbus_cur);
    lv_label_set_text(list[LV_VBUS_CUR_INDEX].label, buff);

    snprintf(buff, sizeof(buff), "%.2f", data->batt_vol);
    lv_label_set_text(list[LV_BATT_VOL_INDEX].label, buff);

    snprintf(buff, sizeof(buff), "%.2f", data->batt_cur);
    lv_label_set_text(list[LV_BATT_CUR_INDEX].label, buff);
}

static lv_res_t lv_power_setting(lv_obj_t *par)
{
    gContainer = lv_obj_create(g_menu_win, NULL);
    lv_obj_set_size(gContainer,  g_menu_view_width, g_menu_view_height);
    lv_obj_set_style(gContainer, &lv_style_transp_fit);

    static lv_style_t style_txt;
    lv_style_copy(&style_txt, &lv_style_plain);
    style_txt.text.font = &lv_font_dejavu_20;
    style_txt.text.letter_space = 2;
    style_txt.text.line_space = 1;
    style_txt.text.color = LV_COLOR_HEX(0xffffff);

    for (int i = 0; i < sizeof(list) / sizeof(list[0]); i++) {
        list[i].label = lv_label_create(gContainer, NULL);
        lv_label_set_text(list[i].label, list[i].txt);
        if ( list[i].base != -1)
            lv_obj_align( list[i].label, list[list[i].base].label, list[i].align, list[i].x, list[i].y);
        else
            lv_obj_align( list[i].label, gContainer, LV_ALIGN_IN_TOP_MID, list[i].x, list[i].y);
        lv_obj_set_style(list[i].label, &style_txt);
    }

    lv_power_list_t state[] = {
        {-1, 0, 85, NULL, LV_ALIGN_IN_TOP_MID, "LD2"},
        {-2, 0, 5, NULL, LV_ALIGN_OUT_BOTTOM_MID, NULL, .get_func = get_ld2_status},
        {0, -50, 0, NULL, LV_ALIGN_OUT_LEFT_MID, "LD1"},
        {-2, 0, 5, NULL, LV_ALIGN_OUT_BOTTOM_MID, NULL, .get_func = get_ld1_status},
        {0, 40, 0, NULL, LV_ALIGN_OUT_RIGHT_MID, "LD3"},
        {-2, 0, 5, NULL, LV_ALIGN_OUT_BOTTOM_MID, NULL, .get_func = get_ld3_status},

        {1, 0, 0, NULL, LV_ALIGN_OUT_BOTTOM_MID, "DC2"},
        {-2, 0, 5, NULL, LV_ALIGN_OUT_BOTTOM_MID, NULL, .get_func = get_dc2_status},
        {3, 0, 0, NULL, LV_ALIGN_OUT_BOTTOM_MID, "LD4"},
        {-2, 0, 5, NULL, LV_ALIGN_OUT_BOTTOM_MID, NULL, .get_func = get_ld4_status},
        {5, 0, 0, NULL, LV_ALIGN_OUT_BOTTOM_MID, "DC3"},
        {-2, 0, 5, NULL, LV_ALIGN_OUT_BOTTOM_MID, NULL, .get_func = get_dc3_status},
    };

    static lv_style_t style_led;
    lv_style_copy(&style_led, &lv_style_pretty_color);
    style_led.body.radius = LV_RADIUS_CIRCLE;
    style_led.body.main_color = LV_COLOR_GREEN;
    style_led.body.grad_color = LV_COLOR_GREEN;
    style_led.body.border.color = LV_COLOR_GREEN;
    style_led.body.border.width = 3;
    style_led.body.border.opa = LV_OPA_100;
    style_led.body.shadow.color = LV_COLOR_GREEN;
    style_led.body.shadow.width = 0;

    for (int i = 0; i < sizeof(state) / sizeof(state[0]); i++) {
        if (state[i].base == -2) {
            state[i].label  = lv_led_create(gContainer, NULL);
            lv_obj_set_size(state[i].label, 10, 10);
            lv_obj_set_style(state[i].label, &style_led);
            lv_obj_align( state[i].label, state[i - 1].label, state[i].align, state[i].x, state[i].y);

            if (state[i].get_func()) {
                lv_led_on(state[i].label);
            } else {
                lv_led_off(state[i].label);
            }

        } else {
            state[i].label = lv_label_create(gContainer, NULL);
            lv_label_set_text(state[i].label, state[i].txt);
            if ( state[i].base != -1)
                lv_obj_align( state[i].label, state[state[i].base].label, state[i].align, state[i].x, state[i].y);
            else
                lv_obj_align( state[i].label, gContainer, LV_ALIGN_IN_TOP_MID, state[i].x, state[i].y);
            lv_obj_set_style(state[i].label, &style_txt);
        }
    }

#ifdef ESP32
    task_event_data_t event_data;
    event_data.type = MESS_EVENT_POWER;
    event_data.power.event = LVGL_POWER_GET_MOINITOR;
    xQueueSend(g_event_queue_handle, &event_data, portMAX_DELAY);
#endif
    return LV_RES_OK;
}

static void lv_power_setting_destroy(void)
{
#ifdef ESP32
    task_event_data_t event_data;
    event_data.type = MESS_EVENT_POWER;
    event_data.power.event = LVGL_POWER_MOINITOR_STOP;
    xQueueSend(g_event_queue_handle, &event_data, portMAX_DELAY);
#endif
    lv_obj_del(gContainer);
    gContainer = NULL;
}

/*********************************************************************
 *
 *                          Sensor
 *
 * ******************************************************************/
void motion_dir_update(uint8_t index)
{
    if (index >= sizeof(motion_img_src) / sizeof(motion_img_src[0]))
        return;
    if (!gObjecter)
        return;
    lv_img_set_src(gObjecter, motion_img_src[index]);
    lv_obj_align(gObjecter, NULL, LV_ALIGN_CENTER, 0, 0);
}

static lv_res_t lv_motion_setting(lv_obj_t *par)
{
    gContainer = lv_obj_create(g_menu_win, NULL);
    lv_obj_set_size(gContainer,  g_menu_view_width, g_menu_view_height);
    lv_obj_set_style(gContainer, &lv_style_transp_fit);

    gObjecter = lv_img_create(gContainer, NULL);
    lv_img_set_src(gObjecter, motion_img_src[0]);
    lv_obj_align(gObjecter, NULL, LV_ALIGN_CENTER, 0, 0);

#ifdef ESP32
    task_event_data_t event_data;
    event_data.type = MESS_EVENT_MOTI;
    event_data.motion.event = LVGL_MOTION_GET_ACCE;
    xQueueSend(g_event_queue_handle, &event_data, portMAX_DELAY);
#endif
    return LV_RES_OK;
}

static void lv_motion_setting_destroy(void)
{
#ifdef ESP32
    task_event_data_t event_data;
    event_data.type = MESS_EVENT_MOTI;
    event_data.motion.event = LVGL_MOTION_STOP;
    xQueueSend(g_event_queue_handle, &event_data, portMAX_DELAY);
#endif
    lv_obj_del(gContainer);
    gContainer = NULL;
    gObjecter = NULL;
}

/*********************************************************************
 *
 *                          GPS
 *
 * ******************************************************************/
// #define ESP32
static char buffer[1024];
lv_obj_t *gps_txt;

uint8_t lv_gps_static_text_update(void *data)
{
#ifdef ESP32
    gps_struct_t *gps = (gps_struct_t *)data;
#ifdef GPS_REDARW

    snprintf(buff, sizeof(buff), "%.2f", gps->lat);
    lv_label_set_text(gps_data[0].label, buff);

    snprintf(buff, sizeof(buff), "%.2f", gps->lng);
    lv_label_set_text(gps_data[1].label, buff);

    snprintf(buff, sizeof(buff), "%u", gps->satellites);
    lv_label_set_text(gps_data[2].label, buff);

    snprintf(buff, sizeof(buff), "%u-%u-%u %u:%u:%u",
             gps->date.year,
             gps->date.month,
             gps->date.day,
             gps->date.hour,
             gps->date.min,
             gps->date.sec
            );
    lv_label_set_text(gps_data[3].label, buff);

    snprintf(buff, sizeof(buff), "%.2f", gps->altitude);
    lv_label_set_text(gps_data[4].label, buff);

    snprintf(buff, sizeof(buff), "%u", gps->course);
    lv_label_set_text(gps_data[5].label, buff);

    snprintf(buff, sizeof(buff), "%.2f", gps->speed);
    lv_label_set_text(gps_data[6].label, buff);
#else
#if defined(UBOX_GPS_MODULE)
    snprintf(buffer, sizeof(buffer), "lat:%.2f\nlng:%.2f\nsatellites:%u\ndate:%u-%u-%u\ntime:%u:%u:%u\naltitude:%.2f/m\nspeed:%.2f/kmph\n",
             gps->lat,
             gps->lng,
             gps->satellites,
             gps->date.year,
             gps->date.month,
             gps->date.day,
             gps->date.hour,
             gps->date.min,
             gps->date.sec,
             gps->altitude,
             gps->speed
            );
#elif defined(ACSIP_S7XG_MODULE)
    snprintf(buffer, sizeof(buffer), "lat:%.2f\nlng:%.2f\ndate:%u-%u-%u\ntime:%u:%u:%u\n",
             gps->lat,
             gps->lng,
             gps->date.year,
             gps->date.month,
             gps->date.day,
             gps->date.hour,
             gps->date.min,
             gps->date.sec
            );
#endif
    lv_label_set_text(gps_txt, buffer);

#endif
#endif
}


static lv_res_t lv_gps_static_text(lv_obj_t *par)
{
#ifdef GPS_REDARW
    lv_obj_t *label;
    lv_point_t size;
    lv_coord_t x = lv_obj_get_width(par) / 2 - 30;
    lv_coord_t y = 0;
    lv_coord_t u_offset = 17;
    lv_coord_t setup = 1;

    lv_obj_t *cont = lv_obj_create(par, NULL);
    lv_obj_set_size(cont,  g_menu_view_width, g_menu_view_height);
    lv_obj_set_style(cont, &lv_style_transp_fit);

    for (int i = 0; i < sizeof(gps_data) / sizeof(gps_data[0]); ++i) {
        label = lv_label_create(cont, NULL);
        lv_label_set_text(label, gps_data[i].name);
        size = lv_font_get_size(label);
        size.x = x - size.x;
        lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, size.x, u_offset * setup);

        gps_data[i].label = lv_label_create(cont, NULL);
        lv_label_set_text(gps_data[i].label, "N/A");
        lv_obj_align(gps_data[i].label, NULL, LV_ALIGN_IN_TOP_LEFT, lv_obj_get_width(cont) / 2, u_offset * setup);
        ++setup;
    }
#else

    /*Create anew style*/
    static lv_style_t style_txt;
    lv_style_copy(&style_txt, &lv_style_plain);
    style_txt.text.font = &lv_font_dejavu_20;
    style_txt.text.letter_space = 2;
    style_txt.text.line_space = 1;
    style_txt.text.color = LV_COLOR_WHITE;//LV_COLOR_HEX(0x606060);

    /*Create a new label*/
    gps_txt = lv_label_create(gContainer, NULL);
    lv_obj_set_style(gps_txt, &style_txt);                    /*Set the created style*/
    lv_label_set_long_mode(gps_txt, LV_LABEL_LONG_BREAK);     /*Break the long lines*/
    lv_label_set_recolor(gps_txt, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_align(gps_txt, LV_LABEL_ALIGN_CENTER);       /*Center aligned lines*/
    lv_label_set_text(gps_txt, "Align lines to the middle\n\n"
                      "#000080 Re-color# #0000ff words of# #6666ff the text#\n\n"
                      "If a line become too long it can be automatically broken into multiple lines");
    lv_obj_set_width(gps_txt, 240);                           /*Set a width*/
    lv_obj_align(gps_txt, NULL, LV_ALIGN_CENTER, 0, -20);      /*Align to center*/

#endif
    return LV_RES_OK;
}

void gps_anim_close()
{
    if (gContainer) {
        lv_obj_clean(gContainer);
    }
}

void gps_create_static_text()
{
    lv_gps_static_text(g_menu_win);
}

static lv_res_t lv_gps_anim_start(lv_obj_t *par)
{
    gContainer = lv_obj_create(par, NULL);
    lv_obj_set_size(gContainer, g_menu_view_width, g_menu_view_height);
    lv_obj_set_style(gContainer, &lv_style_transp_fit);

    lv_obj_t *img = lv_img_create(gContainer, NULL);
    lv_img_set_src(img, &image_location);
    lv_obj_align(img, NULL, LV_ALIGN_CENTER, 0, 0);
    static lv_anim_t a;
    a.var = img;
    a.start = lv_obj_get_y(img);
    a.end = lv_obj_get_y(img) - 10;
    a.fp = (lv_anim_fp_t)lv_obj_set_y;
    a.path = lv_anim_path_linear;
    a.end_cb = NULL;
    a.act_time = -1000;                         /*Negative number to set a delay*/
    a.time = 400;                               /*Animate in 400 ms*/
    a.playback = 1;                             /*Make the animation backward too when it's ready*/
    a.playback_pause = 0;                       /*Wait before playback*/
    a.repeat = 1;                               /*Repeat the animation*/
    a.repeat_pause = 100;                       /*Wait before repeat*/
    lv_anim_create(&a);
    return LV_RES_OK;
}

static void lv_gps_setting_destroy(void)
{
    lv_obj_del(gContainer);
    gContainer = NULL;
#ifdef ESP32
    task_event_data_t event_data;
    event_data.type = MESS_EVENT_GPS;
    event_data.gps.event = LVGL_GPS_STOP;
    xQueueSend(g_event_queue_handle, &event_data, portMAX_DELAY);
#endif
}

static lv_res_t lv_gps_setting(lv_obj_t *par)
{
#ifdef ESP32
    task_event_data_t event_data;
    event_data.type = MESS_EVENT_GPS;
    event_data.gps.event = LVGL_GPS_START;
    xQueueSend(g_event_queue_handle, &event_data, portMAX_DELAY);
#endif
    gps_anim_started = true;
    lv_gps_anim_start(par);
    return LV_RES_OK;
}

/*********************************************************************
 *
 *                          KEYBOARD
 *
 * ******************************************************************/
static const char *btnm_mapplus[][23] = {
    {
        "a", "b", "c",   "\n",
        "d", "e", "f",   "\n",
        "g", "h", "i",   "\n",
        "\202"SYMBOL_OK, "Del", "Exit", "\202"SYMBOL_RIGHT, ""
    },
    {
        "j", "k", "l", "\n",
        "n", "m", "o",  "\n",
        "p", "q", "r",  "\n",
        "\202"SYMBOL_OK, "Del", "Exit", "\202"SYMBOL_RIGHT, ""
    },
    {
        "s", "t", "u",   "\n",
        "v", "w", "x", "\n",
        "y", "z", "\n",
        "\202"SYMBOL_OK, "Del", "Exit", "\202"SYMBOL_RIGHT, ""
    },
    {
        "A", "B", "C",  "\n",
        "D", "E", "F",   "\n",
        "G", "H", "I",  "\n",
        "\202"SYMBOL_OK, "Del", "Exit", "\202"SYMBOL_RIGHT, ""
    },
    {
        "J", "K", "L", "\n",
        "N", "M", "O",  "\n",
        "P", "Q", "R", "\n",
        "\202"SYMBOL_OK, "Del", "Exit", "\202"SYMBOL_RIGHT, ""
    },
    {
        "S", "T", "U",   "\n",
        "V", "W", "X",   "\n",
        "Y", "Z", "\n",
        "\202"SYMBOL_OK, "Del", "Exit", "\202"SYMBOL_RIGHT, ""
    },
    {
        "1", "2", "3",  "\n",
        "4", "5", "6",  "\n",
        "7", "8", "9",  "\n",
        "\202"SYMBOL_OK, "Del", "Exit", "\202"SYMBOL_RIGHT, ""
    },
    {
        "0", "+", "-",  "\n",
        "/", "*", "=",  "\n",
        "!", "?", "#",  "\n",
        "\202"SYMBOL_OK, "Del", "Exit", "\202"SYMBOL_RIGHT, ""
    },
    {
        "<", ">", "@",  "\n",
        "%", "$", "(",  "\n",
        ")", "{", "}",  "\n",
        "\202"SYMBOL_OK, "Del", "Exit", "\202"SYMBOL_RIGHT, ""
    },
    {
        "[", "]", ";",  "\n",
        "\"", "'", ".", "\n",
        ",", ":",  "\n",
        "\202"SYMBOL_OK, "Del", "Exit", "\202"SYMBOL_RIGHT, ""
    }
};

static lv_obj_t *passkeyboard = NULL;
static lv_obj_t *keyboard_cont = NULL;
static lv_obj_t *pass = NULL;
static lv_obj_t *wifi_connect_label = NULL;

static lv_res_t btnm_action(lv_obj_t *btnm, const char *txt)
{
    static int index = 0;
    lv_kb_ext_t *ext = lv_obj_get_ext_attr(passkeyboard);

    if (strcmp(txt, SYMBOL_OK) == 0) {
        lv_obj_del(keyboard_cont);
        lv_connect_wifi(lv_ta_get_text(ext->ta));
    } else if (strcmp(txt, "Exit") == 0) {
        lv_obj_del(keyboard_cont);
        lv_obj_set_hidden(g_menu_win, false);
    } else if (strcmp(txt, SYMBOL_RIGHT) == 0) {
        index = index + 1 >= sizeof(btnm_mapplus) / sizeof(btnm_mapplus[0]) ? 0 : index + 1;
        lv_kb_set_map(passkeyboard, btnm_mapplus[index]);
    } else if (strcmp(txt, SYMBOL_LEFT) == 0) {
        index = index - 1 >= 0  ? index - 1 : sizeof(btnm_mapplus) / sizeof(btnm_mapplus[0]) - 1;
        lv_kb_set_map(passkeyboard, btnm_mapplus[index]);
    } else if (strcmp(txt, "Del") == 0)
        lv_ta_del_char(ext->ta);
    else {
        lv_ta_add_text(ext->ta, txt);
    }
    return LV_RES_OK; /*Return OK because the button matrix is not deleted*/
}

static void create_keyboard()
{
    lv_obj_set_hidden(g_menu_win, true);
    keyboard_cont = lv_cont_create(menu_cont, NULL);
    lv_obj_set_size(keyboard_cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style(keyboard_cont, &lv_style_transp_fit);

    pass = lv_ta_create(keyboard_cont, NULL);
    lv_obj_set_height(pass, 35);
    lv_ta_set_one_line(pass, true);
    lv_ta_set_pwd_mode(pass, true);
    lv_ta_set_text(pass, "");
    lv_obj_align(pass, keyboard_cont, LV_ALIGN_IN_TOP_MID, 0, 30);

    passkeyboard = lv_kb_create(keyboard_cont, NULL);
    lv_kb_set_map(passkeyboard, btnm_mapplus[0]);
    lv_obj_set_height(passkeyboard, LV_VER_RES / 3 * 2);
    lv_obj_align(passkeyboard, keyboard_cont, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    lv_kb_set_ta(passkeyboard, pass);
    lv_btnm_set_action(passkeyboard, btnm_action);
}

static void lv_connect_wifi(const char *password)
{
    lv_obj_set_hidden(g_menu_win, false);
    lv_obj_clean(gContainer);

    wifi_connect_label = lv_label_create(gContainer, NULL);
    lv_label_set_text(wifi_connect_label, "Connecting...");
    lv_obj_align(wifi_connect_label, gContainer, LV_ALIGN_CENTER, 0, 0);

#ifdef ESP32
    strlcpy(auth.password, password, sizeof(auth.password));
    task_event_data_t event_data;
    event_data.type = MESS_EVENT_WIFI;
    event_data.wifi.event = LVGL_WIFI_CONFIG_TRY_CONNECT;
    event_data.wifi.ctx = &auth;
    printf("ssid:%s password:%s \n", auth.ssid, auth.password);
    xQueueSend(g_event_queue_handle, &event_data, portMAX_DELAY);
#endif
}

static void lv_refs_wifi(void *param)
{
    lv_obj_set_hidden(g_menu_win, false);
    lv_obj_del(gContainer);
    gContainer = NULL;
    gObjecter = NULL;
    lv_wifi_setting(g_menu_win);
}

void lv_wifi_connect_fail()
{
    lv_label_set_text(wifi_connect_label, "Connect FAIL");
    lv_obj_align(wifi_connect_label, gContainer, LV_ALIGN_CENTER, 0, -10);
    lv_timer_start(lv_refs_wifi, 800, NULL);
}

void lv_wifi_connect_pass()
{
    lv_label_set_text(wifi_connect_label, "Connect PASS");
    lv_obj_align(wifi_connect_label, gContainer, LV_ALIGN_CENTER, 0, -10);
    lv_timer_start(lv_refs_wifi, 800, NULL);
}

/*********************************************************************
 *
 *                          WIFI
 *
 * ******************************************************************/
static void lv_wifi_setting_destroy()
{
    lv_obj_del(gContainer);
    gContainer = NULL;
    gObjecter = NULL;
}

static lv_res_t wifiap_list_action(lv_obj_t *obj)
{
    const char *ssid = lv_list_get_btn_text(obj);
#ifdef ESP32
    strlcpy(auth.ssid, ssid, sizeof(auth.ssid));
    printf("auth ssid:%s\n", auth.ssid);
#else
    printf("auth ssid:%s\n", ssid);
#endif
    create_keyboard();
    return LV_RES_OK;
}


uint8_t lv_wifi_list_add(const char *ssid, int32_t rssi, uint8_t ch)
{
    if (!gObjecter) {
        lv_setWinBtnInvaild(true);
        lv_obj_clean(gContainer);
        gObjecter = lv_list_create(gContainer, NULL);
        lv_obj_set_size(gObjecter,  g_menu_view_width, g_menu_view_height);
        lv_obj_align(gObjecter, gContainer, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style(gObjecter, &lv_style_transp_fit);

    }
    lv_list_add(gObjecter, SYMBOL_WIFI, ssid, wifiap_list_action);
    return LV_RES_OK;
}

static lv_res_t wifi_scan_btn_cb(struct _lv_obj_t *obj)
{
#ifdef ESP32
    lv_setWinBtnInvaild(false);
    task_event_data_t event_data;
    event_data.type = MESS_EVENT_WIFI;
    event_data.wifi.event = LVGL_WIFI_CONFIG_SCAN;
    xQueueSend(g_event_queue_handle, &event_data, portMAX_DELAY);
    lv_obj_clean(gContainer);
    lv_obj_t *label = lv_label_create(gContainer, NULL);
    lv_label_set_text(label, "Scaning...");
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, -10);
#else
    const char *ssid[] = {"A", "B"};
    for (int i = 0; i < 2; i++) {
        lv_wifi_list_add(ssid[i], 0, 0);
    }
#endif
    return LV_RES_OK;
}

static lv_res_t lv_wifi_setting(lv_obj_t *par)
{
    lv_obj_t *label = NULL;
    gContainer = lv_obj_create(par, NULL);
    lv_obj_set_size(gContainer,  g_menu_view_width, g_menu_view_height);
    lv_obj_set_style(gContainer, &lv_style_transp_fit);
    for (int i = 0; i < sizeof(wifi_data) / sizeof(wifi_data[0]); ++i) {

        wifi_data[i].label = lv_label_create(gContainer, NULL);
        snprintf(buff, sizeof(buff), "%s:%s", wifi_data[i].name, wifi_data[i].get_val());
        lv_label_set_text(wifi_data[i].label, buff);
        if (!i)
            lv_obj_align(wifi_data[i].label, gContainer, LV_ALIGN_IN_TOP_MID, 0, 0);
        else
            lv_obj_align(wifi_data[i].label, wifi_data[i - 1].label, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    }

    lv_obj_t *scanbtn = lv_btn_create(gContainer, NULL);
    lv_obj_align(scanbtn, gContainer, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    lv_obj_set_size(scanbtn, 100, 25);
    label = lv_label_create(scanbtn, NULL);
    lv_label_set_text(label, "Scan");
    lv_btn_set_action(scanbtn, LV_BTN_ACTION_PR, wifi_scan_btn_cb);
    return LV_RES_OK;
}



/*********************************************************************
 *
 *                          FILE
 *
 * ******************************************************************/

static void lv_file_setting_destroy(void)
{
    lv_obj_del(gContainer);
    gContainer = NULL;
    gObjecter = NULL;
}

void lv_file_list_add(const char *filename, uint8_t type)
{
    if (!filename) {
        lv_obj_t *label = lv_label_create(gContainer, NULL);
        lv_label_set_text(label, "SD Card not found");
        lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
    } else {
        if (!gObjecter) {
            gObjecter = lv_list_create(gContainer, NULL);
            lv_obj_set_size(gObjecter,  g_menu_view_width, g_menu_view_height);
            lv_obj_align(gObjecter, NULL, LV_ALIGN_CENTER, 0, 0);
            lv_style_t *style = lv_list_get_style(gObjecter, LV_LIST_STYLE_BTN_REL);
            style->body.padding.ver = 10;
            lv_list_set_style(gObjecter, LV_LIST_STYLE_BTN_REL, style);
        }
        if (gObjecter) {
            if (type)
                lv_list_add(gObjecter, SYMBOL_DIRECTORY, filename, NULL);
            else
                lv_list_add(gObjecter, SYMBOL_FILE, filename, NULL);
        }
    }
}

static lv_res_t lv_file_setting(lv_obj_t *par)
{
    gContainer = lv_cont_create(par, NULL);
    lv_obj_set_style(gContainer, &lv_style_transp_fit);
    lv_obj_set_size(gContainer,  g_menu_view_width, g_menu_view_height);
#ifdef ESP32
    task_event_data_t event_data;
    event_data.type = MESS_EVENT_FILE;
    event_data.file.event = LVGL_FILE_SCAN;
    xQueueSend(g_event_queue_handle, &event_data, portMAX_DELAY);
#else
    const char *buff[] = {"1.jpg", "2.png", "mono"};
    for (int i = 0; i < sizeof(buff) / sizeof(buff[0]); ++i) {
        lv_file_list_add(buff[i], 0);
    }
#endif
    return LV_RES_OK;
}


/*********************************************************************
 *
 *                          SETTING
 *
 * ******************************************************************/
static lv_res_t lv_setting(lv_obj_t *par)
{
    printf("Create lv_setting\n");
    //! backlight level
    gContainer = lv_cont_create(par, NULL);
    lv_obj_set_size(gContainer,  g_menu_view_width, g_menu_view_height);
    lv_obj_set_style(gContainer, &lv_style_transp_fit);


    lv_obj_t *label;
    label = lv_label_create(gContainer, NULL);
    lv_label_set_text(label, "BL:");
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 40);

    static const char *btnm_str[] = {"1", "2", "3", ""};
    lv_obj_t *btnm = lv_btnm_create(gContainer, NULL);
    lv_obj_set_size(btnm, 150, 50);
    lv_btnm_set_map(btnm, btnm_str);
    lv_obj_align(btnm, label, LV_ALIGN_OUT_RIGHT_MID, 30, 0);
    lv_btnm_set_toggle(btnm, true, 3);
    lv_btnm_set_action(btnm, lv_setting_backlight_action);

    label = lv_label_create(gContainer, NULL);
    lv_label_set_text(label, "TH:");
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 100);

    lv_obj_t *ddlist = lv_ddlist_create(gContainer, NULL);
    lv_obj_align(ddlist, label, LV_ALIGN_OUT_RIGHT_MID, 30, 0);
    lv_ddlist_set_options(ddlist, "Alien\nNight\nMono\nNemo\nMaterial");
    lv_ddlist_set_fix_height(ddlist, LV_DPI);
    lv_ddlist_close(ddlist, true);
    lv_ddlist_set_selected(ddlist, 1);
    lv_ddlist_set_hor_fit(ddlist, false);
    lv_obj_set_width(ddlist,  150);
    lv_ddlist_set_action(ddlist, lv_setting_th_action);
    return LV_RES_OK;
}

static void lv_setting_destroy(void)
{
    lv_obj_del(gContainer);
    gContainer = NULL;
    gObjecter = NULL;
    printf("lv_setting_destroy\n");
}
/*********************************************************************
 *
 *                          MENU
 *
 * ******************************************************************/

static void lv_menu_del()
{
    if (curr_index != -1) {
        if (menu_data[curr_index].destroy != NULL) {
            menu_data[curr_index].destroy();
        }
        curr_index = -1;
    }
    lv_obj_del(menu_cont);
    g_menu_in = true;
    prev = -1;
}

static lv_res_t win_btn_click(lv_obj_t *btn)
{
    lv_menu_del();
    return LV_RES_OK;
}


void create_menu(lv_obj_t *par)
{
    lv_obj_t *label;
    lv_obj_t *img;

    if (!par) {
        par = lv_scr_act();
    }

    static lv_style_t style_txt;
    lv_style_copy(&style_txt, &lv_style_plain);
    style_txt.text.font = &lv_font_dejavu_20;
    style_txt.text.color = LV_COLOR_WHITE;
    style_txt.image.color = LV_COLOR_YELLOW;
    style_txt.body.main_color = LV_COLOR_GRAY;
    style_txt.body.border.color = LV_COLOR_GRAY;
    style_txt.body.opa = LV_OPA_10;

    menu_cont = lv_obj_create(par, NULL);
    lv_obj_set_size(menu_cont, lv_obj_get_width(par), lv_obj_get_height(par));

    lv_obj_t *wp = lv_img_create(menu_cont, NULL);
    lv_img_set_src(wp, &img_desktop2);
    lv_obj_set_width(wp, LV_HOR_RES);
    lv_obj_set_protect(wp, LV_PROTECT_POS);

    g_menu_win = lv_win_create(menu_cont, NULL);
    lv_win_set_title(g_menu_win, menu_data[0].name);
    lv_win_set_sb_mode(g_menu_win, LV_SB_MODE_OFF);
    lv_win_set_layout(g_menu_win, LV_LAYOUT_PRETTY);

    static lv_win_ext_t *ext1 ;
    ext1 = lv_obj_get_ext_attr(g_menu_win);
    lv_coord_t height =  lv_obj_get_height(ext1->header);
    lv_obj_t *win_btn = lv_win_add_btn(g_menu_win, SYMBOL_HOME, win_btn_click);
    lv_win_set_btn_size(g_menu_win, 45);

    lv_win_set_style(g_menu_win, LV_WIN_STYLE_HEADER, &style_txt);
    lv_win_set_style(g_menu_win, LV_WIN_STYLE_BG, &style_txt);



#ifdef UBOX_GPS_MODULE
#define GPS_EN  1
#else
#define GPS_EN   0
#endif

#ifdef ACSIP_S7XG_MODULE
#define S7XG_EN  1
#else
#define S7XG_EN   0
#endif

#ifdef ENABLE_BLE
#define BLE_EN  1
#else
#define BLE_EN   0
#endif

    static const lv_point_t vp[] = {
#if     ((BLE_EN) && ((!GPS_EN) && (!S7XG_EN)))
        {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0},
#elif   ((BLE_EN) && (GPS_EN))
        {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0},
#elif   ((BLE_EN) && (S7XG_EN))
        {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0},
#elif   (S7XG_EN)
        {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0},
#elif  (GPS_EN)
        {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0},
#else
        {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0},
#endif
        {LV_COORD_MIN, LV_COORD_MIN}
    };

    g_tileview = lv_tileview_create(g_menu_win, NULL);
    lv_tileview_set_valid_positions(g_tileview, vp);

    lv_tileview_ext_t *ext = lv_obj_get_ext_attr(g_tileview);
    ext->anim_time = 50;
    ext->action = lv_tileview_action;
    lv_page_set_sb_mode(g_tileview, LV_SB_MODE_OFF);

    g_menu_view_width = lv_obj_get_width(g_menu_win) - 20;
    g_menu_view_height = lv_obj_get_height(g_menu_win) - height - 10;
    lv_obj_set_size(g_tileview, g_menu_view_width, g_menu_view_height);

    lv_obj_t *prev_obj = NULL;
    lv_obj_t *cur_obj = NULL;

    for (int i = 0; i < sizeof(menu_data) / sizeof(menu_data[0]); ++i) {
        cur_obj = lv_obj_create(g_tileview, NULL);
        lv_obj_set_size(cur_obj, lv_obj_get_width(g_tileview), lv_obj_get_height(g_tileview));
        lv_obj_set_style(cur_obj, &lv_style_transp_fit);
        lv_obj_align(cur_obj, prev_obj, LV_ALIGN_OUT_RIGHT_MID, 0, 0);

        lv_tileview_add_element(cur_obj);

        img = lv_img_create(cur_obj, NULL);
        lv_img_set_src(img, menu_data[i].src_img);
        lv_obj_align(img, NULL, LV_ALIGN_CENTER, 0, -20);


        if (prev_obj != cur_obj) {
            prev_obj = cur_obj;
        }
    }
    return;
}

bool lv_main_in()
{
    return g_menu_in;
}

void lv_main_step_counter_update(const char *step)
{
    lv_label_set_text(main_data.step_count_label, step);
}

void lv_main_time_update(const char *time, const char *date)
{
    lv_label_set_text(main_data.time_label, time);
    lv_obj_align(main_data.time_label, NULL, LV_ALIGN_CENTER, 0, -80);
    lv_label_set_text(main_data.date_label, date);
    lv_obj_align(main_data.date_label, main_data.time_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
}

void lv_create_ttgo()
{
    if (!main_cont) {
        main_cont = lv_cont_create(lv_scr_act(), NULL);
        lv_obj_set_size(main_cont, LV_HOR_RES, LV_VER_RES);
    } else
        lv_obj_clean(main_cont);

    static lv_style_t style;
    lv_style_copy(&style, &lv_style_scr);
    style.body.main_color = LV_COLOR_BLACK;
    style.body.grad_color = LV_COLOR_BLACK;
    lv_obj_set_style(main_cont, &style);

    lv_obj_t *img = lv_img_create(main_cont, NULL);
    lv_img_set_src(img, &img_bg0);
    lv_img_set_style(img, &style);
    lv_obj_align(img, main_cont, LV_ALIGN_CENTER, 0, 0);
}


void lv_main(void)
{
    g_menu_in = true;

    lv_theme_set_current(lv_theme_material_init(100, NULL));

    lv_font_add(&font_miami, &lv_font_dejavu_20);
    lv_font_add(&font_miami_32, &lv_font_dejavu_20);
    lv_font_add(&font_sumptuous, &lv_font_dejavu_20);
    lv_font_add(&font_sumptuous_24, &lv_font_dejavu_20);


    if (!main_cont) {
        main_cont = lv_cont_create(lv_scr_act(), NULL);
        lv_obj_set_size(main_cont, LV_HOR_RES, LV_VER_RES);
    } else
        lv_obj_clean(main_cont);

    lv_obj_t *wp = lv_img_create(main_cont, NULL);
    lv_img_set_src(wp, &img_bg1);
    lv_obj_set_width(wp, LV_HOR_RES);
    lv_obj_set_protect(wp, LV_PROTECT_POS);

    static lv_style_t style1;
    lv_style_copy(&style1, &lv_style_plain);
    style1.text.font = &font_miami;
    style1.text.color = LV_COLOR_WHITE;

    main_data.time_label = lv_label_create(main_cont, NULL);
    lv_label_set_text(main_data.time_label, "08:30");
    lv_label_set_style(main_data.time_label, &style1);
    lv_obj_align(main_data.time_label, NULL, LV_ALIGN_CENTER, 0, -80);


    static lv_style_t style11;
    lv_style_copy(&style11, &lv_style_plain);
    style11.text.font = &font_sumptuous;
    style11.text.color = LV_COLOR_WHITE;

    main_data.date_label = lv_label_create(main_cont, NULL);
    lv_label_set_text(main_data.date_label, "Wed 8 OCT");
    lv_label_set_style(main_data.date_label, &style11);
    lv_obj_align(main_data.date_label, main_data.time_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);


    static lv_style_t style2;
    lv_style_copy(&style2, &lv_style_plain);
    style2.text.font = &font_sumptuous_24;
    style2.text.color = LV_COLOR_WHITE;

    lv_obj_t *img = lv_img_create(main_cont, NULL);
    lv_img_set_src(img, &img_step_conut);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_LEFT, 15, 120);

    main_data.step_count_label = lv_label_create(main_cont, NULL);
    lv_label_set_text(main_data.step_count_label, "0");
    lv_label_set_style(main_data.step_count_label, &style2);
    lv_obj_align(main_data.step_count_label, img, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    img_batt = lv_img_create(main_cont, NULL);
    lv_img_set_src(img_batt, lv_get_batt_icon());
    lv_obj_align(img_batt, main_cont, LV_ALIGN_IN_TOP_RIGHT, -10, 5);


    static lv_style_t style_pr;
    lv_style_copy(&style_pr, &lv_style_plain);
    style_pr.image.color = LV_COLOR_WHITE;
    style_pr.image.intense = LV_OPA_100;
    style_pr.text.color = LV_COLOR_HEX3(0xaaa);

    lv_obj_t *menuBtn = lv_imgbtn_create(main_cont, NULL);
    lv_imgbtn_set_src(menuBtn, LV_BTN_STATE_REL, &img_menu);
    lv_imgbtn_set_src(menuBtn, LV_BTN_STATE_PR, &img_menu);
    lv_imgbtn_set_src(menuBtn, LV_BTN_STATE_TGL_REL, &img_menu);
    lv_imgbtn_set_src(menuBtn, LV_BTN_STATE_TGL_PR, &img_menu);
    lv_imgbtn_set_style(menuBtn, LV_BTN_STATE_PR, &style_pr);
    lv_imgbtn_set_style(menuBtn, LV_BTN_STATE_TGL_PR, &style_pr);
    lv_imgbtn_set_toggle(menuBtn, true);
    lv_imgbtn_set_action(menuBtn, LV_BTN_ACTION_PR, menubtn_action);
    lv_obj_align(menuBtn, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -30);

}


/**********************
 *   STATIC FUNCTIONS
 **********************/
static lv_res_t lv_tileview_action(lv_obj_t *obj, lv_coord_t x, lv_coord_t y)
{
    if (x == prev) {
        g_menu_in = false;
        lv_obj_set_hidden(obj, true);
        menu_data[x].callback(g_menu_win);
        curr_index = x;
    } else {
        lv_win_set_title(g_menu_win, menu_data[x].name);
        prev = x;
    }
    return LV_RES_OK;
}


static lv_res_t menubtn_action(lv_obj_t *btn)
{
    create_menu(main_cont);
    return LV_RES_OK;
}

static lv_res_t lv_setting_backlight_action(lv_obj_t *obj, const char *txt)
{
#ifdef ESP32
    backlight_setting(atoi(txt));
#endif
    return LV_RES_OK;
}


static lv_res_t lv_setting_th_action(lv_obj_t *obj)
{
    char buf[64];
    lv_ddlist_get_selected_str(obj, buf);
    printf("New option selected on a drop down list: %s\n", buf);

    const char *theme_options[] = {
        "Alien", "Night", "Mono", "Nemo", "Material"
    };

    int option = -1;

    for (int i = 0; i < sizeof(theme_options) / sizeof(theme_options[0]); ++i) {
        if (!strcmp(buf, theme_options[i])) {
            option = i;
            break;
        }
    }
    switch (option) {
    case 0:
#if USE_LV_THEME_ALIEN
        lv_theme_set_current(lv_theme_alien_init(100, NULL));
#endif
        break;
    case 1:
#if USE_LV_THEME_NIGHT
        lv_theme_set_current(lv_theme_night_init(100, NULL));
#endif
        break;
    case 2:
#if USE_LV_THEME_ZEN
        lv_theme_set_current(lv_theme_zen_init(100, NULL));
#endif
        break;
    case 3:
#if USE_LV_THEME_MONO
        lv_theme_set_current(lv_theme_mono_init(100, NULL));
#endif
        break;
    case 4:
#if USE_LV_THEME_NEMO
        lv_theme_set_current(lv_theme_nemo_init(100, NULL));
#endif
        break;
    case 5:
#if USE_LV_THEME_MATERIAL
        lv_theme_set_current(lv_theme_material_init(100, NULL));
#endif
        break;
    default:
        break;
    }
    return LV_RES_OK;
}
