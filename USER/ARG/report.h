#ifndef __REPORT__
#define __REPORT__

#include <stdbool.h>

/*
 * 本驱动旨在提供一个测试加热片同时升温性能的系列功能
 * 
 * 包括两个关键的参数
 * 
 * 1 升温速率差
 * 2 当前升温速率差对应的控温速度
 * 
 * 升温速率差的测试方法：
 * 同一层两片加热片在同一起始温度下，以全功率运行相同时间
 * 等待相同时间结束，记录当前的温差 除上时间，定义为升温速率差
 * 
 * 控温速度性能报告
 * 
 * 主要是测试高温控温速度，方法是开水冷降温，使得平均温度处于常温状态
 * 开始集中控温态，一旦集中控温态结束后，记录超调时间，越短越好。
 * 例如 给定到900摄氏度，控制超过900后开始计时一直记录到再一次小于900为止
 * 也就是记录超调时间。
 * 
 * 使用一个额外的init 和 handle为了避免与正常功能有任何的逻辑关联
 * 
 * 此功能仅仅出现在debug调试阶段，用来检验盒子的升温性能
 * 
 * 注意，打开report功能，将会破坏原生的逻辑，系统不考虑兼容原生系统逻辑的问题
 * 只是单纯的认为，在做这个性能报告检出任务的时候，不会有其他任务执行，也不会
 * 有开始控温这样的任务出现。
 * 
 */

void arg_report_init(void);
void arg_report_handle(void);

//TCP参数
#define TCP_CHECK_HIGH_TEMP 900

//HDR参数 全速升温时长度 单位是S
#define HDR_CHECK_TIM 20

//公共参数
#define NORMAL_TEMP  350
#define TEMP_ERROR_MAX 10


typedef enum {
    check_idle=0,
    prepare_stage,
    check_stage,
}check_status_t;

typedef enum{
    HRD_CLASS=0,
    TCP_CLASS,
}check_class_t;

/*
 * brief : 开始HRD的检测流程 HRD全称为Heating rate difference 升温速率差
 *          原理是先水冷到常温，等待一段时间保持温度，然后固定全速升温10S
 *          计算两片温差，出来一个，通过debug报文报一个结果。从0->4
 * pra @ is_all 决定是按顺序测试0->4所有的盒子还是仅仅只测试一个盒子0
 * 
 * return : 操作是否成功，失败的原因仅有状态机被重复打开
 * Note : 通过debug uart 实时输出结果
 */
bool start_HRD_check(bool is_all);

/*
 * brief : 开始TCP的检测流程 TCP全称为Temperature control performance 控温性能
 *          原理是先水冷到常温，等待一段时间保持温度，然后使用pid算法开始一次
 *          由集中模式开始的控温任务，控温的目标温度为TCP_CHECK_HIGH_TEMP
 * pra @ is_all 决定是按顺序测试0->4所有的盒子还是仅仅只测试一个盒子0
 * 
 * return : 操作是否成功，失败的原因仅有状态机被重复打开
 * Note : 通过debug uart 实时输出结果
 */

bool start_TCP_check(bool is_all);
#endif
