#include "console.h"
#include "common.h"
#include "vmm.h"
// VGA 的显示缓冲的起点是 0xB8000 (+ PAGE_OFFSET)
static uint16_t *video_memory = (uint16_t *)(0xB8000 + PAGE_OFFSET);

//屏幕“光标” 的坐标

static uint8_t cursor_x =0;
static uint8_t cursor_y =0;

// 屏幕是 80 * 25
#define CON_WIDTH  80
#define CON_HIGH   25

// VGA内存输入缓冲区 80 * 128
#define BUFF_WIDTH  80
#define BUFF_HIGH   128

// VGA 输出缓冲区
static uint16_t video_buffer[BUFF_WIDTH * BUFF_HIGH];

// buffer 输出的坐标
static uint16_t buffer_x = 0;
static uint16_t buffer_y = 0;
#define VGA_IDX   0x3D4
#define VGA_SET   0x3D5

// 在这里用到的两个内部寄存器的编号为0xE与0xF，分别表示光标位置的高8位与低8位。
#define CUR_HIGH  0xE
#define CUR_LOW   0xF

// 当前屏幕显示在缓冲区的起始位置
static uint16_t current_line = 0;


static void move_cursor()
{
  // 屏幕是80字节宽度
  uint16_t cursorLocation =  cursor_y * 80 + cursor_x; 
  
  outb(0x3D4, 14);  //设置光标的高字节
  outb(0x3D5, cursorLocation >> 8);  //发送高8位数据
  outb(0x3D4, 15);        //设置光标的低字节
  outb(0x3D5, cursorLocation);   //发送低8位数据
}

void console_clear()
{
    uint8_t attribute_byte = (0 << 4) | (15 & 0x0F);
    uint16_t blank = 0x20 | (attribute_byte << 8);

    int i;
    for (i = 0; i < 80 * 25; i++) {
          video_memory[i] = blank;
    }

    cursor_x = 0;
    cursor_y = 0;

     move_cursor();

    // 初始化 buffer 数据
    for (uint32_t i = 0; i < BUFF_WIDTH * BUFF_HIGH; ++i) {
         video_buffer[i] = blank;
    }

    buffer_x = 0;
    buffer_y = 0;

    current_line = 0;
}

//屏幕滚动
static void scroll()
{
    // 先构造一个黑字白低的构造格式出来
    uint8_t arrtrbute_byte = (0<<4) | (15 &0x0F);
    uint16_t blank = 0x20 | (arrtrbute_byte << 8);  //0x20 是空格
    
    //cursor_y 大于 25 的时候就应该换行了
    if (cursor_y >= 25) {
        // 将所有行的显示数据复制到上一行，第一行永远消失了...
        int i;
        
        for (i = 0 * 80; i < 24 * 80; i++) {
              video_memory[i] = video_memory[i+80];
        }

        // 最后的一行数据现在填充空格，不显示任何字符
        for (i = 24 * 80; i < 25 * 80; i++) {
              video_memory[i] = blank;
        }
        
        // 向上移动了一行，所以 cursor_y 现在是 24
        cursor_y = 24;
    }

}

// 滚动缓冲区
static void scroll_buffer(void)
{
        // attribute_byte 被构造出一个黑底白字的描述格式
        uint8_t attribute_byte = (0 << 4) | (15 & 0x0F);
        uint16_t blank = 0x20 | (attribute_byte << 8);  // space 是 0x20

        // buffer_y 到 BUFF_HIGH - 1 的时候，就该换行了
        if (buffer_y == BUFF_HIGH - 1) {

                // 将所有行的显示数据复制到上一行，第一行永远消失了...
                for (uint32_t i = 0 * BUFF_WIDTH; i < (BUFF_HIGH-1) * BUFF_WIDTH; i++) {
                      video_buffer[i] = video_buffer[i+BUFF_WIDTH];
                }

                // 最后的一行数据现在填充空格，不显示任何字符
                for (uint32_t i = (BUFF_HIGH-1) * BUFF_WIDTH; i < BUFF_HIGH * BUFF_WIDTH; i++) {
                      video_buffer[i] = blank;
                }

                buffer_y--;
        }
}

//显示字符串

void console_putc_color(char c, real_color_t back, real_color_t fore)
{
    uint8_t back_color = (uint8_t)back;
           uint8_t fore_color = (uint8_t)fore;

           uint8_t attribute_byte = (back_color << 4) | (fore_color & 0x0F);
           uint16_t attribute = attribute_byte << 8;

           // 0x08 是 退格键 的 ASCII 码
           // 0x09 是 tab 键 的 ASCII 码
           if (c == 0x08 && buffer_x) {
                 buffer_x--;
           } else if (c == 0x09) {
                 buffer_x = (buffer_x+8) & ~(8-1);
           } else if (c == '\r') {
                 buffer_x = 0;
           } else if (c == '\n') {
                   buffer_x = 0;
                   buffer_y++;
           } else if (c >= ' ') {
                   video_buffer[buffer_y * BUFF_WIDTH + buffer_x] = c | attribute;
                   buffer_x++;
           }

           // 每 80 个字符一行，满80就必须换行了
           if (buffer_x == BUFF_WIDTH) {
                   buffer_x = 0;
                   buffer_y ++;
           }

           // 滚动缓冲区
           scroll_buffer();
}

void console_write(char *cstr)
{
    while (*cstr) {
            console_putc_color(*cstr++, rc_black, rc_white);
    }
    _flush_console_current();

}

void console_write_color(char *cstr, real_color_t back, real_color_t fore)
{
      while (*cstr) {
              console_putc_color(*cstr++, back, fore);
      }
      _flush_console_current();

}
// 刷新屏幕显示到指定位置
 void _flush_console(void)
{
        uint8_t attribute_byte = (0 << 4) | (15 & 0x0F);
        uint16_t blank = 0x20 | (attribute_byte << 8);
        uint16_t begin_line = 0, end_line = 0;

        begin_line = current_line;
        end_line = buffer_y + 1;

        uint32_t i = 0;
        for (uint32_t j = begin_line * CON_WIDTH; j < end_line * CON_WIDTH; ++j) {
                video_memory[i] = video_buffer[j];
                i++;
        }

        while (i < CON_WIDTH * CON_HIGH) {
                video_memory[i] = blank;
                i++;
        }

        cursor_x = buffer_x;
        cursor_y = end_line - begin_line - 1;

        move_cursor();
}

// 刷新屏幕显示到当前输出位置
void _flush_console_current(void)
{
        if (buffer_y >= CON_HIGH - 1) {
                current_line = buffer_y - CON_HIGH + 1;
        } else {
                current_line = 0;
        }
        _flush_console();
}
// 屏幕显示向上移动n行
void console_view_up(uint16_t offset)
{
        if (current_line >= offset) {
                current_line -= offset;
        } else {
                current_line = 0;
        }
        _flush_console();
}

// 屏幕显示向下移动n行
void console_view_down(uint16_t offset)
{
        if (current_line + offset < buffer_y) {
                current_line += offset;
        } else {
                current_line = buffer_y;
        }
        _flush_console();
}
