#ifndef __ARG_VERSION__
#define __ARG_VERSION__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>


#define MAIN_VERSION    1
#define SECOND_VERSION  0
#define IS_RELEASE      0


/*
 * VERSION LOG 
 * 
 * V0.1 基本框架
 * V0.2 完成了注水逻辑 温控方法 还剩余与上位机交互，温控APP逻辑算法
 * V0.3 完成了power代码 完成了看门狗算法，进一步优化DEBUG功能
 * V0.4 升级了MOTOR包，BOX现在功能与DVD驱动器看齐，待微调测试
 * 
 * 
 * V 1.0版本
 * 
 * 修改了温控策略，大幅度修改了PID算法以适应新的温控策略
 * 
 * PID算法控制器修改如下
 * 定义了 集中控温模式 分散控温模式 并且提供了两个bool状态 concentrate_done 和 busy
 * 模糊了所有的PID API ROAD输入，输入的是0-4的
 * 所以例如想打开第0路控温，调用集中控温API，输入0
 * 此时状态机先集中控温第0路，集中控温结束concentrate_done置位，自动进入分散控温模式
 * 当分散控温模式一样结束，会复位busy位，通知上位机。
 * 
 * 所以，打开第0路的前提下，想打开第1路温控，必须在busy复位的条件下调用，否则集中温控函数API会返回false
 * 请求失败。
 * 
 * 水冷的算法如下
 * 
 * 调用者一次只能访问一路水冷泵，水冷在集中控温模式的时间域被访问，不允许被打断。
 * 集中模式结束，一定意味着水冷泵关闭。调用的时候，可以适当考虑水冷泵的关闭条件略
 * 高于target温度。
 * 
 * 增加了数组队列机制，添加了每一路的最高层五种状态定义，见APP.C/.H的修改
 * 为后期写最顶层代码做了铺垫。
 * 最顶层代码的编写，在于如下几点
 * 
 * 1 与TFT屏幕的交互，通知TFT每一路的控温状态
 * 2 控温时间的委托和处理
 * 3 综合测试
 * 见V1.1版本的后续更新
 */

void get_version_str(uint8_t * str,uint16_t len);

#endif
