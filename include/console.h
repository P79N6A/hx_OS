#ifndef INCLUDE_CONSOLE_H_
#define INCLUDE_CONSOLE_H_

#include "types.h"

typedef
enum real_color {
    rc_black = 0,
    rc_blue = 1,
    rc_green = 2,
    rc_cyan = 3,
    rc_red = 4,
    rc_magenta = 5,
    rc_brown = 6,
    rc_light_grey = 7,
    rc_dark_grey = 8,
    rc_light_blue = 9,
    rc_light_green = 10,
    rc_light_cyan = 11,
    rc_light_red = 12,
    rc_light_magenta = 13,
    rc_light_brown  = 14,   // yellow
    rc_white = 15
} real_color_t;

//清屏操作
void console_clear();

// 屏幕输出一个字符带颜色
void console_putc_color(char c,real_color_t back, real_color_t fore);

//屏幕打印一个以\0 结尾的字符串 默认黑底白字
void console_write(char *cstr);

//屏幕打印一个\0 结尾的字符串 带颜色

 void console_write_color(char *cstr, real_color_t back, real_color_t fore);

//屏幕输出一个16进制的整数型

void console_write_hex(uint32_t n, real_color_t back ,real_color_t fore);

//屏幕输出一个十进制的整数

void console_write_dec(uint32_t n, real_color_t back,real_color_t fore);

// 刷新屏幕显示到指定位置
void _flush_console(void);

// 刷新屏幕显示到当前输出位置
void _flush_console_current(void);

// 屏幕显示向上移动n行
void console_view_up(uint16_t offset);

// 屏幕显示向下移动n行
void console_view_down(uint16_t offset);

#endif
