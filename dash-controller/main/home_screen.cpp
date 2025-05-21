/**
 * @file lv_demo_widgets.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

extern "C" {
#include "lvgl.h"
#include "esp_log.h"
#include "esp_err.h"
}

#include "ta_json.hpp"

//#include <stdio.h>
//#include <string.h>

/*********************
 *      DEFINES
 *********************/

static const char *TAG = "home_screen";

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    DISP_SMALL,
    DISP_MEDIUM,
    DISP_LARGE,
} disp_size_t;

namespace speedo {
    lv_obj_t * speed_label;
    lv_obj_t * units_label;
    lv_obj_t * meter;
    lv_meter_scale_t * scale;
    lv_meter_indicator_t * indic_needle;
    lv_meter_indicator_t * indic_arc;
    lv_meter_indicator_t * indic_scale_lines;
    lv_meter_indicator_t * indic_arc_high;
    lv_meter_indicator_t * indic_scale_lines_high;

    void update();
    void set_value(int32_t v);
};

namespace hvac {
    void fan_speed_event_cb(lv_event_t *e);
    void ac_toggle_event_cb(lv_event_t *e);
    void temp_slider_event_cb(lv_event_t *e);
    void zone_slider_event_cb(lv_event_t *e);
};

namespace relays
{
    void send_json(uint8_t relay_num, const String & cmd);
    void close(uint8_t relay_num);

    void btn_event_cb(lv_event_t * e);
}

namespace status_inputs
{
    class status_input_t {
    public:
        String onLabel;
        String offLabel;
        lv_obj_t * status;
        lv_obj_t * status_label;

        void set(bool on = true);
    };

    static const uint8_t input_cnt = 4;
    status_input_t status_input[input_cnt];

}

namespace servos
{
    class servo_t {
    public:
        lv_obj_t * open_label;
        lv_obj_t * close_label;
        lv_obj_t * current_label;

        uint16_t open_pulse = 400;
        uint16_t close_pulse = 600;
        uint16_t current_pulse = 500;

        void adjust(int16_t rel_adjust);

        void open();
        void close();
        void set(uint8_t position);         // position is percentage open
        void set_abs(uint16_t position);    // position is absolute position between 0 and 1000

        void get_json(Buffer & buffer);
        void get_pulse_json(uint8_t servo_num, Buffer & buffer);

        void update();
    };

    static const uint8_t servo_cnt = 4;
    servo_t servo[servo_cnt];

    void get_json(Buffer & buffer);
    void send_pulse_json(uint8_t servo_num);

    void update();

    void btn_event_cb(lv_event_t * e);
}

namespace config {
    typedef enum {
        UNITS_METRIC,
        UNITS_BRITISH,
    } units_t;

    units_t units = UNITS_BRITISH;
    lv_palette_t palette;

    void store();
    void load();

    void units_event_cb(lv_event_t * e);
}

/**********************
 *  STATIC PROTOTYPES
 **********************/
// static void accessories_create(lv_obj_t * parent);
static void main_create(lv_obj_t * parent);
static void settings_create(lv_obj_t * parent);
static void color_changer_create(lv_obj_t * parent);

//static lv_obj_t * create_meter_box(lv_obj_t * parent, const char * title, const char * text1, const char * text2,
//                                   const char * text3);

static void color_changer_event_cb(lv_event_t * e);
static void color_event_cb(lv_event_t * e);
// static void ta_event_cb(lv_event_t * e);
/*
static void birthday_event_cb(lv_event_t * e);
static void calendar_event_cb(lv_event_t * e);
static void slider_event_cb(lv_event_t * e);
static void chart_event_cb(lv_event_t * e);
*/

extern "C" {
esp_err_t write_file(const char * filename, const char * data, int datalen);
int read_file(const char * filename, char * buffer, int bufferlen);
esp_err_t i2c_master_send_data(uint8_t *data, size_t len);

extern const lv_img_dsc_t temperature_bar_lg;
extern const lv_img_dsc_t HVAC_icons_bar;
}

/**********************
 *  STATIC VARIABLES
 **********************/
static disp_size_t disp_size;

static lv_obj_t * tv;
// static lv_obj_t * calendar;
static lv_style_t style_text_muted;
static lv_style_t style_title;
static lv_style_t style_xl;
static lv_style_t style_icon;
static lv_style_t style_bullet;
static lv_style_t style_indicators;

// static lv_obj_t * meter1;
// static lv_obj_t * meter2;
// static lv_obj_t * speedo;
/*
static lv_obj_t * chart1;
static lv_obj_t * chart2;
static lv_obj_t * chart3;
*/

/*
static lv_chart_series_t * ser1;
static lv_chart_series_t * ser2;
static lv_chart_series_t * ser3;
static lv_chart_series_t * ser4;
*/

static const lv_font_t * font_large;
static const lv_font_t * font_normal;

/*
static uint32_t session_desktop = 1000;
static uint32_t session_tablet = 1000;
static uint32_t session_mobile = 1000;
*/

// static lv_timer_t * meter2_timer;

// static speedo_t speedo;
//  static config config;

static const uint16_t max_json_size = 1024;
static Queue<JToken> json_tokens(64);
static JSON json(json_tokens);
static Buffer json_buff(max_json_size);

/**********************
 *      MACROS
 **********************/

#define UNIT_PICK(BRITISH, METRIC) (config::units == config::UNITS_BRITISH ? BRITISH : METRIC)

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

static esp_err_t write_file(const String & filename, const String & data)
{
    return write_file(filename.Pch(), data.Pch(), data.Len());
}

static esp_err_t read_file(const String & filename, Buffer & buffer)
{
    int bytes_read;

    buffer.Reset();
    bytes_read = read_file(filename.Pch(), buffer.Pch(), buffer.MaxLen() - 1);
    if (bytes_read > 0) {
        buffer.SetLenToSzLen();
        return ESP_OK;
    }
    return ESP_FAIL;
}

void speedo::update()
{
    lv_meter_set_scale_range(meter, scale, 0, UNIT_PICK(90, 150), 300, 120);
    lv_meter_set_scale_ticks(meter, scale, UNIT_PICK(25, 31), 3, 17, lv_color_white());
    lv_meter_set_scale_major_ticks(meter, scale, UNIT_PICK(4, 6), 4, 22, lv_color_white(), 15);

    lv_meter_set_indicator_start_value(meter, indic_arc, 0);
    lv_meter_set_indicator_end_value(meter, indic_arc, UNIT_PICK(80, 130));

    lv_meter_set_indicator_start_value(meter, indic_scale_lines, 0);
    lv_meter_set_indicator_end_value(meter, indic_scale_lines, UNIT_PICK(80, 130));

    lv_meter_set_indicator_start_value(meter, indic_arc_high, UNIT_PICK(80, 130));
    lv_meter_set_indicator_end_value(meter, indic_arc_high, UNIT_PICK(90, 150));

    lv_meter_set_indicator_start_value(meter, indic_scale_lines_high, UNIT_PICK(80, 130));
    lv_meter_set_indicator_end_value(meter, indic_scale_lines_high, UNIT_PICK(90, 150));

    lv_label_set_text(units_label, UNIT_PICK("M/H", "K/H"));
    lv_obj_center(units_label);
}

void speedo::set_value(int32_t v)
{
    if (!meter || !indic_needle) {
        return;
    }
    lv_meter_set_indicator_value(meter, indic_needle, v);
    lv_label_set_text_fmt(speed_label, "%d", (int)v);
}

void hvac::fan_speed_event_cb(lv_event_t *e) {
    lv_obj_t *btnm = lv_event_get_target(e);
    uint16_t id = lv_btnmatrix_get_selected_btn(btnm);
    const char *txt = lv_btnmatrix_get_btn_text(btnm, id);
    printf("Fan speed selected: %s\n", txt);
}

void hvac::ac_toggle_event_cb(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);
    /*
    const char *txt = lv_btn_get_text(btn);
    if (strcmp(txt, "A/C ON") == 0) {
        lv_btn_set_text(btn, "A/C OFF");
        printf("A/C turned off\n");
    } else {
        lv_btn_set_text(btn, "A/C ON");
        printf("A/C turned on\n");
    }
    */
}

void hvac::temp_slider_event_cb(lv_event_t *e) {
    lv_obj_t *slider = lv_event_get_target(e);
    int value = lv_slider_get_value(slider);
    printf("Temperature set to: %d°C\n", value);
}

void hvac::zone_slider_event_cb(lv_event_t *e) {
    lv_obj_t *slider = lv_event_get_target(e);
    int value = lv_slider_get_value(slider);
    printf("Airflow zone set to: %d (Feet=0, Defrost=100)\n", value);
}

void relays::send_json(uint8_t relay_num, const String & cmd)
{
    json_buff = "{\"relay\":{\"num\":";
    json_buff.StrFromInt(relay_num);
    json_buff += ",\"state\":\"";
    json_buff += cmd;
    json_buff += "\"}}\n\0";

    i2c_master_send_data((uint8_t *)json_buff.Pch(), json_buff.Len());    
}

