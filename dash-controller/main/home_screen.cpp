/**
 * @file lv_demo_widgets.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

extern "C"
{
#include "lvgl.h"
#include "esp_log.h"
#include "esp_err.h"
}

#include "ta_json.hpp"

// #include <stdio.h>
// #include <string.h>

/*********************
 *      DEFINES
 *********************/

static const char *TAG = "home_screen";

/**********************
 *      TYPEDEFS
 **********************/
typedef enum
{
    DISP_SMALL,
    DISP_MEDIUM,
    DISP_LARGE,
} disp_size_t;

namespace speedo
{
    lv_obj_t *speed_label;
    lv_obj_t *units_label;
    lv_obj_t *meter;
    lv_meter_scale_t *scale;
    lv_meter_indicator_t *indic_needle;
    lv_meter_indicator_t *indic_arc;
    lv_meter_indicator_t *indic_scale_lines;
    lv_meter_indicator_t *indic_arc_high;
    lv_meter_indicator_t *indic_scale_lines_high;

    void update();
    void set_value(int32_t v);
};

namespace hvac
{
    bool fan_on = false;
    uint8_t fan_speed = 0; // 0-3
    lv_obj_t *fan_speed_button;
    lv_obj_t *temp_slider;
    lv_obj_t *zone_slider;

    bool ac_on = false;

    void fan_speed_event_cb(lv_event_t *e);
    void fan_toggle_event_cb(lv_event_t *e);

    void send_fan_json();

    void ac_toggle_event_cb(lv_event_t *e);
    void send_ac_json();

    void temp_slider_event_cb(lv_event_t *e);
    void zone_slider_event_cb(lv_event_t *e);

    void store();
    void load();
};

namespace relays
{
    void send_json(uint8_t relay_num, const String &cmd);
    void close(uint8_t relay_num);

    void btn_event_cb(lv_event_t *e);
}

namespace air_ride
{
    static const uint8_t mode_cnt = 3;
    static const uint8_t mode_travel = 0;
    static const uint8_t mode_level = 1;
    static const uint8_t mode_manual = 2;

    uint8_t mode = 0; // 0-2, travel, level, manual
    lv_obj_t *button[2]; // manual mode does not have a mode button
    lv_obj_t *manual_button[2][2];

    void create_mode_button(lv_obj_t *parent, uint32_t m);
    void set_mode(uint8_t new_mode);
    void send_mode_json();

    void create_manual_button(lv_obj_t *parent, bool left, bool up);
    lv_obj_t *manual_button_obj(bool left, bool up);

    void mode_event_cb(lv_event_t *e);
    void manual_event_cb(lv_event_t *e);
}

namespace status_inputs
{
    class status_input_t
    {
    public:
        String onLabel;
        String offLabel;
        lv_obj_t *status;
        lv_obj_t *status_label;

        void set(bool on = true);
    };

    static const uint8_t input_cnt = 3;
    status_input_t status_input[input_cnt];
}

namespace servos
{
    class servo_t
    {
    public:
        lv_obj_t *open_label;
        lv_obj_t *close_label;
        lv_obj_t *current_label;

        uint16_t open_pulse = 400;
        uint16_t close_pulse = 600;
        uint16_t current_pulse = 500;

        void adjust(int16_t rel_adjust);

        void open();
        void close();
        void set(uint8_t position);      // position is percentage open
        void set_abs(uint16_t position); // position is absolute position between 0 and 1000

        void get_pulse_json(uint8_t servo_num, Buffer &buffer, bool config = false);

        void update();
    };

    static const uint8_t servo_cnt = 4;
    servo_t servo[servo_cnt];

    void get_config_json(Buffer &buffer);
    void send_pulse_json(uint8_t servo_num);
    void send_all_pulses_json();

    void update();

    void btn_event_cb(lv_event_t *e);
}

namespace config
{
    typedef enum
    {
        UNITS_METRIC,
        UNITS_BRITISH,
    } units_t;

    units_t units = UNITS_BRITISH;
    lv_palette_t palette;

    void store();
    void load();

    void units_event_cb(lv_event_t *e);
}

/**********************
 *  STATIC PROTOTYPES
 **********************/
// static void accessories_create(lv_obj_t * parent);
static void main_create(lv_obj_t *parent);
static void settings_create(lv_obj_t *parent);
static void color_changer_create(lv_obj_t *parent);

// static lv_obj_t * create_meter_box(lv_obj_t * parent, const char * title, const char * text1, const char * text2,
//                                    const char * text3);

static void color_changer_event_cb(lv_event_t *e);
static void color_event_cb(lv_event_t *e);
// static void ta_event_cb(lv_event_t * e);
/*
static void birthday_event_cb(lv_event_t * e);
static void calendar_event_cb(lv_event_t * e);
static void slider_event_cb(lv_event_t * e);
static void chart_event_cb(lv_event_t * e);
*/

extern "C"
{
    esp_err_t write_file(const char *filename, const char *data, int datalen);
    int read_file(const char *filename, char *buffer, int bufferlen);
    esp_err_t i2c_master_send_data(uint8_t *data, size_t len);

    extern const lv_img_dsc_t temperature_bar_lg;
    extern const lv_img_dsc_t HVAC_icons_bar;
}

/**********************
 *  STATIC VARIABLES
 **********************/
static disp_size_t disp_size;

static lv_obj_t *tv;
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

