#ifndef INCLUDE_CHAR_DEV_H_
#define INCLUDE_CHAR_DEV_H_

#include <types.h>

// 字符设备接口

typedef
struct char_dev {
    const char *name;
    bool is_readable;
    bool is_writeable;
    struct char_ops{
        int (*init)(void);
        bool (*device_valid)(void);
        const char *(*get_desc)(void);
        int (*read)(void *, uint32_t);
        int (*write)(const void *, uint32_t);
        int (*ioctl)(int,int);
    }ops;
    struct char_dev *next;                          // 字符设备链
}char_dev_t;


// 全局字符设备链表
extern char_dev_t *char_devs;

// 字符设备初始化
void char_dev_init(void);

// 内核注册字符设备
int add_char_dev(char_dev_t *cdev);

// Keyboard 设备结构
extern char_dev_t kboard_dev;

#endif  // INCLUDE_CHAR_DEV_H_
