import matplotlib.pyplot as plt
from pylab import *

def square_output(dat):
    for i in range(0,len(dat)):

        if(i % 10 == 0):
            print("\r\n",end='')

        if(int(dat[i]) < 10):
            print("  ",end='')  
        elif(int(dat[i] <99)):
            print(" ",end='')
        else:
            pass   

        print(int(dat[i]),end='')
        print(",",end='')


#make trapezoid graph
#这是梯形曲线的生成代码，输入几个关键参数之后，代码会将它画出来
def trapezoid_graph_maker(min_speed,max_speed,lasting_tim,tim_total,needshow=False):
    """min_speed最小速度,max_speed最大速度,lasting_tim最大速度持续时间，tim_total总计持续多长时间"""
    print("min speed : %d max speed : %d lasting_tim : %d tim_total : %d" % (min_speed,max_speed,lasting_tim,tim_total))
    rlt_graph = []
    #画梯形，从最小值折线min_speed 运动到最大值 max_speed ,然后持续lasting_tim时间
    #注意 lasting_tim 必须小于 tim_total

    if(lasting_tim > tim_total):
        print("parameter error lasting time too big\r\n")
    
    start_lasting_tim = 0
    now_speed =0

    start_lasting_tim = ( tim_total - lasting_tim ) // 2
    #10 400-800
    k = ( max_speed - min_speed ) / start_lasting_tim

    print("k = %f" % k)

    for i in range(0,tim_total):
        if(i >= start_lasting_tim and i<= (start_lasting_tim + lasting_tim) ):
            rlt_graph.append (max_speed)
        elif(i < start_lasting_tim):
            rlt_graph.append ( min_speed + k * i)
            now_speed = rlt_graph[i]
        else:
            rlt_graph.append ( max_speed -  k * (i - (start_lasting_tim + lasting_tim)))
            now_speed = rlt_graph[i]            

        #print("%d" % rlt_graph[i])
    
    #格式化输出适合C代码的输出
    square_output(rlt_graph)


    #画图代码
    if(needshow):
        mpl.rcParams['font.sans-serif'] = ['SimHei']
        x = range(0,tim_total)
        y = rlt_graph
        plt.plot(x, y, marker='o', mec='r', mfc='w',label=u'speed_tim')
        plt.legend()

        name = [str(i) for i in range(0,tim_total)]
        #plt.xticks(x, name, rotation=45)
        plt.margins(0)
        plt.subplots_adjust(bottom=0.15)
        plt.xlabel(u"time(s)") #X轴标签
        plt.ylabel("Speed") #Y轴标签
        plt.title("Speed - tim graph") #标题
        plt.show()



if __name__ == "__main__":
    trapezoid_graph_maker(0,100,50,100,False)