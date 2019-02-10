/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * LCD Python module.
 *
 */
#include <mp.h>
#include <objstr.h>
#include <spi.h>
#include "imlib.h"
#include "fb_alloc.h"
#include "vfs_wrapper.h"
#include "py_assert.h"
#include "py_helper.h"
#include "py_image.h"
#include "st7789.h"
#include "sleep.h"
/*****freeRTOS****/
#include "FreeRTOS.h"
#include "task.h"

#include "sysctl.h"


#define printf(...)

// #define RST_PORT            GPIOD
// #define RST_PIN             GPIO_PIN_12
// #define RST_PIN_WRITE(bit)  HAL_GPIO_WritePin(RST_PORT, RST_PIN, bit);

// #define RS_PORT             GPIOD
// #define RS_PIN              GPIO_PIN_13
// #define RS_PIN_WRITE(bit)   HAL_GPIO_WritePin(RS_PORT, RS_PIN, bit);

// #define CS_PORT             GPIOB
// #define CS_PIN              GPIO_PIN_12
// #define CS_PIN_WRITE(bit)   HAL_GPIO_WritePin(CS_PORT, CS_PIN, bit);

// #define LED_PORT            GPIOA
// #define LED_PIN             GPIO_PIN_5
// #define LED_PIN_WRITE(bit)  HAL_GPIO_WritePin(LED_PORT, LED_PIN, bit);

// extern mp_obj_t pyb_spi_send(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args);
// extern mp_obj_t pyb_spi_make_new(mp_obj_t type_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args);
// extern mp_obj_t pyb_spi_deinit(mp_obj_t self_in);

// static mp_obj_t spi_port = NULL;
static int width = 0;
static int height = 0;
static enum { LCD_NONE, LCD_SHIELD } type = LCD_NONE;
// static bool backlight_init = false;

// Send out 8-bit data using the SPI object.
static void lcd_write_command_byte(uint8_t data_byte)
{
    // mp_map_t arg_map;
    // arg_map.all_keys_are_qstrs = true;
    // arg_map.is_fixed = true;
    // arg_map.is_ordered = true;
    // arg_map.used = 0;
    // arg_map.alloc = 0;
    // arg_map.table = NULL;

    tft_set_datawidth(8);
	tft_write_command(data_byte);

    // CS_PIN_WRITE(false);
    // RS_PIN_WRITE(false); // command
    // pyb_spi_send(
    //     2, (mp_obj_t []) {
    //         spi_port,
    //         mp_obj_new_int(data_byte)
    //     },
    //     &arg_map
    // );
    // CS_PIN_WRITE(true);
}

// Send out 8-bit data using the SPI object.
static void lcd_write_data_byte(uint8_t data_byte)
{
    // mp_map_t arg_map;
    // arg_map.all_keys_are_qstrs = true;
    // arg_map.is_fixed = true;
    // arg_map.is_ordered = true;
    // arg_map.used = 0;
    // arg_map.alloc = 0;
    // arg_map.table = NULL;

    uint8_t temp_type;
    temp_type = data_byte;
    tft_set_datawidth(8);
    tft_write_byte(&temp_type, 1);

    // CS_PIN_WRITE(false);
    // RS_PIN_WRITE(true); // data
    // pyb_spi_send(
    //     2, (mp_obj_t []) {
    //         spi_port,
    //         mp_obj_new_int(data_byte)
    //     },
    //     &arg_map
    // );
    // CS_PIN_WRITE(true);
}

// Send out 8-bit data using the SPI object.
static void lcd_write_command(uint8_t data_byte, uint32_t len, uint8_t *dat)
{
    // lcd_write_command_byte(data_byte);
    // for (uint32_t i=0; i<len; i++) lcd_write_data_byte(dat[i]);

    tft_set_datawidth(8);
	tft_write_command(data_byte);
    tft_write_byte(dat, len);

}

// Send out 8-bit data using the SPI object.
static void lcd_write_data(uint32_t len, uint8_t *dat)
{
    // mp_obj_str_t arg_str;
    // arg_str.base.type = &mp_type_bytes;
    // arg_str.hash = 0;
    // arg_str.len = len;
    // arg_str.data = dat;

    // mp_map_t arg_map;
    // arg_map.all_keys_are_qstrs = true;
    // arg_map.is_fixed = true;
    // arg_map.is_ordered = true;
    // arg_map.used = 0;
    // arg_map.alloc = 0;
    // arg_map.table = NULL;

    // CS_PIN_WRITE(false);
    // RS_PIN_WRITE(true); // data
    // pyb_spi_send(
    //     2, (mp_obj_t []) {
    //         spi_port,
    //         &arg_str
    //     },
    //     &arg_map
    // );
    // CS_PIN_WRITE(true);
    //tft_set_datawidth(8);
    tft_write_byte(dat, len);
}