void relays::btn_event_cb(lv_event_t * e)
{
    char * data = (char *)e->user_data;
    uint8_t relay_num = data[0] - '0';

    switch (e->code) {
        case LV_EVENT_CLICKED: {
            send_json(relay_num, data[2] == 'n' ? "on" : "off");
            break;
        }
        default: {
            break;
        }
    }
}

void status_inputs::status_input_t::set(bool on)
{
    lv_obj_set_style_bg_color(status, lv_palette_darken(on ? LV_PALETTE_RED : LV_PALETTE_GREEN, 4), 0);
    lv_label_set_text(status_label, on ? onLabel.Pch() : offLabel.Pch());
}

void servos::servo_t::open()
{
    
}

void servos::servo_t::close()
{
    
}

void servos::servo_t::set(uint8_t position)
{
    
}

void servos::servo_t::set_abs(uint16_t position)
{
    
}

void servos::servo_t::get_json(Buffer & buffer)
{
    buffer += "{\"open_pulse\":";
    buffer.StrFromInt(open_pulse);
    buffer += ",\"close_pulse\":";
    buffer.StrFromInt(close_pulse);
    buffer += '}';
}

void servos::servo_t::get_pulse_json(uint8_t servo_num, Buffer & buffer)
{
    buffer += "{\"num\":";
    buffer.StrFromInt(servo_num + 12);  // using servos 12 to 15 as 0 to 4
    buffer += ",\"pulse\":";
    buffer.StrFromInt(current_pulse);
    buffer += '}';
}

void servos::get_json(Buffer & buffer)
{
    buffer += "\"servos\":[";
    for (uint8_t i = 0; i < servo_cnt; i++) {
        if (i > 0) {
            buffer += ',';
        }
        servo[i].get_json(buffer);
    }
    buffer += "]";
}

void servos::send_pulse_json(uint8_t servo_num)
{
    json_buff = "{\"servo\":";
    servo[servo_num].get_pulse_json(servo_num, json_buff);
    json_buff += "}\n\0";

    i2c_master_send_data((uint8_t *)json_buff.Pch(), json_buff.Len());
};

void servos::servo_t::update()
{
    char temp[128];
    Buffer temp_buffer(temp, sizeof(temp));

    temp_buffer = "Curr: ";
    temp_buffer.StrFromInt(current_pulse);
    lv_label_set_text(current_label, temp_buffer.Pch());

    temp_buffer = "Open: ";
    temp_buffer.StrFromInt(open_pulse);
    lv_label_set_text(open_label, temp_buffer.Pch());

    temp_buffer = "Close: ";
    temp_buffer.StrFromInt(close_pulse);
    lv_label_set_text(close_label, temp_buffer.Pch());
}

void servos::servo_t::adjust(int16_t rel_adjust)
{
    if (current_pulse + rel_adjust >= 1000) {
        current_pulse = 1000;
    } 
    else if ((int16_t)current_pulse + rel_adjust <= 0) {
        current_pulse = 0;
    }
    else {
        current_pulse += rel_adjust;
    }
    update();
}

void servos::btn_event_cb(lv_event_t * e)
{
    static bool press_hold = false;
    char * data = (char *)e->user_data;
    uint8_t servo_num = data[0] - '0';

    switch (e->code) {
        case LV_EVENT_PRESSING: {
            if (press_hold) {
                servo[servo_num].adjust(data[1] == 'u' ? 10 : data[1] == 'd' ? -10 : 0);
//                servos::send_pulse_json(servo_num);
            }
            break;
        }
        case LV_EVENT_LONG_PRESSED: {
            press_hold = true;
            break;
        }
        case LV_EVENT_RELEASED: {
            if (!press_hold) {
                servo[servo_num].adjust(data[1] == 'u' ? 10 : data[1] == 'd' ? -10 : 0);                
            }
            servos::send_pulse_json(servo_num);
            press_hold = false;
            break;
        }
        case LV_EVENT_CLICKED: {
            if (data[1] == 'o') {
                servo[servo_num].open_pulse = servo[servo_num].current_pulse;
            }
            else if (data[1] == 'c') {
                servo[servo_num].close_pulse = servo[servo_num].current_pulse;
            }
            else {
                return;
            }
            servo[servo_num].update();

            config::store();
            break;
        }
        default: {
            break;
        }
    }
}

void config::store() {
    ESP_LOGI(TAG, "%s", __FUNCTION__);

    json_buff = "{\"units\":\"";
    json_buff += (units == UNITS_BRITISH ? "british" : "metric");
    json_buff += "\",\"palette\":";
    json_buff.StrFromInt((int32_t)palette);
    json_buff += ',';
    servos::get_json(json_buff);
    json_buff += "}";
    write_file("config.jsn", json_buff);
}

void config::load() {
    ESP_LOGI(TAG, "%s", __FUNCTION__);

    if (read_file("config.jsn", json_buff) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to load config");
        return;
    }
    ESP_LOGI(TAG, "load read: %s", json_buff.Pch());
    if (!json.Parse(json_buff)) {
        ESP_LOGE(TAG, "Error parsing config JSON: \n%s", json.ErrorString().Pch());
        return;
    }

    units = (json.Root()->Find("units", "metric") ? UNITS_METRIC : UNITS_BRITISH);

    palette = LV_PALETTE_BLUE;
    JToken * pJTokenPalette = json.Root()->Find("palette");
    if (pJTokenPalette) {
        palette = (lv_palette_t) pJTokenPalette->Child()->Value().StrToInt();
        ESP_LOGI(TAG, "Read palette %d", palette);
    }

    JToken * pJTokenServo = json.Root()->Find("servos");
    uint8_t i;
    for (i = 0; pJTokenServo && i < 2; i++) {
        pJTokenServo = pJTokenServo->Child();
    }

    for (i = 0; pJTokenServo && pJTokenServo->Child() && i < servos::servo_cnt; i++) {
        JToken * pJToken = pJTokenServo->Child()->Find("open_pulse");

        if (pJToken && pJToken->Child()) {
            servos::servo[i].open_pulse = pJToken->Child()->Value().StrToInt();
        }
        
        pJToken = pJTokenServo->Child()->Find("close_pulse");

        if (pJToken && pJToken->Child()) {
            servos::servo[i].close_pulse = pJToken->Child()->Value().StrToInt();
        }
        pJTokenServo = pJTokenServo->Next();
    }
}

void config::units_event_cb(lv_event_t * e)
{
    ESP_LOGI(TAG, "%s", __FUNCTION__);

    config::units = (config::units == UNITS_BRITISH ? UNITS_METRIC : UNITS_BRITISH);
    speedo::update();

    config::store();
}

void home_screen(void)
{
    ESP_LOGI(TAG, "%s", __FUNCTION__);

    config::load();

    if(LV_HOR_RES <= 320) disp_size = DISP_SMALL;
    else if(LV_HOR_RES < 720) disp_size = DISP_MEDIUM;
    else disp_size = DISP_LARGE;

    ESP_LOGI(TAG, "disp_size %d", disp_size);

    lv_coord_t tab_h;
    tab_h = 45;
    font_normal = &lv_font_montserrat_14;
    font_large = &lv_font_montserrat_24;

    lv_palette_t palette_secondary = (lv_palette_t) (config::palette + 3); /*Use another palette as secondary*/
    if (palette_secondary >= _LV_PALETTE_LAST) palette_secondary = (lv_palette_t) 0;
#if LV_USE_THEME_DEFAULT
    lv_theme_default_init(NULL, lv_palette_main(config::palette), lv_palette_main(palette_secondary),
                            LV_THEME_DEFAULT_DARK, font_normal);
#endif

    lv_style_init(&style_text_muted);
    lv_style_set_text_opa(&style_text_muted, LV_OPA_50);

    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, font_large);

    lv_style_init(&style_xl);
    lv_style_set_text_font(&style_xl, &lv_font_montserrat_48);

    lv_style_init(&style_icon);
    lv_color_t color = lv_palette_main(config::palette);
    lv_style_set_text_color(&style_icon, color);
    lv_style_set_text_font(&style_icon, font_large);

    lv_style_init(&style_bullet);
    lv_style_set_border_width(&style_bullet, 0);
    lv_style_set_radius(&style_bullet, LV_RADIUS_CIRCLE);

    lv_style_init(&style_indicators);
    lv_style_set_radius(&style_indicators, 0); // Square corners
    lv_style_set_bg_color(&style_indicators, lv_palette_darken(LV_PALETTE_GREEN, 4));
    lv_style_set_border_color(&style_indicators, lv_palette_darken(LV_PALETTE_GREY, 4));
    lv_style_set_border_width(&style_indicators, 2);
    lv_style_set_pad_all(&style_indicators, 5); // Padding around the label

    tv = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, tab_h);
    lv_obj_set_style_text_font(lv_scr_act(), font_normal, 0);
    lv_obj_t *tab_content = lv_tabview_get_content(tv);
    // Disable the swipe-to-switch behavior by clearing the scrollable flag
    lv_obj_clear_flag(tab_content, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * t1 = lv_tabview_add_tab(tv, "Main");
//    lv_obj_t * t2 = lv_tabview_add_tab(tv, "Accessories");
    lv_obj_t * t3 = lv_tabview_add_tab(tv, "Config");

    main_create(t1);
//    accessories_create(t2);
    settings_create(t3);

    color_changer_create(tv);
}

