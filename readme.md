[TOC]

## 介绍

QuickButton，支持单击、双击、长按功能；支持动态的改变长按的响应时间；



## 接口一览

```
int8_t button_create(button_obj_t *obj, uint8_t (*read_pin)(void), void (*cb)(uint8_t event, void *arg), void *arg);
int8_t button_set_longpress_time(button_obj_t *obj, uint16_t time);
void button_tick(uint16_t period_ms);
```



## 如何使用

1. button_create创建一个按键对象，并把读取按键引脚函数的接口作为参数，注册到按键对象里边；读取到按键触发的电平时，返回1，否则返回0。
2. 周期性的调用button_tick。
3. 如果需要改变长按的有效时间，则使用button_set_longpress_time。



## 例子

参考 ./demo