static mp_obj_t py_lcd_deinit()
{
    switch (type) {
        case LCD_NONE:
            return mp_const_none;
        case LCD_SHIELD:
            // HAL_GPIO_DeInit(RST_PORT, RST_PIN);
            // HAL_GPIO_DeInit(RS_PORT, RS_PIN);
            // HAL_GPIO_DeInit(CS_PORT, CS_PIN);
            // pyb_spi_deinit(spi_port);
            // spi_port = NULL;
            width = 0;
            height = 0;
            type = LCD_NONE;
            // if (backlight_init) {
            //     HAL_GPIO_DeInit(LED_PORT, LED_PIN);
            //     backlight_init = false;
            // }
            return mp_const_none;
    }
    return mp_const_none;
}

static mp_obj_t py_lcd_init(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    py_lcd_deinit();
    switch (py_helper_keyword_int(n_args, args, 0, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_type), LCD_SHIELD)) {
        case LCD_NONE:
            return mp_const_none;
        case LCD_SHIELD:
        {
            // GPIO_InitTypeDef GPIO_InitStructure;
            // GPIO_InitStructure.Pull  = GPIO_NOPULL;
            // GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
            // GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_OD;

            // GPIO_InitStructure.Pin = CS_PIN;
            // CS_PIN_WRITE(true); // Set first to prevent glitches.
            // HAL_GPIO_Init(CS_PORT, &GPIO_InitStructure);

            // GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;

            // GPIO_InitStructure.Pin = RST_PIN;
            // RST_PIN_WRITE(true); // Set first to prevent glitches.
            // HAL_GPIO_Init(RST_PORT, &GPIO_InitStructure);

            // GPIO_InitStructure.Pin = RS_PIN;
            // RS_PIN_WRITE(true); // Set first to prevent glitches.
            // HAL_GPIO_Init(RS_PORT, &GPIO_InitStructure);

            // spi_port = pyb_spi_make_new(NULL,
            //     2, // n_args
            //     3, // n_kw
            //     (mp_obj_t []) {
            //         MP_OBJ_NEW_SMALL_INT(2), // SPI Port
            //         MP_OBJ_NEW_SMALL_INT(SPI_MODE_MASTER),
            //         MP_OBJ_NEW_QSTR(MP_QSTR_baudrate),
            //         MP_OBJ_NEW_SMALL_INT(1000000000/66), // 66 ns clk period
            //         MP_OBJ_NEW_QSTR(MP_QSTR_polarity),
            //         MP_OBJ_NEW_SMALL_INT(0),
            //         MP_OBJ_NEW_QSTR(MP_QSTR_phase),
            //         MP_OBJ_NEW_SMALL_INT(0)
            //     }
            // );
            width = 320;
            height = 240;
            type = LCD_SHIELD;
            // backlight_init = false;


            sysctl->power_sel.power_mode_sel6 = 1;
            sysctl->power_sel.power_mode_sel7 = 1;

            fpioa_set_function(37, 25);
            fpioa_set_function(38, 26);
            fpioa_set_function(36, 15);
            fpioa_set_function(39, 17);
            
            lcd_init();
            lcd_clear(WHITE);
            vTaskDelay(120/portTICK_PERIOD_MS);

            return mp_const_none;
        }
    }
    return mp_const_none;
}

static mp_obj_t py_lcd_width()
{
    if (type == LCD_NONE) return mp_const_none;
    return mp_obj_new_int(width);
}

static mp_obj_t py_lcd_height()
{
    if (type == LCD_NONE) return mp_const_none;
    return mp_obj_new_int(height);
}

static mp_obj_t py_lcd_type()
{
    if (type == LCD_NONE) return mp_const_none;
    return mp_obj_new_int(type);
}

static mp_obj_t py_lcd_set_backlight(mp_obj_t state_obj)
{
    // switch (type) {
    //     case LCD_NONE:
    //         return mp_const_none;
    //     case LCD_SHIELD:
    //     {
    //         // bool bit = !!mp_obj_get_int(state_obj);
    //         // if (!backlight_init) {
    //         //     GPIO_InitTypeDef GPIO_InitStructure;
    //         //     GPIO_InitStructure.Pull  = GPIO_NOPULL;
    //         //     GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    //         //     GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_OD;
    //         //     GPIO_InitStructure.Pin = LED_PIN;
    //         //     LED_PIN_WRITE(bit); // Set first to prevent glitches.
    //         //     HAL_GPIO_Init(LED_PORT, &GPIO_InitStructure);
    //         //     backlight_init = true;
    //         // }
    //         // LED_PIN_WRITE(bit);
    //         // return mp_const_none;
    //     }
    // }
    return mp_const_none;
}