static const lv_font_t *font_large;
static const lv_font_t *font_normal;

/*
static uint32_t session_desktop = 1000;
static uint32_t session_tablet = 1000;
static uint32_t session_mobile = 1000;
*/

// static lv_timer_t * meter2_timer;

// static speedo_t speedo;
//  static config config;

static const lv_coord_t default_btn_width = 94;
static const lv_coord_t default_btn_height = 50;

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

static esp_err_t write_file(const String &filename, const String &data)
{
    return write_file(filename.Pch(), data.Pch(), data.Len());
}

static esp_err_t read_file(const String &filename, Buffer &buffer)
{
    int bytes_read;

    buffer.Reset();
    bytes_read = read_file(filename.Pch(), buffer.Pch(), buffer.MaxLen() - 1);
    if (bytes_read > 0)
    {
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
    if (!meter || !indic_needle)
    {
        return;
    }
    lv_meter_set_indicator_value(meter, indic_needle, v);
    lv_label_set_text_fmt(speed_label, "%d", (int)v);
}

void hvac::send_ac_json()
{
    json_buff = "{\"ac\":{\"state\":\"";
    json_buff += ac_on ? "on" : "off";
    json_buff += "\"}}\n\0";
    i2c_master_send_data((uint8_t *)json_buff.Pch(), json_buff.Len());
}

void hvac::send_fan_json()
{
    json_buff = "{\"fan\":{\"speed\":\"";
    json_buff.StrFromInt(fan_speed);
    json_buff += "\",\"state\":\"";
    json_buff += fan_on ? "on" : "off";
    json_buff += "\"}}\n\0";

    i2c_master_send_data((uint8_t *)json_buff.Pch(), json_buff.Len());
}

void hvac::fan_speed_event_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);

    // Assuming the label is the first child of the button
    lv_obj_t *label = lv_obj_get_child(btn, 0);

    fan_speed = (fan_speed + 1) % 4; // Cycle through 0-3

    switch (fan_speed)
    {
    case 0:
        lv_label_set_text(label, "Low");
        break;
    case 1:
        lv_label_set_text(label, "Med Low");
        break;
    case 2:
        lv_label_set_text(label, "Med");
        break;
    case 3:
        lv_label_set_text(label, "High");
        break;

    default:
        break;
    }

    send_fan_json();
}

void hvac::fan_toggle_event_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);

    // Assuming the label is the first child of the button
    lv_obj_t *label = lv_obj_get_child(btn, 0);

    if (fan_on)
    {
        lv_label_set_text(label, "Fan Off");
        lv_obj_set_style_bg_color(btn, lv_palette_darken(config::palette, 4), 0);
        lv_obj_set_style_bg_color(fan_speed_button, lv_palette_darken(config::palette, 4), 0);
    }
    else
    {
        lv_label_set_text(label, "Fan On");
        lv_obj_set_style_bg_color(btn, lv_palette_main(config::palette), 0);
        lv_obj_set_style_bg_color(fan_speed_button, lv_palette_main(config::palette), 0);
    }
    fan_on = !fan_on;

    send_fan_json();
}

void hvac::ac_toggle_event_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);

    // Assuming the label is the first child of the button
    lv_obj_t *label = lv_obj_get_child(btn, 0);

    if (ac_on)
    {
        lv_label_set_text(label, "A/C Off");
        lv_obj_set_style_bg_color(btn, lv_palette_darken(config::palette, 4), 0);
    }
    else
    {
        lv_label_set_text(label, "A/C On");
        lv_obj_set_style_bg_color(btn, lv_palette_main(config::palette), 0);
    }

    ac_on = !ac_on;

    send_ac_json();
}

void hvac::temp_slider_event_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int value = lv_slider_get_value(slider);
    servos::servo[0].set(value);
    servos::send_pulse_json(0);
}

void hvac::zone_slider_event_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int value = lv_slider_get_value(slider);
    servos::servo[1].set(value <= 25 ? 100 
                        : value <= 50 ? 100 - ((value - 25) * 4)
                        : value <= 75 ? (value - 50) * 4
                        : 100 - ((value - 75) * 4));
    servos::servo[2].set(value <= 25 ? value * 4 
                        : value <= 50 ? 100
                        : value <= 75 ? 100 - ((value - 50) * 4)
                        : 0);
    servos::servo[3].set(value <= 50 ? 0
                        : value <= 75 ? (value - 50) * 4
                        : 100);
    servos::send_all_pulses_json();
}

void hvac::store()
{
    ESP_LOGI(TAG, "%s", __FUNCTION__);

    json_buff = "{\"temp\":";
    json_buff.StrFromInt(lv_slider_get_value(temp_slider));
    json_buff += ",\"zone\":";
    json_buff.StrFromInt(lv_slider_get_value(zone_slider));
    json_buff += "}";
    write_file("hvac.jsn", json_buff);
}

void hvac::load()
{
    ESP_LOGI(TAG, "%s", __FUNCTION__);

    if (read_file("hvac.jsn", json_buff) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to load hvac.jsn");
        return;
    }
    ESP_LOGI(TAG, "load read: %s", json_buff.Pch());
    if (!json.Parse(json_buff))
    {
        ESP_LOGE(TAG, "Error parsing hvac JSON: \n%s", json.ErrorString().Pch());
        return;
    }

    JToken *pJToken = json.Root()->Find("temp");
    if (pJToken && pJToken->Child())
    {
        int temp_value = pJToken->Child()->Value().StrToInt();
        lv_slider_set_value(temp_slider, temp_value, LV_ANIM_OFF);
    }

    pJToken = json.Root()->Find("zone");
    if (pJToken && pJToken->Child())
    {
        int zone_value = pJToken->Child()->Value().StrToInt();
        lv_slider_set_value(zone_slider, zone_value, LV_ANIM_OFF);
    }
}

