
import serial

import pyqtgraph as pg

import numpy as np

import array

import threading

from time import sleep

import re

 

app = pg.mkQApp()

win = pg.GraphicsWindow()

win.setWindowTitle(u'波形显示')

p = win.addPlot()#win.addPlot()添加一个波形窗口，多次调用会将窗口分成多个界面

 

data_dict = {}#存放所有收到的数据

p.addLegend() #不添加就显示不了图例 ，一定要放在plot前调用

 

Max_count = 500 #页面最多显示的数据个数

 

#配置波形显示信息

def DisplayConfig():   

    p.showGrid(x=True,y=True, alpha=0.5)

    p.setLabels(left='y/V',bottom='x/point',title='imu')#left纵坐标名 bottom横坐标名

    label = pg.TextItem()

    p.addItem(label)

    

 

#将串口收到的数据添加到字典

#数据格式 “name1,float;name2,flaot\n”  

def AddDataToDict(line):

    line = line.split("\\n") #目的是去除最后的\n换行，别的方式还没用明白

    str_arr = line[0].split(';')#因为上边分割了一下，所以是数组

    color = ['b','g','r', 'c','m','y', 'k','w']#颜色表，这些应该够了，最多8条线，在添加颜色可以用(r,g,b)表示

    for a in str_arr: #遍历获取单个变量 如“a,1;b,2;c,3”中的"a,1"

        s = a.split(',')#提取名称和数据部分

        if(len(s) != 2):#不等于2字符串可能错了，正确的只有名称和数据两个字符串

            return

        name = s[0]

        val_str = re.findall(r"^[-+]?([0-9]+(\.[0-9]*)?|\.[0-9]+)([eE][-+]?[0-9]+)?$",s[1])[0]#用正则表达式提取数字部分

        if(len(val_str)>0):#再判断下是否匹配到了数字

            val = float(val_str[0])#转成浮点型数字

            if(data_dict.get(name) == None): #判断是否存在添加当前键值，None则需要添加键值                  

                #curve = p.plot(pen = color[len(data_dict)],name=name,symbolBrush=color[len(data_dict)])#为新的变量添加新的曲线,显示数据点

                curve = p.plot(pen = color[len(data_dict)],name=name)#为新的变量添加新的曲线

                data_dict[name] = [curve]  #在字典中添加当前键值，并赋值曲线，字典数据格式{key:[curve,[dadta1,data2,...]]}

                data_dict[name].append([val])#将当前数据已列表的形式添加到字典对象中

            else:#键值存在直接添加到对应的数据部分

                if(len(data_dict[s[0]][1]) == Max_count):#限制一下页面数据个数

                    data_dict[s[0]][1]=data_dict[s[0]][1][1:-1]

                    data_dict[s[0]][1][-1] = float(s[1])

                else:

                    data_dict[s[0]][1].append(val)

        else:#接收错误

            print("error:" + a)

            return 

        

 

def addToDisplay():

    for i in data_dict.items():

        data = i[1][1] #数据部分

        curve = i[1][0] #当前的线

        # if(len(data) > 1000):#一个界面都数据控制在1000个

        #     data=data[-1000:]

        # else:

        #     data = data[:]

        curve.setData(data)#添加数据显示

 

 

def ComRecvDeal():

    t = InitCom('com14',115200) #串口号和波特率自行设置

    if(t.isOpen() == True):      

        t.flushInput() #先清空一下缓冲区

        while(True):

            line = t.readline().decode() #line是bytes格式，使用decode()转成字符串

            AddDataToDict(line)#把收到的数据添加到字典中

    else:

        print("串口打开失败")

    

 

def InitCom(port,b):

    t = serial.Serial(port,b)

    return t

 

if __name__ == "__main__":

    th= threading.Thread(target=ComRecvDeal)#创建串口接收线程

    th.start()

    timer = pg.QtCore.QTimer()

    timer.timeout.connect(addToDisplay) #定时刷新数据显示

    timer.start(10)

    app.exec_()

    
