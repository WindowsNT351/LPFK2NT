### LPFK2NT
让你的IBM LPFK可编程键盘在现代电脑上使用<br />
4年前代码，勿喷<br />

## 📥如何使用
1) 本程序支持多个键盘（不过我没试过）<br />
2) 在Default中的config.ini中，修改键盘数Keyboard > Num,在对应键盘编号（从0开始）写上对应的串口号，配置好是否要调试窗口，在Software > WHide<br />
3) 进入到键盘对应文件夹,打开config.ini,修改Bps为9600，Par为1，KeyMaxNum为32，LightCtrl为148。<br />
4) 如果想要开机画面，打开LEDEdit，按下想要亮的位置，修改完本帧之后，键盘输入键盘序号，切换到下一帧，以此类推，弄好之后，把newVid下的所有文件复制到Default\0\修改StartVidio为1，修改SVMaxNum为总帧数，SVSleepTime为每帧间隔。<br />
5) 修改各个键值做出的反应，Lock为是否锁定，LightMode为灯的状态，0为禁用，1为锁定，2为闪烁，KeyNum和上面相同，1--5为LPFK按下后的反应，可以修改为键盘扫描码，256-258为鼠标左右中键。<br />
5) 打开COMDriver后台驻留，完成！<br />

## ℹ关于
4年前的老老老屑作，大概不会继续做支持！<br />

<br /><br /><br /><br />
Copyright 351Workshop 2022-2024