void relays::send_json(uint8_t relay_num, const String &cmd)
{
    json_buff = "{\"relay\":{\"num\":\"";
    json_buff.StrFromInt(relay_num);
    json_buff += "\",\"state\":\"";
    json_buff += cmd;
    json_buff += "\"}}\n\0";

    i2c_master_send_data((uint8_t *)json_buff.Pch(), json_buff.Len());
}

void relays::btn_event_cb(lv_event_t *e)
{
    char *data = (char *)e->user_data;
    uint8_t relay_num = data[0] - '0';

    switch (e->code)
    {
    case LV_EVENT_CLICKED:
    {
        send_json(relay_num, data[2] == 'n' ? "on" : "off");
        break;
    }
    default:
    {
        break;
    }
    }
}

void air_ride::create_manual_button(lv_obj_t *parent, bool left, bool up)
{
    uint32_t m = (left ? 0 : 1) + (up ? 0 : 2); // 0: left up, 1: right up, 2: left down, 3: right down
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, default_btn_width, default_btn_height);
    lv_obj_set_style_bg_color(btn, lv_palette_main(config::palette), 0);
    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, left ? (up ? "Left Up" : "Left Dn") : (up ? "Right Up" : "Right Dn"));
    lv_obj_center(lbl);
    lv_obj_add_event_cb(btn, manual_event_cb, LV_EVENT_PRESSED, (void *)m);
    lv_obj_add_event_cb(btn, manual_event_cb, LV_EVENT_RELEASED, (void *)m);
    manual_button[left][up] = btn;
}

lv_obj_t *air_ride::manual_button_obj(bool left, bool up)
{
    return manual_button[left][up];
}

void air_ride::create_mode_button(lv_obj_t *parent, uint32_t m)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, default_btn_width, default_btn_height);
    lv_obj_set_style_bg_color(btn, lv_palette_darken(config::palette, 4), 0);
    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, m == mode_travel ? "Travel" : "Level");
    lv_obj_center(lbl);
    lv_obj_add_event_cb(btn, mode_event_cb, LV_EVENT_CLICKED, (void *)m);
    button[m] = btn;
}

void air_ride::set_mode(uint8_t new_mode)
{
    if (new_mode >= mode_cnt)
    {
        return;
    }
    mode = new_mode;

    for (uint8_t i = 0; i < mode_cnt - 1; i++)
    {
        lv_obj_set_style_bg_color(button[i], lv_palette_darken(config::palette, 4), 0);
    }

    if (mode < mode_manual) {
        lv_obj_set_style_bg_color(button[mode], lv_palette_main(config::palette), 0);
    }

    for (uint8_t i = 0; i < 4; i++)
    {
        lv_obj_set_style_bg_color(manual_button[i / 2][i % 2], mode == mode_manual ? lv_palette_main(config::palette) : lv_palette_darken(config::palette, 4), 0);
    }

    // Only send mode JSON if not in manual mode
    if (mode < mode_manual) {
        send_mode_json();
    }
}

void air_ride::send_mode_json()
{
    json_buff = "{\"airRide\":{\"mode\":\"";
    json_buff += (mode == mode_travel   ? "travel"
                  : mode == mode_level ? "level"
                              : "manual");
    json_buff += "\"}}\n\0";

    i2c_master_send_data((uint8_t *)json_buff.Pch(), json_buff.Len());
}

void air_ride::mode_event_cb(lv_event_t *e)
{
    uint8_t new_mode = (uint8_t)((uint32_t)e->user_data);

    if (new_mode > 2)
    {
        ESP_LOGE(TAG, "Invalid mode: %d", new_mode);
        return;
    }

    set_mode(new_mode);
}

void air_ride::manual_event_cb(lv_event_t *e)
{
    uint8_t m = (uint8_t)((uint32_t)e->user_data);

    if (m > 3)
    {
        ESP_LOGE(TAG, "Invalid manual mode: %d", m);
        return;
    }

    if (mode != 2)
    {
        ESP_LOGW(TAG, "Manual control requested while not in manual mode, switching to manual mode.");
        set_mode(2);
    }

    if (e->code == LV_EVENT_RELEASED)
    {
        json_buff = "{\"airRide\":{\"manual\":\"stop\"}}\n\0";
    }
    else if (e->code == LV_EVENT_PRESSED)
    {
        json_buff = "{\"airRide\":{\"manual\":{\"";
        json_buff += ((m & 1) == 0 ? "left" : "right");
        json_buff += "\":\"";
        json_buff += ((m & 2) == 0 ? "up" : "down");
        json_buff += "\"}}}\n\0";
    }
    else
    {
        ESP_LOGE(TAG, "Unexpected event code: %d", e->code);
        return;
    }

    i2c_master_send_data((uint8_t *)json_buff.Pch(), json_buff.Len());
}

void status_inputs::status_input_t::set(bool on)
{
    lv_obj_set_style_bg_color(status, lv_palette_darken(on ? LV_PALETTE_RED : LV_PALETTE_GREEN, 4), 0);
    lv_label_set_text(status_label, on ? onLabel.Pch() : offLabel.Pch());
}