static mp_obj_t py_lcd_get_backlight()
{
    // switch (type) {
    //     case LCD_NONE:
    //         return mp_const_none;
    //     case LCD_SHIELD:
    //         // if (!backlight_init) {
    //         //     return mp_const_none;
    //         // }
    //         // return mp_obj_new_int(HAL_GPIO_ReadPin(LED_PORT, LED_PIN));
    // }
    return mp_const_none;
}

static mp_obj_t py_lcd_display(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    rectangle_t rect;
    uint16_t line_n;
    uint16_t x,y;
    uint16_t y_offset;
    py_helper_keyword_rectangle_roi(arg_img, n_args, args, 1, kw_args, &rect);
    
    printf("w %d h %d\n",rect.w,rect.h);
    // Fit X.
    int l_pad = 0, r_pad = 0;
    if (rect.w > width) {
        int adjust = rect.w - width;
        rect.w -= adjust;
        rect.x += adjust / 2;
    } else if (rect.w < width) {
        int adjust = width - rect.w;
        l_pad = adjust / 2;
        r_pad = (adjust + 1) / 2;
    }
    printf("l_pad %d r_pad %d\n",l_pad,r_pad);
    // Fit Y.
    int t_pad = 0, b_pad = 0;
    if (rect.h > height) {
        int adjust = rect.h - height;
        rect.h -= adjust;
        rect.y += adjust / 2;
    } else if (rect.h < height) {
        int adjust = height - rect.h;
        t_pad = adjust / 2;
        b_pad = (adjust + 1) / 2;
    }
    printf("t_pad %d b_pad %d\n",t_pad,b_pad);
    //lcd_set_area(0, 0, 480, 320);
    //lcd_set_area(0, 0, rect.w, rect.h);
    // tft_set_datawidth(8);
    y_offset = 0;
    switch (type) {
        case LCD_NONE:
            return mp_const_none;
        case LCD_SHIELD:
            //lcd_write_command_byte(0x2C);
            printf("lcd_write_command_byte\n");
            //uint8_t *zero = (uint8_t *)(malloc(width*2*sizeof(uint8_t)));
            uint16_t temp_data = BLUE;
            // uint16_t *line = (uint16_t *)(malloc(width*sizeof(uint16_t)));
            uint16_t line;
            // memset(zero,BLACK,width*2*sizeof(uint8_t));
            lcd_set_area(0, 0, width, height);
            tft_set_datawidth(16);
            for (int i=0; i<t_pad; i++) {
                //printf("t_pad %d x %d i %d\n",t_pad,x,i);
                //lcd_write_data(width*2, zero);
				temp_data = RED;
                for(x=0; x<width;x++)
                    //t_set_datawidth(16);
                    tft_write_half(&temp_data, 1);
            }
            y_offset +=t_pad;
            //printf("for (int i=0; i<t_pad; i++) \n");
            for (int i=0; i<rect.h; i++) {
                if (l_pad) {
                    //lcd_write_data(l_pad*2, zero); // l_pad < width
                    temp_data = BLUE;
                    for(x=0; x<l_pad;x++)
                        //t_set_datawidth(16);
                        tft_write_half(&temp_data, 1);
                }
                //intf("i %d\n",i);
                
                if (IM_IS_GS(arg_img)) {
                    for (int j=0; j<rect.w; j++) {
                        uint8_t pixel = IM_GET_GS_PIXEL(arg_img, (rect.x + j), (rect.y + i));
                        line = IM_RGB565(IM_R825(pixel),IM_G826(pixel),IM_B825(pixel));
                        //t_set_datawidth(16);
                        tft_write_half(&line, 1);
                        //printf("j %d,line %d\n",j,line);
                    }
                    //printf("lcd_write_data line %d\n",line);
                    // for(x=0; x<rect.w;x++)
                    // {
                    //     tft_set_datawidth(16);
                    //     tft_write_half(&line, 1);
                    // }

                        // lcd_draw_point(x+l_pad, y_offset, line[x+l_pad]);
                    //lcd_write_data(rect.w*2, (uint8_t *) line);
                    
                    // for(line_n=0;line_n<rect.w*2;line_n++)
                    // {
                    //     printf("line_n %d line %d\n",line_n,*(uint8_t *)(line));
                    // }
                    
                } else {
                    // for(x=0; x<rect.w;x++)
                    // {
                    //     // tft_set_datawidth(16);
                    //     // tft_write_half(
                    //     // (((uint16_t *) arg_img->pixels) +
                    //     // ((rect.y + i) * arg_img->w) + rect.x+l_pad), rect.w);
                    //     tft_set_datawidth(16);
                    //     tft_write_half((uint16_t *)
                    //     (((uint16_t *) arg_img->pixels) +
                    //     ((rect.y + i) * arg_img->w) + rect.x+x+l_pad), 1);
                    //     // lcd_draw_point(x+l_pad, y_offset, *(uint16_t *)
                    //     // (((uint16_t *) arg_img->pixels) +
                    //     // ((rect.y + i) * arg_img->w) + rect.x+x+l_pad));

                    // }
                    tft_write_half((uint16_t *)
                        (((uint16_t *) arg_img->pixels) +
                        ((rect.y + i) * arg_img->w) + rect.x+l_pad), rect.w);

                        // lcd_draw_point(x+l_pad, y_offset, *(uint16_t *)
                        // (((uint16_t *) arg_img->pixels) +
                        // ((rect.y + i) * arg_img->w) + rect.x+x+l_pad));
                    // lcd_write_data(rect.w*2, (uint8_t *)
                    //     (((uint16_t *) arg_img->pixels) +
                    //     ((rect.y + i) * arg_img->w) + rect.x));
                    // for(line_n=0;line_n<rect.w*2;line_n++)
                    // {
                    //     printf("------------line_n %d line %d\n",line_n,*(uint8_t *)
                    //     (((uint16_t *) arg_img->pixels) +
                    //     ((rect.y + i) * arg_img->w) + rect.x+line_n));
                    // }
                }
                if (r_pad) {
                    //printf("r_pad %d x %d i %d\n",r_pad,x+l_pad+arg_img->w,i);
                    temp_data = GREEN;
                    for(x=0; x<r_pad;x++)
                        //t_set_datawidth(16);
                        tft_write_half(&temp_data, 1);
                    //lcd_write_data(r_pad*2, zero); // r_pad < width
                }
                y_offset ++;
            }
            for (int i=0; i<b_pad; i++) {
                // lcd_write_data(width*2, zero);
                //printf("b_pad %d x %d i %d\n",b_pad,x,y_offset+1);
                temp_data = YELLOW;
                for(x=0; x<width;x++)
                    //t_set_datawidth(16);
                    tft_write_half(&temp_data, 1);
            }
            // free(zero);
            // free(line);
            return mp_const_none;
    }
    return mp_const_none;
}