extern "C" {
void run_home_screen(void) {
    home_screen();
}

void handle_message(const char *pch)
{
    if (!json.Parse(pch)) {
        ESP_LOGE(TAG, "Error parsing config JSON: \n%s", json.ErrorString().Pch());
        return;
    }

    JToken * pJToken;
    if ((pJToken = json.Root()->Find("gps"))) {
        ESP_LOGI(TAG, "here 1");
        pJToken = pJToken->Child();
        if (pJToken && (pJToken = pJToken->Find("speed"))) {
            ESP_LOGI(TAG, "here 2");
            pJToken = pJToken->Child();
            if (pJToken && (pJToken = pJToken->Find(config::units == config::UNITS_METRIC ? "kmph" : "mph"))) {
                ESP_LOGI(TAG, "here 3: %d", (int)pJToken->ChildValue().StrToInt());
                speedo::set_value((int32_t)pJToken->ChildValue().StrToInt());
            }
        }
    }
    else if ((pJToken = json.Root()->Find("12vInputs"))) {
        status_inputs::status_input[0].set(pJToken->Child()->Value().StrToInt());
        ESP_LOGI(TAG, "handle_message 12vInputs: %d", (int)pJToken->Child()->Value().StrToInt());
    }
}
}

void home_screen_close(void)
{
    ESP_LOGI(TAG, "%s", __FUNCTION__);

    /*Delete all animation*/
    lv_anim_del(NULL, NULL);

    lv_obj_clean(lv_scr_act());

    lv_style_reset(&style_text_muted);
    lv_style_reset(&style_title);
    lv_style_reset(&style_xl);
    lv_style_reset(&style_icon);
    lv_style_reset(&style_bullet);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void main_create(lv_obj_t * parent)
{
    ESP_LOGI(TAG, "%s", __FUNCTION__);

//    char temp[128];
//    Buffer temp_buffer(temp, sizeof(temp));

    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_height(cont, 280);
    lv_obj_set_flex_grow(cont, 1);

    speedo::meter = lv_meter_create(cont);
    lv_obj_remove_style(speedo::meter, NULL, LV_PART_MAIN);
    lv_obj_remove_style(speedo::meter, NULL, LV_PART_INDICATOR);
 //   lv_obj_set_width(speedo::meter, 200);

    lv_obj_add_flag(lv_obj_get_parent(speedo::meter), LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);

    /*Add a special circle to the needle's pivot*/
    lv_obj_set_style_pad_hor(speedo::meter, 1, 0);
    lv_obj_set_style_size(speedo::meter, 120, LV_PART_INDICATOR);
    lv_obj_set_style_radius(speedo::meter, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(speedo::meter, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(speedo::meter, lv_palette_darken(LV_PALETTE_GREY, 3), LV_PART_INDICATOR);
//    lv_obj_set_style_outline_color(speedo::meter, lv_color_white(), LV_PART_INDICATOR);
//    lv_obj_set_style_outline_width(speedo::meter, 3, LV_PART_INDICATOR);
    lv_obj_set_style_text_color(speedo::meter, lv_palette_darken(LV_PALETTE_GREY, 1), LV_PART_TICKS);

    speedo::scale = lv_meter_add_scale(speedo::meter);
    speedo::indic_arc = lv_meter_add_arc(speedo::meter, speedo::scale, 12, lv_palette_main(LV_PALETTE_BLUE), 0);
    speedo::indic_scale_lines = lv_meter_add_scale_lines(speedo::meter, speedo::scale, 
                                    lv_palette_darken(LV_PALETTE_BLUE, 3), lv_palette_darken(LV_PALETTE_BLUE, 3), true, 0);
    speedo::indic_arc_high = lv_meter_add_arc(speedo::meter, speedo::scale, 10, lv_palette_main(LV_PALETTE_RED), 0);
    speedo::indic_scale_lines_high = lv_meter_add_scale_lines(speedo::meter, speedo::scale, lv_palette_darken(LV_PALETTE_RED, 3),
                                     lv_palette_darken(LV_PALETTE_RED, 3), true, 0);
    
    speedo::indic_needle = lv_meter_add_needle_line(speedo::meter, speedo::scale, 4, lv_palette_darken(LV_PALETTE_RED, 4), -12);

    speedo::speed_label = lv_label_create(speedo::meter);
    lv_label_set_text(speedo::speed_label, "N/A");
    lv_obj_add_style(speedo::speed_label, &style_xl, 0);

/*    lv_obj_t * speed_units_label = lv_label_create(speedo::meter);
    lv_label_set_text(speed_units_label, "M/H");
*/
    lv_obj_t * units_btn = lv_btn_create(speedo::meter);
    lv_obj_set_height(units_btn, LV_SIZE_CONTENT);
    lv_obj_add_event_cb(units_btn, config::units_event_cb, LV_EVENT_CLICKED, NULL);

    speedo::units_label = lv_label_create(units_btn);
    speedo::update();

    lv_obj_update_layout(parent);
    lv_obj_set_size(speedo::meter, 240, 240);
    lv_obj_align(speedo::speed_label, LV_ALIGN_CENTER, 0, lv_pct(0));
    lv_obj_align_to(units_btn, speedo::speed_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 50);

    // Create the first status indicator
    lv_obj_t *status = lv_obj_create(parent);
    lv_obj_add_style(status, &style_indicators, 0);
    lv_obj_set_size(status, 140, 50); // Size of the square
    status_inputs::status_input[0].status = status;
    lv_obj_align_to(status, speedo::meter, LV_ALIGN_OUT_TOP_RIGHT, 0, 50);

    // Create a label for the status indicator
    lv_obj_t *label = lv_label_create(status);
    status_inputs::status_input[0].onLabel = "Low Fuel";
    status_inputs::status_input[0].offLabel = "Fuel";
    lv_label_set_text(label, status_inputs::status_input[0].offLabel.Pch());
    lv_obj_center(label); // Center the label within the container
    status_inputs::status_input[0].status_label = label;

/*
    static const char *fan_map[] = { "Off", "Low", "Medium", "High", "" };
    lv_obj_t *fan_btnm = lv_btnmatrix_create(parent);
    lv_btnmatrix_set_map(fan_btnm, fan_map);
    lv_obj_set_size(fan_btnm, 200, 50);
    lv_obj_align_to(fan_btnm, speedo::meter, LV_ALIGN_BOTTOM_LEFT, 0, 50);
    lv_obj_add_event_cb(fan_btnm, hvac::fan_speed_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
*/

    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_set_width(panel, lv_pct(100)); 
    lv_obj_set_height(panel, 300); 
    // lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_ROW_WRAP);
/*
    label = lv_label_create(panel);
    lv_label_set_text(label, "HVAC");
    lv_obj_add_style(label, &style_title, 0);
*/
    // Create the A/C toggle button
    lv_obj_t *ac_btn = lv_btn_create(panel);
    lv_obj_set_size(ac_btn, 100, 50);
//    lv_obj_align_to(ac_btn, speedo::meter, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_t *ac_label = lv_label_create(ac_btn);
    lv_label_set_text(ac_label, "A/C OFF");
    lv_obj_center(ac_label);
    lv_obj_add_event_cb(ac_btn, hvac::ac_toggle_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *img = lv_img_create(panel);
    // Set the image source to the C array image
    lv_img_set_src(img, &temperature_bar_lg);  // Use the LVGL Image Converter C array here
    lv_obj_align_to(img, panel, LV_ALIGN_TOP_MID, 0, 70);

    // Create the temperature adjustment slider
    lv_obj_t *temp_slider = lv_slider_create(panel);
    lv_slider_set_range(temp_slider, 0, 100);
    lv_obj_set_width(temp_slider, lv_pct(90));
    lv_obj_set_height(temp_slider, 40);
    lv_obj_set_style_bg_opa(temp_slider, LV_OPA_TRANSP, LV_PART_MAIN);  // Make the main background transparent
    lv_obj_set_style_bg_opa(temp_slider, LV_OPA_TRANSP, LV_PART_INDICATOR);  // Make the indicator (track) transparent

    lv_obj_align_to(temp_slider, img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(temp_slider, hvac::temp_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    img = lv_img_create(panel);
    // Set the image source to the C array image
    lv_img_set_src(img, &HVAC_icons_bar);  // Use the LVGL Image Converter C array here
    lv_obj_align_to(img, temp_slider, LV_ALIGN_BOTTOM_MID, 0, 60);

    // Create the airflow zone slider
    lv_obj_t *zone_slider = lv_slider_create(panel);
    lv_slider_set_range(zone_slider, 0, 100);  // Range from feet to defrost
    lv_obj_set_width(zone_slider, lv_pct(90));
    lv_obj_set_height(zone_slider, 40);
//    lv_obj_set_style_bg_opa(zone_slider, LV_OPA_TRANSP, LV_PART_MAIN);  // Make the main background transparent
    lv_obj_set_style_bg_opa(zone_slider, LV_OPA_TRANSP, LV_PART_INDICATOR);  // Make the indicator (track) transparent
//    lv_obj_set_style_bg_opa(zone_slider, LV_OPA_50, LV_PART_KNOB);  // Make knob opaque
    lv_obj_set_style_radius(zone_slider, LV_RADIUS_CIRCLE, LV_PART_KNOB);  // Make knob circular

    lv_obj_align_to(zone_slider, img, LV_ALIGN_BOTTOM_MID, 0, 46);
    lv_obj_add_event_cb(zone_slider, hvac::zone_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

        // Create the blower off button
    lv_obj_t *blower_off_btn = lv_btn_create(panel);
    lv_obj_set_size(blower_off_btn, 100, 50);
    lv_obj_t *blower_off_label = lv_label_create(blower_off_btn);
    lv_label_set_text(blower_off_label, "A/C OFF");
    lv_obj_center(blower_off_label);
    lv_obj_add_event_cb(blower_off_btn, relays::btn_event_cb, LV_EVENT_CLICKED, encode_action(i,"on"));
}

#if 0
static void accessories_create(lv_obj_t * parent)
{
    ESP_LOGI(TAG, "%s", __FUNCTION__);

    lv_obj_t * panel1 = lv_obj_create(parent);
    lv_obj_set_height(panel1, LV_SIZE_CONTENT);

/*
    LV_IMG_DECLARE(img_demo_widgets_avatar);
    lv_obj_t * avatar = lv_img_create(panel1);
    lv_img_set_src(avatar, &img_demo_widgets_avatar);
*/

    lv_obj_t * name = lv_label_create(panel1);
    lv_label_set_text(name, "Elena Smith");
    lv_obj_add_style(name, &style_title, 0);

    lv_obj_t * dsc = lv_label_create(panel1);
    lv_obj_add_style(dsc, &style_text_muted, 0);
    lv_label_set_text_fmt(dsc, "LV_HOR_RES: %d, LV_VER_RES: %d", LV_HOR_RES, LV_VER_RES);
    lv_label_set_long_mode(dsc, LV_LABEL_LONG_WRAP);

    lv_obj_t * email_icn = lv_label_create(panel1);
    lv_obj_add_style(email_icn, &style_icon, 0);
    lv_label_set_text(email_icn, LV_SYMBOL_ENVELOPE);

    lv_obj_t * email_label = lv_label_create(panel1);
    lv_label_set_text(email_label, "elena@smith.com");

    lv_obj_t * call_icn = lv_label_create(panel1);
    lv_obj_add_style(call_icn, &style_icon, 0);
    lv_label_set_text(call_icn, LV_SYMBOL_CALL);

    lv_obj_t * call_label = lv_label_create(panel1);
    lv_label_set_text(call_label, "+79 246 123 4567");

    lv_obj_t * log_out_btn = lv_btn_create(panel1);
    lv_obj_set_height(log_out_btn, LV_SIZE_CONTENT);

    lv_obj_t * label = lv_label_create(log_out_btn);
    lv_label_set_text(label, "Log out");
    lv_obj_center(label);

    lv_obj_t * invite_btn = lv_btn_create(panel1);
    lv_obj_add_state(invite_btn, LV_STATE_DISABLED);
    lv_obj_set_height(invite_btn, LV_SIZE_CONTENT);

    label = lv_label_create(invite_btn);
    lv_label_set_text(label, "Invite");
    lv_obj_center(label);

    /*Create a keyboard*/
    lv_obj_t * kb = lv_keyboard_create(lv_scr_act());
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

    /*Create the second panel*/
    lv_obj_t * panel2 = lv_obj_create(parent);
    lv_obj_set_height(panel2, LV_SIZE_CONTENT);

    lv_obj_t * panel2_title = lv_label_create(panel2);
    lv_label_set_text(panel2_title, "Your profile");
    lv_obj_add_style(panel2_title, &style_title, 0);

    lv_obj_t * user_name_label = lv_label_create(panel2);
    lv_label_set_text(user_name_label, "User name");
    lv_obj_add_style(user_name_label, &style_text_muted, 0);

    lv_obj_t * user_name = lv_textarea_create(panel2);
    lv_textarea_set_one_line(user_name, true);
    lv_textarea_set_placeholder_text(user_name, "Your name");
 //   lv_obj_add_event_cb(user_name, ta_event_cb, LV_EVENT_ALL, kb);

    lv_obj_t * password_label = lv_label_create(panel2);
    lv_label_set_text(password_label, "Password");
    lv_obj_add_style(password_label, &style_text_muted, 0);

    lv_obj_t * password = lv_textarea_create(panel2);
    lv_textarea_set_one_line(password, true);
    lv_textarea_set_password_mode(password, true);
    lv_textarea_set_placeholder_text(password, "Min. 8 chars.");
 //   lv_obj_add_event_cb(password, ta_event_cb, LV_EVENT_ALL, kb);

    lv_obj_t * gender_label = lv_label_create(panel2);
    lv_label_set_text(gender_label, "Gender");
    lv_obj_add_style(gender_label, &style_text_muted, 0);

    lv_obj_t * gender = lv_dropdown_create(panel2);
    lv_dropdown_set_options_static(gender, "Male\nFemale\nOther");

    lv_obj_t * birthday_label = lv_label_create(panel2);
    lv_label_set_text(birthday_label, "Birthday");
    lv_obj_add_style(birthday_label, &style_text_muted, 0);

    lv_obj_t * birthdate = lv_textarea_create(panel2);
    lv_textarea_set_one_line(birthdate, true);
    lv_obj_add_event_cb(birthdate, birthday_event_cb, LV_EVENT_ALL, NULL);

    /*Create the third panel*/
    lv_obj_t * panel3 = lv_obj_create(parent);
    lv_obj_t * panel3_title = lv_label_create(panel3);
    lv_label_set_text(panel3_title, "Your skills");
    lv_obj_add_style(panel3_title, &style_title, 0);

    lv_obj_t * experience_label = lv_label_create(panel3);
    lv_label_set_text(experience_label, "Experience");
    lv_obj_add_style(experience_label, &style_text_muted, 0);

    lv_obj_t * slider1 = lv_slider_create(panel3);
    lv_obj_set_width(slider1, LV_PCT(95));
    lv_obj_add_event_cb(slider1, slider_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_refresh_ext_draw_size(slider1);

    lv_obj_t * team_player_label = lv_label_create(panel3);
    lv_label_set_text(team_player_label, "Team player");
    lv_obj_add_style(team_player_label, &style_text_muted, 0);

    lv_obj_t * sw1 = lv_switch_create(panel3);

    lv_obj_t * hard_working_label = lv_label_create(panel3);
    lv_label_set_text(hard_working_label, "Hard-working");
    lv_obj_add_style(hard_working_label, &style_text_muted, 0);

    lv_obj_t * sw2 = lv_switch_create(panel3);

    static lv_coord_t grid_main_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_main_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

    /*Create the top panel*/
    static lv_coord_t grid_1_col_dsc[] = {LV_GRID_CONTENT, 1, LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_1_row_dsc[] = {
        LV_GRID_CONTENT, /*Name*/
        LV_GRID_CONTENT, /*Description*/
        LV_GRID_CONTENT, /*Email*/
        -20,
        LV_GRID_CONTENT, /*Phone*/
        LV_GRID_CONTENT, /*Buttons*/
        LV_GRID_TEMPLATE_LAST
    };

    static lv_coord_t grid_2_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_2_row_dsc[] = {
        LV_GRID_CONTENT,  /*Title*/
        5,                /*Separator*/
        LV_GRID_CONTENT,  /*Box title*/
        40,               /*Box*/
        LV_GRID_CONTENT,  /*Box title*/
        40,               /*Box*/
        LV_GRID_CONTENT,  /*Box title*/
        40,               /*Box*/
        LV_GRID_CONTENT,  /*Box title*/
        40,               /*Box*/
        LV_GRID_TEMPLATE_LAST
    };

    lv_obj_set_grid_dsc_array(parent, grid_main_col_dsc, grid_main_row_dsc);
    lv_obj_set_grid_cell(panel1, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);

//    lv_obj_set_width(log_out_btn, 120);
//    lv_obj_set_width(invite_btn, 120);

    lv_obj_set_grid_dsc_array(panel1, grid_1_col_dsc, grid_1_row_dsc);
    lv_obj_set_grid_cell(name, LV_GRID_ALIGN_START, 2, 2, LV_GRID_ALIGN_CENTER, 0, 1);
/*    lv_obj_set_grid_cell(dsc, LV_GRID_ALIGN_STRETCH, 2, 2, LV_GRID_ALIGN_START, 1, 1);
    lv_obj_set_grid_cell(email_label, LV_GRID_ALIGN_START, 3, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    lv_obj_set_grid_cell(email_icn, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    lv_obj_set_grid_cell(call_icn, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 4, 1);
    lv_obj_set_grid_cell(call_label, LV_GRID_ALIGN_START, 3, 1, LV_GRID_ALIGN_CENTER, 4, 1);
    lv_obj_set_grid_cell(log_out_btn, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 5, 1);
    lv_obj_set_grid_cell(invite_btn, LV_GRID_ALIGN_END, 3, 1, LV_GRID_ALIGN_CENTER, 5, 1);

    lv_obj_set_grid_cell(panel2, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 1, 1);
    lv_obj_set_grid_dsc_array(panel2, grid_2_col_dsc, grid_2_row_dsc);
    lv_obj_set_grid_cell(panel2_title, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_grid_cell(user_name_label, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 2, 1);
    lv_obj_set_grid_cell(user_name, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_START, 3, 1);
    lv_obj_set_grid_cell(password_label, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 4, 1);
    lv_obj_set_grid_cell(password, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_START, 5, 1);
    lv_obj_set_grid_cell(birthday_label, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 6, 1);
    lv_obj_set_grid_cell(birthdate, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_START, 7, 1);
    lv_obj_set_grid_cell(gender_label, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 8, 1);
    lv_obj_set_grid_cell(gender, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_START, 9, 1);

    lv_obj_set_grid_cell(panel3, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_grid_dsc_array(panel3, grid_2_col_dsc, grid_2_row_dsc);
    lv_obj_set_grid_cell(panel3_title, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_grid_cell(slider1, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 3, 1);
    lv_obj_set_grid_cell(experience_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
    lv_obj_set_grid_cell(hard_working_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 4, 1);
    lv_obj_set_grid_cell(sw2, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 5, 1);
    lv_obj_set_grid_cell(team_player_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 6, 1);
    lv_obj_set_grid_cell(sw1, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 7, 1);
    */
}
#endif

char * encode_action(uint8_t dev_num, const String & button)
{
    Buffer result(button.Len() + 5);
    result.StrFromInt(dev_num);
    result += button;

    return result.Pch();
}

void settings_create(lv_obj_t * parent)
{
    ESP_LOGI(TAG, "%s", __FUNCTION__);

    char temp[128];
    Buffer temp_buffer(temp, sizeof(temp));

    lv_obj_t * main_panel = lv_obj_create(parent);
    lv_obj_set_width(main_panel, lv_pct(100)); 
//    lv_obj_set_height(panel, LV_SIZE_CONTENT);
//    lv_obj_set_flex_grow(panel, 1);
    lv_obj_set_flex_flow(main_panel, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_t * title = lv_label_create(main_panel);
    lv_label_set_text(title, "Servo Configuration (DO NOT TOUCH!!!)");
    lv_obj_add_style(title, &style_title, 0);

    for (uint8_t i = 0; i < servos::servo_cnt; i++) 
    {
        lv_obj_t * label;
        lv_obj_t * panel = lv_obj_create(main_panel);
        lv_obj_set_width(panel, lv_pct(100)); 
        lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_ROW_WRAP);

        label = lv_label_create(panel);
        temp_buffer = "I/O : ";
        temp_buffer.StrFromInt(i);
        lv_label_set_text(label, temp_buffer.Pch());
        lv_obj_add_style(label, &style_title, 0);

        lv_obj_t * btn = lv_btn_create(panel);
        lv_obj_set_height(btn, LV_SIZE_CONTENT);
        lv_obj_set_width(btn, lv_pct(28));
        lv_obj_add_event_cb(btn, relays::btn_event_cb, LV_EVENT_CLICKED, encode_action(i,"on"));

        label = lv_label_create(btn);
        lv_label_set_text(label, "Relay On");
        lv_obj_center(label);

        btn = lv_btn_create(panel);
        lv_obj_set_height(btn, LV_SIZE_CONTENT);
        lv_obj_set_width(btn, lv_pct(28));
        lv_obj_add_event_cb(btn, relays::btn_event_cb, LV_EVENT_CLICKED, encode_action(i,"off"));

        label = lv_label_create(btn);
        lv_label_set_text(label, "Relay off");
        lv_obj_center(label);

        // Add a spacer to force the next label to a new row
        lv_obj_t *spacer = lv_obj_create(panel);
        lv_obj_set_size(spacer, LV_PCT(100), 0);

        label = lv_label_create(panel);
        lv_obj_add_style(label, &style_title, 0);
        lv_obj_set_width(label, lv_pct(30)); 
        servos::servo[i].current_label = label;

        label = lv_label_create(panel);
        lv_obj_add_style(label, &style_title, 0);
        lv_obj_set_width(label, lv_pct(30)); 
        servos::servo[i].open_label = label;

        label = lv_label_create(panel);
        lv_obj_add_style(label, &style_title, 0);
        lv_obj_set_width(label, lv_pct(30));
        servos::servo[i].close_label = label;

        servos::servo[i].update();

        spacer = lv_obj_create(panel);
        lv_obj_set_size(spacer, LV_PCT(100), 0);

        btn = lv_btn_create(panel);
        lv_obj_set_height(btn, LV_SIZE_CONTENT);
        lv_obj_set_width(btn, lv_pct(14));
        lv_obj_add_event_cb(btn, servos::btn_event_cb, LV_EVENT_PRESSING, encode_action(i,"up"));
        lv_obj_add_event_cb(btn, servos::btn_event_cb, LV_EVENT_LONG_PRESSED, encode_action(i,"up"));
        lv_obj_add_event_cb(btn, servos::btn_event_cb, LV_EVENT_RELEASED, encode_action(i,"up"));

        label = lv_label_create(btn);
        lv_label_set_text(label, LV_SYMBOL_UP);
        lv_obj_center(label);

        btn = lv_btn_create(panel);
        lv_obj_set_height(btn, LV_SIZE_CONTENT);
        lv_obj_set_width(btn, lv_pct(14));
        lv_obj_add_event_cb(btn, servos::btn_event_cb, LV_EVENT_PRESSING, encode_action(i,"down"));
        lv_obj_add_event_cb(btn, servos::btn_event_cb, LV_EVENT_LONG_PRESSED, encode_action(i,"down"));
        lv_obj_add_event_cb(btn, servos::btn_event_cb, LV_EVENT_RELEASED, encode_action(i,"down"));

        label = lv_label_create(btn);
        lv_label_set_text(label, LV_SYMBOL_DOWN);
        lv_obj_center(label);

        btn = lv_btn_create(panel);
        lv_obj_set_height(btn, LV_SIZE_CONTENT);
        lv_obj_set_width(btn, lv_pct(28));
        lv_obj_add_event_cb(btn, servos::btn_event_cb, LV_EVENT_CLICKED, encode_action(i,"open"));

        label = lv_label_create(btn);
        lv_label_set_text(label, "Set Open");
        lv_obj_center(label);

        btn = lv_btn_create(panel);
        lv_obj_set_height(btn, LV_SIZE_CONTENT);
        lv_obj_set_width(btn, lv_pct(28));
        lv_obj_add_event_cb(btn, servos::btn_event_cb, LV_EVENT_CLICKED, encode_action(i,"close"));

        label = lv_label_create(btn);
        lv_label_set_text(label, "Set Close");
        lv_obj_center(label);

        lv_obj_set_height(panel, LV_SIZE_CONTENT);
    }

    lv_obj_set_height(main_panel, LV_SIZE_CONTENT);

#if 0
    lv_obj_t * date = lv_label_create(panel1);
    lv_label_set_text(date, "8-15 July, 2021");
    lv_obj_add_style(date, &style_text_muted, 0);

    lv_obj_t * amount = lv_label_create(panel1);
    lv_label_set_text(amount, "$27,123.25");
    lv_obj_add_style(amount, &style_title, 0);

    lv_obj_t * hint = lv_label_create(panel1);
    lv_label_set_text(hint, LV_SYMBOL_UP" 17% growth this week");
    lv_obj_set_style_text_color(hint, lv_palette_main(LV_PALETTE_GREEN), 0);

    chart3 = lv_chart_create(panel1);
    lv_chart_set_axis_tick(chart3, LV_CHART_AXIS_PRIMARY_Y, 0, 0, 6, 1, true, 80);
    lv_chart_set_axis_tick(chart3, LV_CHART_AXIS_PRIMARY_X, 0, 0, 7, 1, true, 50);
    lv_chart_set_type(chart3, LV_CHART_TYPE_BAR);
    lv_chart_set_div_line_count(chart3, 6, 0);
    lv_chart_set_point_count(chart3, 7);
//    lv_obj_add_event_cb(chart3, shop_chart_event_cb, LV_EVENT_ALL, NULL);

    ser4 = lv_chart_add_series(chart3, lv_theme_get_color_primary(chart3), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_next_value(chart3, ser4, lv_rand(60, 90));
    lv_chart_set_next_value(chart3, ser4, lv_rand(60, 90));
    lv_chart_set_next_value(chart3, ser4, lv_rand(60, 90));
    lv_chart_set_next_value(chart3, ser4, lv_rand(60, 90));
    lv_chart_set_next_value(chart3, ser4, lv_rand(60, 90));
    lv_chart_set_next_value(chart3, ser4, lv_rand(60, 90));
    lv_chart_set_next_value(chart3, ser4, lv_rand(60, 90));
    lv_chart_set_next_value(chart3, ser4, lv_rand(60, 90));
    lv_chart_set_next_value(chart3, ser4, lv_rand(60, 90));
    lv_chart_set_next_value(chart3, ser4, lv_rand(60, 90));
    lv_chart_set_next_value(chart3, ser4, lv_rand(60, 90));
    lv_chart_set_next_value(chart3, ser4, lv_rand(60, 90));

    if(disp_size == DISP_LARGE) {
        static lv_coord_t grid1_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
        static lv_coord_t grid1_row_dsc[] = {
            LV_GRID_CONTENT,  /*Title*/
            LV_GRID_CONTENT,  /*Sub title*/
            20,               /*Spacer*/
            LV_GRID_CONTENT,  /*Amount*/
            LV_GRID_CONTENT,  /*Hint*/
            LV_GRID_TEMPLATE_LAST
        };

        lv_obj_set_size(chart3, lv_pct(100), lv_pct(100));
        lv_obj_set_style_pad_column(chart3, LV_DPX(30), 0);


        lv_obj_set_grid_dsc_array(panel1, grid1_col_dsc, grid1_row_dsc);
        lv_obj_set_grid_cell(title, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
        lv_obj_set_grid_cell(date, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 1, 1);
        lv_obj_set_grid_cell(amount, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 3, 1);
        lv_obj_set_grid_cell(hint, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 4, 1);
        lv_obj_set_grid_cell(chart3, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 5);
    }
    else if(disp_size == DISP_MEDIUM) {
        static lv_coord_t grid1_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
        static lv_coord_t grid1_row_dsc[] = {
            LV_GRID_CONTENT,  /*Title + Date*/
            LV_GRID_CONTENT,  /*Amount + Hint*/
            200,              /*Chart*/
            LV_GRID_TEMPLATE_LAST
        };

        lv_obj_update_layout(panel1);
        lv_obj_set_width(chart3, lv_obj_get_content_width(panel1) - 20);
        lv_obj_set_style_pad_column(chart3, LV_DPX(30), 0);

        lv_obj_set_grid_dsc_array(panel1, grid1_col_dsc, grid1_row_dsc);
        lv_obj_set_grid_cell(title, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
        lv_obj_set_grid_cell(date, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
        lv_obj_set_grid_cell(amount, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
        lv_obj_set_grid_cell(hint, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
        lv_obj_set_grid_cell(chart3, LV_GRID_ALIGN_END, 0, 2, LV_GRID_ALIGN_STRETCH, 2, 1);
    }
    else if(disp_size == DISP_SMALL) {
        static lv_coord_t grid1_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
        static lv_coord_t grid1_row_dsc[] = {
            LV_GRID_CONTENT,  /*Title*/
            LV_GRID_CONTENT,  /*Date*/
            LV_GRID_CONTENT,  /*Amount*/
            LV_GRID_CONTENT,  /*Hint*/
            LV_GRID_CONTENT,  /*Chart*/
            LV_GRID_TEMPLATE_LAST
        };

        lv_obj_set_width(chart3, LV_PCT(95));
        lv_obj_set_height(chart3, LV_VER_RES - 70);
        lv_obj_set_style_max_height(chart3, 300, 0);
        lv_chart_set_zoom_x(chart3, 512);

        lv_obj_set_grid_dsc_array(panel1, grid1_col_dsc, grid1_row_dsc);
        lv_obj_set_grid_cell(title, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
        lv_obj_set_grid_cell(date, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 1, 1);
        lv_obj_set_grid_cell(amount, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
        lv_obj_set_grid_cell(hint, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 3, 1);
        lv_obj_set_grid_cell(chart3, LV_GRID_ALIGN_END, 0, 1, LV_GRID_ALIGN_START, 4, 1);
    }

    lv_obj_t * list = lv_obj_create(parent);
    if(disp_size == DISP_SMALL) {
        lv_obj_add_flag(list, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
        lv_obj_set_height(list, LV_PCT(100));
    }
    else {
        lv_obj_set_height(list, LV_PCT(100));
        lv_obj_set_style_max_height(list, 300, 0);
    }

    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_grow(list, 1);
    lv_obj_add_flag(list, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);

    title = lv_label_create(list);
    lv_label_set_text(title, "Top products");
    lv_obj_add_style(title, &style_title, 0);

    lv_obj_t * notifications = lv_obj_create(parent);
    if(disp_size == DISP_SMALL) {
        lv_obj_add_flag(notifications, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
        lv_obj_set_height(notifications, LV_PCT(100));
    }
    else  {
        lv_obj_set_height(notifications, LV_PCT(100));
        lv_obj_set_style_max_height(notifications, 300, 0);
    }

    lv_obj_set_flex_flow(notifications, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_grow(notifications, 1);

    title = lv_label_create(notifications);
    lv_label_set_text(title, "Notification");
    lv_obj_add_style(title, &style_title, 0);

    lv_obj_t * cb;
    cb = lv_checkbox_create(notifications);
    lv_checkbox_set_text(cb, "Item purchased");

    cb = lv_checkbox_create(notifications);
    lv_checkbox_set_text(cb, "New connection");

    cb = lv_checkbox_create(notifications);
    lv_checkbox_set_text(cb, "New subscriber");
    lv_obj_add_state(cb, LV_STATE_CHECKED);

    cb = lv_checkbox_create(notifications);
    lv_checkbox_set_text(cb, "New message");
    lv_obj_add_state(cb, LV_STATE_DISABLED);

    cb = lv_checkbox_create(notifications);
    lv_checkbox_set_text(cb, "Milestone reached");
    lv_obj_add_state(cb, LV_STATE_CHECKED | LV_STATE_DISABLED);

    cb = lv_checkbox_create(notifications);
    lv_checkbox_set_text(cb, "Out of stock");
#endif

}

static void color_changer_create(lv_obj_t * parent)
{
    ESP_LOGI(TAG, "%s", __FUNCTION__);

    static lv_palette_t palette[] = {
        LV_PALETTE_BLUE, LV_PALETTE_GREEN, LV_PALETTE_BLUE_GREY,  LV_PALETTE_ORANGE,
        LV_PALETTE_RED, LV_PALETTE_PURPLE, LV_PALETTE_TEAL, _LV_PALETTE_LAST
    };

    lv_obj_t * color_cont = lv_obj_create(parent);
    lv_obj_remove_style_all(color_cont);
    lv_obj_set_flex_flow(color_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(color_cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(color_cont, LV_OBJ_FLAG_FLOATING);

    lv_obj_set_style_bg_color(color_cont, lv_color_black(), 0);
    lv_obj_set_style_pad_right(color_cont, true ? LV_DPX(47) : LV_DPX(55), 0);
    lv_obj_set_style_bg_opa(color_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(color_cont, LV_RADIUS_CIRCLE, 0);

    if(disp_size == DISP_SMALL) lv_obj_set_size(color_cont, LV_DPX(52), LV_DPX(52));
    else lv_obj_set_size(color_cont, LV_DPX(60), LV_DPX(60));

    lv_obj_align(color_cont, LV_ALIGN_BOTTOM_RIGHT, - LV_DPX(10),  - LV_DPX(10));

    uint32_t i;
    for(i = 0; palette[i] != _LV_PALETTE_LAST; i++) {
        lv_obj_t * c = lv_btn_create(color_cont);
        lv_obj_set_style_bg_color(c, lv_palette_main(palette[i]), 0);
        lv_obj_set_style_radius(c, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_opa(c, LV_OPA_TRANSP, 0);
        lv_obj_set_size(c, 20, 20);
        lv_obj_add_event_cb(c, color_event_cb, LV_EVENT_ALL, &palette[i]);
        lv_obj_clear_flag(c, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    }

    lv_obj_t * btn = lv_btn_create(parent);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_FLOATING | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(btn, lv_color_black(), LV_STATE_CHECKED);
    lv_obj_set_style_pad_all(btn, 10, 0);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_add_event_cb(btn, color_changer_event_cb, LV_EVENT_ALL, color_cont);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_TINT, 0);

    lv_obj_set_size(btn, LV_DPX(50), LV_DPX(50));
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -LV_DPX(15), -LV_DPX(15));
}

static void color_changer_anim_cb(void * var, int32_t v)
{
    lv_obj_t * obj = (lv_obj_t *)var;
    lv_coord_t max_w = lv_obj_get_width(lv_obj_get_parent(obj)) - LV_DPX(20);
    lv_coord_t w;

    if(disp_size == DISP_SMALL) {
        w = lv_map(v, 0, 256, LV_DPX(52), max_w);
        lv_obj_set_width(obj, w);
        lv_obj_align(obj, LV_ALIGN_BOTTOM_RIGHT, - LV_DPX(10),  - LV_DPX(10));
    }
    else {
        w = lv_map(v, 0, 256, LV_DPX(60), max_w);
        lv_obj_set_width(obj, w);
        lv_obj_align(obj, LV_ALIGN_BOTTOM_RIGHT, - LV_DPX(10),  - LV_DPX(10));
    }

    if(v > LV_OPA_COVER) v = LV_OPA_COVER;

    uint32_t i;
    for(i = 0; i < lv_obj_get_child_cnt(obj); i++) {
        lv_obj_set_style_opa(lv_obj_get_child(obj, i), v, 0);
    }

}

static void color_changer_event_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t * color_cont = (lv_obj_t *) lv_event_get_user_data(e);
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, color_cont);
        lv_anim_set_exec_cb(&a, color_changer_anim_cb);
        if(lv_obj_get_width(color_cont) < LV_HOR_RES / 2) {
            lv_anim_set_values(&a, 0, 256);
        }
        else {
            lv_anim_set_values(&a, 256, 0);
        }
        lv_anim_set_time(&a, 200);
        lv_anim_start(&a);
    }
}

static void color_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_FOCUSED) {
        lv_obj_t * color_cont = lv_obj_get_parent(obj);
        if(lv_obj_get_width(color_cont) < LV_HOR_RES / 2) {
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, color_cont);
            lv_anim_set_exec_cb(&a, color_changer_anim_cb);
            lv_anim_set_values(&a, 0, 256);
            lv_anim_set_time(&a, 200);
            lv_anim_start(&a);
        }
    }
    else if(code == LV_EVENT_CLICKED) {
        config::palette = *((lv_palette_t *)lv_event_get_user_data(e));
        lv_palette_t palette_secondary = (lv_palette_t) (config::palette + 3); /*Use another palette as secondary*/
        if (palette_secondary >= _LV_PALETTE_LAST) palette_secondary = (lv_palette_t) 0;
#if LV_USE_THEME_DEFAULT
        lv_theme_default_init(NULL, lv_palette_main(config::palette), lv_palette_main(palette_secondary),
                              LV_THEME_DEFAULT_DARK, font_normal);
#endif
        lv_color_t color = lv_palette_main(config::palette);
        lv_style_set_text_color(&style_icon, color);
        ESP_LOGI(TAG, "set palette: %d", (uint16_t)config::palette);
        config::store();
  //      lv_chart_set_series_color(chart1, ser1, color);
    //    lv_chart_set_series_color(chart2, ser3, color);
    }
}

#if 0
static lv_obj_t * create_meter_box(lv_obj_t * parent, const char * title, const char * text1, const char * text2,
                                   const char * text3)
{
    ESP_LOGI(TAG, "%s", __FUNCTION__);

    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_height(cont, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(cont, 1);

    lv_obj_t * title_label = lv_label_create(cont);
    lv_label_set_text(title_label, title);
    lv_obj_add_style(title_label, &style_title, 0);

    lv_obj_t * meter = lv_meter_create(cont);
    lv_obj_remove_style(meter, NULL, LV_PART_MAIN);
    lv_obj_remove_style(meter, NULL, LV_PART_INDICATOR);
    lv_obj_set_width(meter, LV_PCT(100));

    lv_obj_t * bullet1 = lv_obj_create(cont);
    lv_obj_set_size(bullet1, 13, 13);
    lv_obj_remove_style(bullet1, NULL, LV_PART_SCROLLBAR);
    lv_obj_add_style(bullet1, &style_bullet, 0);
    lv_obj_set_style_bg_color(bullet1, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_t * label1 = lv_label_create(cont);
    lv_label_set_text(label1, text1);

    lv_obj_t * bullet2 = lv_obj_create(cont);
    lv_obj_set_size(bullet2, 13, 13);
    lv_obj_remove_style(bullet2, NULL, LV_PART_SCROLLBAR);
    lv_obj_add_style(bullet2, &style_bullet, 0);
    lv_obj_set_style_bg_color(bullet2, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_t * label2 = lv_label_create(cont);
    lv_label_set_text(label2, text2);

    lv_obj_t * bullet3 = lv_obj_create(cont);
    lv_obj_set_size(bullet3, 13, 13);
    lv_obj_remove_style(bullet3,  NULL, LV_PART_SCROLLBAR);
    lv_obj_add_style(bullet3, &style_bullet, 0);
    lv_obj_set_style_bg_color(bullet3, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_obj_t * label3 = lv_label_create(cont);
    lv_label_set_text(label3, text3);

    if(disp_size == DISP_MEDIUM) {
        static lv_coord_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_FR(8), LV_GRID_TEMPLATE_LAST};
        static lv_coord_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

        lv_obj_set_grid_dsc_array(cont, grid_col_dsc, grid_row_dsc);
        lv_obj_set_grid_cell(title_label, LV_GRID_ALIGN_START, 0, 4, LV_GRID_ALIGN_START, 0, 1);
        lv_obj_set_grid_cell(meter, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 1, 3);
        lv_obj_set_grid_cell(bullet1, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_CENTER, 2, 1);
        lv_obj_set_grid_cell(bullet2, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_CENTER, 3, 1);
        lv_obj_set_grid_cell(bullet3, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_CENTER, 4, 1);
        lv_obj_set_grid_cell(label1, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_CENTER, 2, 1);
        lv_obj_set_grid_cell(label2, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_CENTER, 3, 1);
        lv_obj_set_grid_cell(label3, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_CENTER, 4, 1);
    }
    else {
        static lv_coord_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
        static lv_coord_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
        lv_obj_set_grid_dsc_array(cont, grid_col_dsc, grid_row_dsc);
        lv_obj_set_grid_cell(title_label, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 0, 1);
        lv_obj_set_grid_cell(meter, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 1, 1);
        lv_obj_set_grid_cell(bullet1, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
        lv_obj_set_grid_cell(bullet2, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 3, 1);
        lv_obj_set_grid_cell(bullet3, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 4, 1);
        lv_obj_set_grid_cell(label1, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 2, 1);
        lv_obj_set_grid_cell(label2, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 3, 1);
        lv_obj_set_grid_cell(label3, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 4, 1);
    }


    return meter;

}

static void ta_event_cb(lv_event_t * e)
{
    ESP_LOGI(TAG, "%s", __FUNCTION__);

    return;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = (lv_obj_t *) lv_event_get_user_data(e);
    if(code == LV_EVENT_FOCUSED) {
        if(lv_indev_get_type(lv_indev_get_act()) != LV_INDEV_TYPE_KEYPAD) {
            lv_keyboard_set_textarea(kb, ta);
            lv_obj_set_style_max_height(kb, LV_HOR_RES * 2 / 3, 0);
            lv_obj_update_layout(tv);   /*Be sure the sizes are recalculated*/
            lv_obj_set_height(tv, LV_VER_RES - lv_obj_get_height(kb));
            lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
            lv_obj_scroll_to_view_recursive(ta, LV_ANIM_OFF);
        }
    }
    else if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_set_height(tv, LV_VER_RES);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_indev_reset(NULL, ta);

    }
    else if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_set_height(tv, LV_VER_RES);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_state(ta, LV_STATE_FOCUSED);
        lv_indev_reset(NULL, ta);   /*To forget the last clicked object to make it focusable again*/
    }
}

static void birthday_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);

    if(code == LV_EVENT_FOCUSED) {
        if(lv_indev_get_type(lv_indev_get_act()) == LV_INDEV_TYPE_POINTER) {
            if(calendar == NULL) {
                lv_obj_add_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);
                calendar = lv_calendar_create(lv_layer_top());
                lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_50, 0);
                lv_obj_set_style_bg_color(lv_layer_top(), lv_palette_main(LV_PALETTE_GREY), 0);
                if(disp_size == DISP_SMALL) lv_obj_set_size(calendar, 180, 200);
                else if(disp_size == DISP_MEDIUM) lv_obj_set_size(calendar, 200, 220);
                else  lv_obj_set_size(calendar, 300, 330);
                lv_calendar_set_showed_date(calendar, 1990, 01);
                lv_obj_align(calendar, LV_ALIGN_CENTER, 0, 30);
                lv_obj_add_event_cb(calendar, calendar_event_cb, LV_EVENT_ALL, ta);

                lv_calendar_header_dropdown_create(calendar);
            }
        }
    }
}

static void calendar_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = (lv_obj_t *) lv_event_get_user_data(e);
    lv_obj_t * obj = lv_event_get_current_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_calendar_date_t d;
        lv_calendar_get_pressed_date(obj, &d);
        char buf[32];
        lv_snprintf(buf, sizeof(buf), "%02d.%02d.%d", d.day, d.month, d.year);
        lv_textarea_set_text(ta, buf);

        lv_obj_del(calendar);
        calendar = NULL;
        lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);
    }
}

static void slider_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
        lv_coord_t * s = (lv_coord_t *) lv_event_get_param(e);
        *s = LV_MAX(*s, 60);
    }
    else if(code == LV_EVENT_DRAW_PART_END) {
        lv_obj_draw_part_dsc_t * dsc = (lv_obj_draw_part_dsc_t *) lv_event_get_param(e);
        if(dsc->part == LV_PART_KNOB && lv_obj_has_state(obj, LV_STATE_PRESSED)) {
            char buf[8];
            lv_snprintf(buf, sizeof(buf), "%"LV_PRId32, lv_slider_get_value(obj));

            lv_point_t text_size;
            lv_txt_get_size(&text_size, buf, font_normal, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

            lv_area_t txt_area;
            txt_area.x1 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2 - text_size.x / 2;
            txt_area.x2 = txt_area.x1 + text_size.x;
            txt_area.y2 = dsc->draw_area->y1 - 10;
            txt_area.y1 = txt_area.y2 - text_size.y;

            lv_area_t bg_area;
            bg_area.x1 = txt_area.x1 - LV_DPX(8);
            bg_area.x2 = txt_area.x2 + LV_DPX(8);
            bg_area.y1 = txt_area.y1 - LV_DPX(8);
            bg_area.y2 = txt_area.y2 + LV_DPX(8);

            lv_draw_rect_dsc_t rect_dsc;
            lv_draw_rect_dsc_init(&rect_dsc);
            rect_dsc.bg_color = lv_palette_darken(LV_PALETTE_GREY, 3);
            rect_dsc.radius = LV_DPX(5);
            lv_draw_rect(dsc->draw_ctx, &rect_dsc, &bg_area);

            lv_draw_label_dsc_t label_dsc;
            lv_draw_label_dsc_init(&label_dsc);
            label_dsc.color = lv_color_white();
            label_dsc.font = font_normal;
            lv_draw_label(dsc->draw_ctx, &label_dsc, &txt_area, buf, NULL);
        }
    }
}

static void chart_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_PRESSED || code == LV_EVENT_RELEASED) {
        lv_obj_invalidate(obj); /*To make the value boxes visible*/
    }
    else if(code == LV_EVENT_DRAW_PART_BEGIN) {
        lv_obj_draw_part_dsc_t * dsc = (lv_obj_draw_part_dsc_t *) lv_event_get_param(e);
        /*Set the markers' text*/
        if(dsc->part == LV_PART_TICKS && dsc->id == LV_CHART_AXIS_PRIMARY_X) {
            if(lv_chart_get_type(obj) == LV_CHART_TYPE_BAR) {
                const char * month[] = {"I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI", "XII"};
                lv_snprintf(dsc->text, dsc->text_length, "%s", month[dsc->value]);
            }
            else {
                const char * month[] = {"Jan", "Febr", "March", "Apr", "May", "Jun", "July", "Aug", "Sept", "Oct", "Nov", "Dec"};
                lv_snprintf(dsc->text, dsc->text_length, "%s", month[dsc->value]);
            }
        }

        /*Add the faded area before the lines are drawn */
        else if(dsc->part == LV_PART_ITEMS) {
#if LV_DRAW_COMPLEX
            /*Add  a line mask that keeps the area below the line*/
            if(dsc->p1 && dsc->p2) {
                lv_draw_mask_line_param_t line_mask_param;
                lv_draw_mask_line_points_init(&line_mask_param, dsc->p1->x, dsc->p1->y, dsc->p2->x, dsc->p2->y,
                                              LV_DRAW_MASK_LINE_SIDE_BOTTOM);
                int16_t line_mask_id = lv_draw_mask_add(&line_mask_param, NULL);

                /*Add a fade effect: transparent bottom covering top*/
                lv_coord_t h = lv_obj_get_height(obj);
                lv_draw_mask_fade_param_t fade_mask_param;
                lv_draw_mask_fade_init(&fade_mask_param, &obj->coords, LV_OPA_COVER, obj->coords.y1 + h / 8, LV_OPA_TRANSP,
                                       obj->coords.y2);
                int16_t fade_mask_id = lv_draw_mask_add(&fade_mask_param, NULL);

                /*Draw a rectangle that will be affected by the mask*/
                lv_draw_rect_dsc_t draw_rect_dsc;
                lv_draw_rect_dsc_init(&draw_rect_dsc);
                draw_rect_dsc.bg_opa = LV_OPA_50;
                draw_rect_dsc.bg_color = dsc->line_dsc->color;

                lv_area_t obj_clip_area;
                _lv_area_intersect(&obj_clip_area, dsc->draw_ctx->clip_area, &obj->coords);
                const lv_area_t * clip_area_ori = dsc->draw_ctx->clip_area;
                dsc->draw_ctx->clip_area = &obj_clip_area;
                lv_area_t a;
                a.x1 = dsc->p1->x;
                a.x2 = dsc->p2->x - 1;
                a.y1 = LV_MIN(dsc->p1->y, dsc->p2->y);
                a.y2 = obj->coords.y2;
                lv_draw_rect(dsc->draw_ctx, &draw_rect_dsc, &a);
                dsc->draw_ctx->clip_area = clip_area_ori;
                /*Remove the masks*/
                lv_draw_mask_remove_id(line_mask_id);
                lv_draw_mask_remove_id(fade_mask_id);
            }
#endif


            const lv_chart_series_t * ser = (const lv_chart_series_t *) dsc->sub_part_ptr;

            if(lv_chart_get_pressed_point(obj) == dsc->id) {
                if(lv_chart_get_type(obj) == LV_CHART_TYPE_LINE) {
                    dsc->rect_dsc->outline_color = lv_color_white();
                    dsc->rect_dsc->outline_width = 2;
                }
                else {
                    dsc->rect_dsc->shadow_color = ser->color;
                    dsc->rect_dsc->shadow_width = 15;
                    dsc->rect_dsc->shadow_spread = 0;
                }

                char buf[8];
                lv_snprintf(buf, sizeof(buf), "%"LV_PRIu32, dsc->value);

                lv_point_t text_size;
                lv_txt_get_size(&text_size, buf, font_normal, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

                lv_area_t txt_area;
                if(lv_chart_get_type(obj) == LV_CHART_TYPE_BAR) {
                    txt_area.y2 = dsc->draw_area->y1 - LV_DPX(15);
                    txt_area.y1 = txt_area.y2 - text_size.y;
                    if(ser == lv_chart_get_series_next(obj, NULL)) {
                        txt_area.x1 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2;
                        txt_area.x2 = txt_area.x1 + text_size.x;
                    }
                    else {
                        txt_area.x2 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2;
                        txt_area.x1 = txt_area.x2 - text_size.x;
                    }
                }
                else {
                    txt_area.x1 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2 - text_size.x / 2;
                    txt_area.x2 = txt_area.x1 + text_size.x;
                    txt_area.y2 = dsc->draw_area->y1 - LV_DPX(15);
                    txt_area.y1 = txt_area.y2 - text_size.y;
                }

                lv_area_t bg_area;
                bg_area.x1 = txt_area.x1 - LV_DPX(8);
                bg_area.x2 = txt_area.x2 + LV_DPX(8);
                bg_area.y1 = txt_area.y1 - LV_DPX(8);
                bg_area.y2 = txt_area.y2 + LV_DPX(8);

                lv_draw_rect_dsc_t rect_dsc;
                lv_draw_rect_dsc_init(&rect_dsc);
                rect_dsc.bg_color = ser->color;
                rect_dsc.radius = LV_DPX(5);
                lv_draw_rect(dsc->draw_ctx, &rect_dsc, &bg_area);

                lv_draw_label_dsc_t label_dsc;
                lv_draw_label_dsc_init(&label_dsc);
                label_dsc.color = lv_color_white();
                label_dsc.font = font_normal;
                lv_draw_label(dsc->draw_ctx, &label_dsc, &txt_area,  buf, NULL);
            }
            else {
                dsc->rect_dsc->outline_width = 0;
                dsc->rect_dsc->shadow_width = 0;
            }
        }
    }
}
#endif