void servos::servo_t::open()
{
    current_pulse = open_pulse;
}

void servos::servo_t::close()
{
    current_pulse = close_pulse;
}

void servos::servo_t::set(uint8_t position)
{
    if (position > 100)
    {
        position = 100;
    }

    current_pulse = open_pulse + (close_pulse - open_pulse) * (100 - position) / 100;    
}

void servos::servo_t::set_abs(uint16_t position)
{
    if (position > 1000)
    {
        position = 1000;
    }

    current_pulse = position;
}

void servos::servo_t::get_pulse_json(uint8_t servo_num, Buffer &buffer, bool config)
{
    buffer += "{\"num\":";
    buffer.StrFromInt(servo_num + 12); // using servos 12 to 15 as 0 to 4
    if (config)
    {
        buffer += ",\"open_pulse\":";
        buffer.StrFromInt(open_pulse);
        buffer += ",\"close_pulse\":";
        buffer.StrFromInt(close_pulse);
    } else {
        buffer += ",\"pulse\":";
        buffer.StrFromInt(current_pulse);
    }
    buffer += '}';
}

void servos::get_config_json(Buffer &buffer)
{
    buffer += "\"servos\":[";
    for (uint8_t i = 0; i < servo_cnt; i++)
    {
        if (i > 0)
        {
            buffer += ',';
        }
        servo[i].get_pulse_json(i, buffer, true);
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

void servos::send_all_pulses_json()
{
    json_buff = "{\"servos\":[";
    for (uint8_t i = 0; i < servo_cnt; i++)
    {
        if (i > 0)
        {
            json_buff += ',';
        }
        servo[i].get_pulse_json(i, json_buff);
    }
    json_buff += "]}\n\0";

    i2c_master_send_data((uint8_t *)json_buff.Pch(), json_buff.Len());
}

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
    if (current_pulse + rel_adjust >= 1000)
    {
        current_pulse = 1000;
    }
    else if ((int16_t)current_pulse + rel_adjust <= 0)
    {
        current_pulse = 0;
    }
    else
    {
        current_pulse += rel_adjust;
    }
    update();
}

void servos::btn_event_cb(lv_event_t *e)
{
    static bool press_hold = false;
    char *data = (char *)e->user_data;
    uint8_t servo_num = data[0] - '0';

    switch (e->code)
    {
    case LV_EVENT_PRESSING:
    {
        if (press_hold)
        {
            servo[servo_num].adjust(data[1] == 'u' ? 10 : data[1] == 'd' ? -10
                                                                         : 0);
            //                servos::send_pulse_json(servo_num);
        }
        break;
    }
    case LV_EVENT_LONG_PRESSED:
    {
        press_hold = true;
        break;
    }
    case LV_EVENT_RELEASED:
    {
        if (!press_hold)
        {
            servo[servo_num].adjust(data[1] == 'u' ? 10 : data[1] == 'd' ? -10
                                                                         : 0);
        }
        servos::send_pulse_json(servo_num);
        press_hold = false;
        break;
    }
    case LV_EVENT_CLICKED:
    {
        if (data[1] == 'o')
        {
            servo[servo_num].open_pulse = servo[servo_num].current_pulse;
        }
        else if (data[1] == 'c')
        {
            servo[servo_num].close_pulse = servo[servo_num].current_pulse;
        }
        else
        {
            return;
        }
        servo[servo_num].update();

        config::store();
        break;
    }
    default:
    {
        break;
    }
    }
}

void config::store()
{
    ESP_LOGI(TAG, "%s", __FUNCTION__);

    json_buff = "{\"units\":\"";
    json_buff += (units == UNITS_BRITISH ? "british" : "metric");
    json_buff += "\",\"palette\":";
    json_buff.StrFromInt((int32_t)palette);
    json_buff += ',';
    servos::get_config_json(json_buff);
    json_buff += "}";
    write_file("config.jsn", json_buff);
}

void config::load()
{
    ESP_LOGI(TAG, "%s", __FUNCTION__);

    if (read_file("config.jsn", json_buff) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to load config");
        return;
    }
    ESP_LOGI(TAG, "load read: %s", json_buff.Pch());
    if (!json.Parse(json_buff))
    {
        ESP_LOGE(TAG, "Error parsing config JSON: \n%s", json.ErrorString().Pch());
        return;
    }

    units = (json.Root()->Find("units", "metric") ? UNITS_METRIC : UNITS_BRITISH);

    palette = LV_PALETTE_BLUE;
    JToken *pJTokenPalette = json.Root()->Find("palette");
    if (pJTokenPalette)
    {
        palette = (lv_palette_t)pJTokenPalette->Child()->Value().StrToInt();
        ESP_LOGI(TAG, "Read palette %d", palette);
    }

    JToken *pJTokenServo = json.Root()->Find("servos");
    for (uint8_t i = 0; pJTokenServo && i < 2; i++)
    {
        pJTokenServo = pJTokenServo->Child();
    }

    while (pJTokenServo && pJTokenServo->Child())
    {
        JToken *pJToken = pJTokenServo->Child()->Find("num");
        uint8_t servo_num = 0;

        if (pJTokenServo->Find("num"))
        {
            servo_num = pJToken->Child()->Value().StrToInt() - 12; // servos 12 to 15
            if (servo_num >= servos::servo_cnt)
            {
                ESP_LOGE(TAG, "Invalid servo number %d, skipping", servo_num);
                continue;
            }
        }

        pJToken = pJTokenServo->Child()->Find("open_pulse");

        if (pJToken && pJToken->Child())
        {
            servos::servo[servo_num].open_pulse = pJToken->Child()->Value().StrToInt();
        }

        pJToken = pJTokenServo->Child()->Find("close_pulse");

        if (pJToken && pJToken->Child())
        {
            servos::servo[servo_num].close_pulse = pJToken->Child()->Value().StrToInt();
        }

        pJTokenServo = pJTokenServo->Next();
    }
}

void config::units_event_cb(lv_event_t *e)
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

    if (LV_HOR_RES <= 320)
        disp_size = DISP_SMALL;
    else if (LV_HOR_RES < 720)
        disp_size = DISP_MEDIUM;
    else
        disp_size = DISP_LARGE;

    ESP_LOGI(TAG, "disp_size %d", disp_size);

    lv_coord_t tab_h;
    tab_h = 45;
    font_normal = &lv_font_montserrat_14;
    font_large = &lv_font_montserrat_24;

    lv_palette_t palette_secondary = (lv_palette_t)(config::palette + 3); /*Use another palette as secondary*/
    if (palette_secondary >= _LV_PALETTE_LAST)
        palette_secondary = (lv_palette_t)0;
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
//    lv_style_set_border_color(&style_indicators, lv_palette_darken(LV_PALETTE_GREEN, 4));
  //  lv_style_set_border_width(&style_indicators, 2);
    lv_style_set_pad_all(&style_indicators, 5); // Padding around the label

    tv = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, tab_h);
    lv_obj_set_style_text_font(lv_scr_act(), font_normal, 0);
    lv_obj_t *tab_content = lv_tabview_get_content(tv);
    // Disable the swipe-to-switch behavior by clearing the scrollable flag
    lv_obj_clear_flag(tab_content, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *t1 = lv_tabview_add_tab(tv, "Main");
    //    lv_obj_t * t2 = lv_tabview_add_tab(tv, "Accessories");
    lv_obj_t *t3 = lv_tabview_add_tab(tv, "Config");

    main_create(t1);
    //    accessories_create(t2);
    settings_create(t3);

    color_changer_create(t3);
}