static mp_obj_t py_lcd_clear()
{
    switch (type) {
        case LCD_NONE:
            return mp_const_none;
        case LCD_SHIELD:
            // lcd_write_command_byte(0x2C);
            // uint8_t *zero = fb_alloc0(width*2);
            // for (int i=0; i<height; i++) {
            //     lcd_write_data(width*2, zero);
            // }
            // fb_free();
            lcd_clear(BLACK);
            return mp_const_none;
    }
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_lcd_init_obj, 0, py_lcd_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_deinit_obj, py_lcd_deinit);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_width_obj, py_lcd_width);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_height_obj, py_lcd_height);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_type_obj, py_lcd_type);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_lcd_set_backlight_obj, py_lcd_set_backlight);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_get_backlight_obj, py_lcd_get_backlight);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_lcd_display_obj, 1, py_lcd_display);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_clear_obj, py_lcd_clear);
static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),        MP_OBJ_NEW_QSTR(MP_QSTR_lcd) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_init),            (mp_obj_t)&py_lcd_init_obj          },
    { MP_OBJ_NEW_QSTR(MP_QSTR_deinit),          (mp_obj_t)&py_lcd_deinit_obj        },
    { MP_OBJ_NEW_QSTR(MP_QSTR_width),           (mp_obj_t)&py_lcd_width_obj         },
    { MP_OBJ_NEW_QSTR(MP_QSTR_height),          (mp_obj_t)&py_lcd_height_obj        },
    { MP_OBJ_NEW_QSTR(MP_QSTR_type),            (mp_obj_t)&py_lcd_type_obj          },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_backlight),   (mp_obj_t)&py_lcd_set_backlight_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_backlight),   (mp_obj_t)&py_lcd_get_backlight_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_display),         (mp_obj_t)&py_lcd_display_obj       },
    { MP_OBJ_NEW_QSTR(MP_QSTR_clear),           (mp_obj_t)&py_lcd_clear_obj         },
    { NULL, NULL },
};
STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t lcd_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t)&globals_dict,
};

void py_lcd_init0()
{
    py_lcd_deinit();
}