extern "C"
{
    void run_home_screen(void)
    {
        home_screen();
    }

    void handle_message(const char *pch)
    {
        if (!json.Parse(pch))
        {
            ESP_LOGE(TAG, "Error parsing config JSON: \n%s", json.ErrorString().Pch());
            return;
        }

        JToken *pJToken;
        if (json.Root()->Find("info", "Status saved"))
        {
            hvac::store();
        }
        else if ((pJToken = json.Root()->Find("gps")))
        {
            pJToken = pJToken->Child();
            if (pJToken && (pJToken = pJToken->Find("speed")))
            {
                pJToken = pJToken->Child();
                if (pJToken && (pJToken = pJToken->Find(config::units == config::UNITS_METRIC ? "kmph" : "mph")))
                {
                    speedo::set_value((int32_t)pJToken->ChildValue().StrToInt());
                }
            }
        }
        else if ((pJToken = json.Root()->Find("12vInputs")))
        {
            for (uint8_t i = 0; i < status_inputs::input_cnt; i++)
            {
                status_inputs::status_input[i].set(pJToken->Child()->Value().StrToInt() & (1 << i));
            }
        }
        else if ((pJToken = json.Root()->Find("servos")))
        {
            JToken *pJTokenServo = pJToken;
            for (uint8_t i = 0; pJTokenServo && i < 2; i++)
            {
                pJTokenServo = pJTokenServo->Child();
            }

            while (pJTokenServo && pJTokenServo->Child())
            {
                JToken *pJToken = pJTokenServo->Child()->Find("num");
                uint8_t servo_num = 0;

                if (pJTokenServo->Find("num"))
                {
                    servo_num = pJToken->Child()->Value().StrToInt() - 12; // servos 12 to 15
                    if (servo_num >= servos::servo_cnt)
                    {
                        ESP_LOGE(TAG, "Invalid servo number %d, skipping", servo_num);
                        continue;
                    }
                }

                pJToken = pJTokenServo->Child()->Find("pulse");

                if (pJToken && pJToken->Child())
                {
                    servos::servo[servo_num].current_pulse = pJToken->Child()->Value().StrToInt();
                    servos::servo[servo_num].update();
                }

                pJTokenServo = pJTokenServo->Next();
            }
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

static void main_create(lv_obj_t *parent)
{
    ESP_LOGI(TAG, "%s", __FUNCTION__);

    //    char temp[128];
    //    Buffer temp_buffer(temp, sizeof(temp));

    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_t *cont = lv_obj_create(parent);
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
    lv_obj_t *units_btn = lv_btn_create(speedo::meter);
    lv_obj_set_height(units_btn, LV_SIZE_CONTENT);
    lv_obj_add_event_cb(units_btn, config::units_event_cb, LV_EVENT_CLICKED, NULL);

    speedo::units_label = lv_label_create(units_btn);
    speedo::update();

    lv_obj_update_layout(parent);
    lv_obj_set_size(speedo::meter, 240, 240);
    lv_obj_align(speedo::speed_label, LV_ALIGN_CENTER, 0, lv_pct(0));
    lv_obj_align_to(units_btn, speedo::speed_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 50);

    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_set_width(panel, lv_pct(35));
    lv_obj_set_height(panel, 280);

    for (int i = 0; i < status_inputs::input_cnt; i++)
    {
        lv_obj_t *status = lv_obj_create(panel);
        lv_obj_add_style(status, &style_indicators, 0);
        lv_obj_set_size(status, 110, 60); // Size of the square
        status_inputs::status_input[i].status = status;
        if (i == 0) {
            lv_obj_align(status, LV_ALIGN_TOP_MID, 0, 0);
        } else {
            lv_obj_align_to(status, status_inputs::status_input[i - 1].status, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
        }

        // Create a label for the status indicator
        lv_obj_t *label = lv_label_create(status);
        status_inputs::status_input[i].onLabel = (i == 0 ? "Low Fuel" : i == 1 ? "Low Coolant" : "Brake On");
        status_inputs::status_input[i].offLabel = (i == 0 ? "Fuel" : i == 1 ? "Coolant" : "Brake Off");
        lv_label_set_text(label, status_inputs::status_input[i].offLabel.Pch());
        lv_obj_center(label); // Center the label within the container
        status_inputs::status_input[i].status_label = label;
    }

    panel = lv_obj_create(parent);
    lv_obj_set_width(panel, lv_pct(100));
    lv_obj_set_height(panel, 260);

    lv_obj_t *fan_btn = lv_btn_create(panel);
    lv_obj_set_size(fan_btn, default_btn_width, default_btn_height);
    lv_obj_set_style_bg_color(fan_btn, lv_palette_darken(config::palette, 4), 0);
    lv_obj_t *fan_label = lv_label_create(fan_btn);
    lv_label_set_text(fan_label, "Fan Off");
    lv_obj_center(fan_label);
    lv_obj_add_event_cb(fan_btn, hvac::fan_toggle_event_cb, LV_EVENT_CLICKED, NULL);

    hvac::fan_speed_button = lv_btn_create(panel);
    lv_obj_set_size(hvac::fan_speed_button, default_btn_width, default_btn_height);
    lv_obj_align_to(hvac::fan_speed_button, fan_btn, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_set_style_bg_color(hvac::fan_speed_button, lv_palette_darken(config::palette, 4), 0);
    lv_obj_t *fan_speed_label = lv_label_create(hvac::fan_speed_button);
    lv_label_set_text(fan_speed_label, "Low");
    lv_obj_center(fan_speed_label);
    lv_obj_add_event_cb(hvac::fan_speed_button, hvac::fan_speed_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *ac_btn = lv_btn_create(panel);
    lv_obj_set_size(ac_btn, default_btn_width, default_btn_height);
    lv_obj_set_style_bg_color(ac_btn, lv_palette_darken(config::palette, 4), 0);
    lv_obj_align_to(ac_btn, panel, LV_ALIGN_TOP_RIGHT, 0, 0);
    //    lv_obj_align_to(ac_btn, speedo::meter, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_t *ac_label = lv_label_create(ac_btn);
    lv_label_set_text(ac_label, "A/C Off");
    lv_obj_center(ac_label);
    lv_obj_add_event_cb(ac_btn, hvac::ac_toggle_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *img = lv_img_create(panel);
    // Set the image source to the C array image
    lv_img_set_src(img, &temperature_bar_lg); // Use the LVGL Image Converter C array here
    lv_obj_align_to(img, panel, LV_ALIGN_TOP_MID, 0, 70);

    // Create the temperature adjustment slider
    hvac::temp_slider = lv_slider_create(panel);
    lv_slider_set_range(hvac::temp_slider, 0, 100);
    lv_obj_set_width(hvac::temp_slider, lv_pct(90));
    lv_obj_set_height(hvac::temp_slider, 40);
    lv_obj_set_style_bg_opa(hvac::temp_slider, LV_OPA_TRANSP, LV_PART_MAIN);      // Make the main background transparent
    lv_obj_set_style_bg_opa(hvac::temp_slider, LV_OPA_TRANSP, LV_PART_INDICATOR); // Make the indicator (track) transparent

    lv_obj_align_to(hvac::temp_slider, img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(hvac::temp_slider, hvac::temp_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    img = lv_img_create(panel);
    // Set the image source to the C array image
    lv_img_set_src(img, &HVAC_icons_bar); // Use the LVGL Image Converter C array here
    lv_obj_align_to(img, hvac::temp_slider, LV_ALIGN_BOTTOM_MID, 0, 60);

    // Create the airflow zone slider
    hvac::zone_slider = lv_slider_create(panel);
    lv_slider_set_range(hvac::zone_slider, 0, 100); // Range from feet to defrost
    lv_obj_set_width(hvac::zone_slider, lv_pct(90));
    lv_obj_set_height(hvac::zone_slider, 40);
    //    lv_obj_set_style_bg_opa(hvac::zone_slider, LV_OPA_TRANSP, LV_PART_MAIN);  // Make the main background transparent
    lv_obj_set_style_bg_opa(hvac::zone_slider, LV_OPA_TRANSP, LV_PART_INDICATOR); // Make the indicator (track) transparent
                                                                            //    lv_obj_set_style_bg_opa(zone_slider, LV_OPA_50, LV_PART_KNOB);  // Make knob opaque
    lv_obj_set_style_radius(hvac::zone_slider, LV_RADIUS_CIRCLE, LV_PART_KNOB);   // Make knob circular

    lv_obj_align_to(hvac::zone_slider, img, LV_ALIGN_BOTTOM_MID, 0, 46);
    lv_obj_add_event_cb(hvac::zone_slider, hvac::zone_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    panel = lv_obj_create(parent);
    lv_obj_set_width(panel, lv_pct(100));
    lv_obj_set_height(panel, 150);

    air_ride::create_manual_button(panel, true, true);

    air_ride::create_manual_button(panel, true, false);
    lv_obj_align_to(air_ride::manual_button_obj(true, false), air_ride::manual_button_obj(true, true), LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    air_ride::create_manual_button(panel, false, true);
    lv_obj_align_to(air_ride::manual_button_obj(false, true), panel, LV_ALIGN_TOP_RIGHT, 0, 0);

    air_ride::create_manual_button(panel, false, false);
    lv_obj_align_to(air_ride::manual_button_obj(false, false), air_ride::manual_button_obj(false, true), LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

//    air_ride::button[0] = air_ride::create_mode_button(panel, 0, "Manual");
//    lv_obj_align_to(air_ride::button[0], air_ride::manual_button[1][0], LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    air_ride::create_mode_button(panel, air_ride::mode_travel);
    lv_obj_align_to(air_ride::button[air_ride::mode_travel], air_ride::manual_button_obj(true, true), LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    air_ride::create_mode_button(panel, air_ride::mode_level);
    lv_obj_align_to(air_ride::button[air_ride::mode_level], air_ride::manual_button_obj(false, true), LV_ALIGN_OUT_LEFT_MID, -10, 0);

    air_ride::set_mode(air_ride::mode_manual); // Set initial mode to Manual

    hvac::load();

    i2c_master_send_data((uint8_t *)"{\"req\":\"status\"}", strlen("{\"req\":\"status\"}"));
}

char *encode_action(uint8_t dev_num, const String &button)
{
    Buffer result(button.Len() + 5);
    result.StrFromInt(dev_num);
    result += button;

    return result.Pch();
}

void settings_create(lv_obj_t *parent)
{
    ESP_LOGI(TAG, "%s", __FUNCTION__);

    char temp[128];
    Buffer temp_buffer(temp, sizeof(temp));

    lv_obj_t *main_panel = lv_obj_create(parent);
    lv_obj_set_width(main_panel, lv_pct(100));
    //    lv_obj_set_height(panel, LV_SIZE_CONTENT);
    //    lv_obj_set_flex_grow(panel, 1);
    lv_obj_set_flex_flow(main_panel, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_t *title = lv_label_create(main_panel);
    lv_label_set_text(title, "Servo Config (DO NOT TOUCH!!!)");
    lv_obj_add_style(title, &style_title, 0);

    for (uint8_t i = 0; i < servos::servo_cnt; i++)
    {
        lv_obj_t *label;
        lv_obj_t *panel = lv_obj_create(main_panel);
        lv_obj_set_width(panel, lv_pct(100));
        lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_ROW_WRAP);

        label = lv_label_create(panel);
        temp_buffer = "I/O : ";
        temp_buffer.StrFromInt(i);
        lv_label_set_text(label, temp_buffer.Pch());
        lv_obj_add_style(label, &style_title, 0);

        lv_obj_t *btn = lv_btn_create(panel);
        lv_obj_set_height(btn, LV_SIZE_CONTENT);
        lv_obj_set_width(btn, lv_pct(28));
        lv_obj_add_event_cb(btn, relays::btn_event_cb, LV_EVENT_CLICKED, encode_action(i, "on"));

        label = lv_label_create(btn);
        lv_label_set_text(label, "Relay On");
        lv_obj_center(label);

        btn = lv_btn_create(panel);
        lv_obj_set_height(btn, LV_SIZE_CONTENT);
        lv_obj_set_width(btn, lv_pct(28));
        lv_obj_add_event_cb(btn, relays::btn_event_cb, LV_EVENT_CLICKED, encode_action(i, "off"));

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
        lv_obj_add_event_cb(btn, servos::btn_event_cb, LV_EVENT_PRESSING, encode_action(i, "up"));
        lv_obj_add_event_cb(btn, servos::btn_event_cb, LV_EVENT_LONG_PRESSED, encode_action(i, "up"));
        lv_obj_add_event_cb(btn, servos::btn_event_cb, LV_EVENT_RELEASED, encode_action(i, "up"));

        label = lv_label_create(btn);
        lv_label_set_text(label, LV_SYMBOL_UP);
        lv_obj_center(label);

        btn = lv_btn_create(panel);
        lv_obj_set_height(btn, LV_SIZE_CONTENT);
        lv_obj_set_width(btn, lv_pct(14));
        lv_obj_add_event_cb(btn, servos::btn_event_cb, LV_EVENT_PRESSING, encode_action(i, "down"));
        lv_obj_add_event_cb(btn, servos::btn_event_cb, LV_EVENT_LONG_PRESSED, encode_action(i, "down"));
        lv_obj_add_event_cb(btn, servos::btn_event_cb, LV_EVENT_RELEASED, encode_action(i, "down"));

        label = lv_label_create(btn);
        lv_label_set_text(label, LV_SYMBOL_DOWN);
        lv_obj_center(label);

        btn = lv_btn_create(panel);
        lv_obj_set_height(btn, LV_SIZE_CONTENT);
        lv_obj_set_width(btn, lv_pct(28));
        lv_obj_add_event_cb(btn, servos::btn_event_cb, LV_EVENT_CLICKED, encode_action(i, "open"));

        label = lv_label_create(btn);
        lv_label_set_text(label, "Set Open");
        lv_obj_center(label);

        btn = lv_btn_create(panel);
        lv_obj_set_height(btn, LV_SIZE_CONTENT);
        lv_obj_set_width(btn, lv_pct(28));
        lv_obj_add_event_cb(btn, servos::btn_event_cb, LV_EVENT_CLICKED, encode_action(i, "close"));

        label = lv_label_create(btn);
        lv_label_set_text(label, "Set Close");
        lv_obj_center(label);

        lv_obj_set_height(panel, LV_SIZE_CONTENT);
    }

    lv_obj_set_height(main_panel, LV_SIZE_CONTENT);
}

static void color_changer_create(lv_obj_t *parent)
{
    ESP_LOGI(TAG, "%s", __FUNCTION__);

    static lv_palette_t palette[] = {
        LV_PALETTE_BLUE, LV_PALETTE_GREEN, LV_PALETTE_BLUE_GREY, LV_PALETTE_ORANGE,
        LV_PALETTE_RED, LV_PALETTE_PURPLE, LV_PALETTE_TEAL, _LV_PALETTE_LAST};

    lv_obj_t *color_cont = lv_obj_create(parent);
    lv_obj_remove_style_all(color_cont);
    lv_obj_set_flex_flow(color_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(color_cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(color_cont, LV_OBJ_FLAG_FLOATING);

    lv_obj_set_style_bg_color(color_cont, lv_color_black(), 0);
    lv_obj_set_style_pad_right(color_cont, true ? LV_DPX(47) : LV_DPX(55), 0);
    lv_obj_set_style_bg_opa(color_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(color_cont, LV_RADIUS_CIRCLE, 0);

    if (disp_size == DISP_SMALL)
        lv_obj_set_size(color_cont, LV_DPX(52), LV_DPX(52));
    else
        lv_obj_set_size(color_cont, LV_DPX(60), LV_DPX(60));

    lv_obj_align(color_cont, LV_ALIGN_BOTTOM_RIGHT, -LV_DPX(10), -LV_DPX(10));

    uint32_t i;
    for (i = 0; palette[i] != _LV_PALETTE_LAST; i++)
    {
        lv_obj_t *c = lv_btn_create(color_cont);
        lv_obj_set_style_bg_color(c, lv_palette_main(palette[i]), 0);
        lv_obj_set_style_radius(c, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_opa(c, LV_OPA_TRANSP, 0);
        lv_obj_set_size(c, 20, 20);
        lv_obj_add_event_cb(c, color_event_cb, LV_EVENT_ALL, &palette[i]);
        lv_obj_clear_flag(c, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    }

    lv_obj_t *btn = lv_btn_create(parent);
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

static void color_changer_anim_cb(void *var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    lv_coord_t max_w = lv_obj_get_width(lv_obj_get_parent(obj)) - LV_DPX(20);
    lv_coord_t w;

    if (disp_size == DISP_SMALL)
    {
        w = lv_map(v, 0, 256, LV_DPX(52), max_w);
        lv_obj_set_width(obj, w);
        lv_obj_align(obj, LV_ALIGN_BOTTOM_RIGHT, -LV_DPX(10), -LV_DPX(10));
    }
    else
    {
        w = lv_map(v, 0, 256, LV_DPX(60), max_w);
        lv_obj_set_width(obj, w);
        lv_obj_align(obj, LV_ALIGN_BOTTOM_RIGHT, -LV_DPX(10), -LV_DPX(10));
    }

    if (v > LV_OPA_COVER)
        v = LV_OPA_COVER;

    uint32_t i;
    for (i = 0; i < lv_obj_get_child_cnt(obj); i++)
    {
        lv_obj_set_style_opa(lv_obj_get_child(obj, i), v, 0);
    }
}

static void color_changer_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        lv_obj_t *color_cont = (lv_obj_t *)lv_event_get_user_data(e);
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, color_cont);
        lv_anim_set_exec_cb(&a, color_changer_anim_cb);
        if (lv_obj_get_width(color_cont) < LV_HOR_RES / 2)
        {
            lv_anim_set_values(&a, 0, 256);
        }
        else
        {
            lv_anim_set_values(&a, 256, 0);
        }
        lv_anim_set_time(&a, 200);
        lv_anim_start(&a);
    }
}

static void color_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_FOCUSED)
    {
        lv_obj_t *color_cont = lv_obj_get_parent(obj);
        if (lv_obj_get_width(color_cont) < LV_HOR_RES / 2)
        {
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, color_cont);
            lv_anim_set_exec_cb(&a, color_changer_anim_cb);
            lv_anim_set_values(&a, 0, 256);
            lv_anim_set_time(&a, 200);
            lv_anim_start(&a);
        }
    }
    else if (code == LV_EVENT_CLICKED)
    {
        config::palette = *((lv_palette_t *)lv_event_get_user_data(e));
        lv_palette_t palette_secondary = (lv_palette_t)(config::palette + 3); /*Use another palette as secondary*/
        if (palette_secondary >= _LV_PALETTE_LAST)
            palette_secondary = (lv_palette_t)0;
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
