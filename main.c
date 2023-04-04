#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <linux/input.h>  

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>


#include <stdio.h>//printf、scanf
#include <termios.h>
#include <sys/types.h>//open
#include <sys/stat.h>//open
#include <fcntl.h>//open
#include <string.h>//bzero
#include "uart.h"


#if 1
	#define DEBUG printf("file is: %s, function is: %s, line is: %d\n\n", __FILE__, __FUNCTION__, __LINE__);
#else
	#define DEBUG
#endif




//定义相关变量
int touch_lcdfd = -1, x = 0, y = 0;
//定义一个结构体用以存放输入事件数据
struct input_event buf;

//函数声明
void touch_lcd(void);


//定义全局变量
int lcd_fd, bmp_fd, ret;
int *lcd_fp = NULL;


//定义变量和数组
int keynum1 = 0;						//输入密码个数
int key1[6] = {0};                      //输入密码的储存
int truekey[6] = {1, 4, 7, 1, 5, 9};    //正确密码
int panduan = 0;                       //判断  
int eng=0;								//密码循环次数
int jiesuo=0;							//解锁方式
//编译命令：arm-linux-gcc    mmap_show_bmp.c
//执行命令：./a.out  123.bmp

//处理屏幕设备

int lcd_init(void)
{	
	//打开屏幕
	lcd_fd = open("/dev/fb0", O_RDWR);
	if(lcd_fd == -1)
	{
		perror("open LCD failed");
		return -1;
	}
	
	//建立内存映射
	lcd_fp = mmap(NULL, 800*480*4, PROT_WRITE, MAP_SHARED, lcd_fd, 0);
	if(lcd_fp == NULL)
	{
		perror("mmap failed\n");
		return -1;
	}
}

//打开图片文件
//show_bmp("./456.bmp");
int show_bmp(char *pathname)
{	
	//打开图片，利用open函数打开指定路径的图片文件
	bmp_fd = open( pathname, O_RDWR);
	if(bmp_fd == -1)
	{
		perror("open bmp failed");
		return -1;
	}
	
	//跳过前54个字节BMP格式头
	lseek(bmp_fd, 54, SEEK_SET);
	
	//定义数组存放图片的真实数据（BGR为3个字节）
	char bmp_buf[800*480*3]={0};
	
	//读取真实颜色数据，由于BMP图片真实的格式为BGR，即一个像素点为3个字节，所以一张800*480大小的图片的总字节数为800*480*3
	ret = read(bmp_fd, bmp_buf, sizeof(bmp_buf));	//read函数是每次读取一个字节，读取指定的总字节数
	if(ret == -1)
	{
		perror("read bmp failed");
		return -1;
	}
	
	int i, j;
	int bmp_cmp_buf[800*480] = {0};
	//作24位图的BGR 转换成 屏幕的ARGB
	for(i=0,j=0; i<800*480; i++, j+=3)
	{
						//bmp_buf[0] = 一个字节：B， bmp_buf[012] = 一个字节：BGR,  一行像素点：BGR BGR BGR BGR BGR BGR
		bmp_cmp_buf[i] = 0x00<<24 | (bmp_buf[j+2] << 16) | (bmp_buf[j+1] << 8) | (bmp_buf[j+0] << 0);
	
	}
	
	int x, y;
	
	int bmp_up_buf[800*480] = {0};
	//作图片的上下颠倒处理
	for(y=0; y<480; y++)
	{
		for(x=0; x<800; x++)
		{
			bmp_up_buf[800*y+x] = bmp_cmp_buf[800*(479-y)+x];
		}	
	}
	
	//把数据写入映射内存（屏幕）
	for(y=0; y<480; y++)
	{
		for(x=0; x<800; x++)
		{
			lcd_fp[800*y+x] = bmp_up_buf[800*y+x];
		}
	}
	
	//关闭图片文件
	close(bmp_fd);
}
//图片切换
int keyshow() 
{
     switch (keynum1)
       {
        case 0:
            show_bmp("./2002.bmp");
            break;
        case 1:
            show_bmp("./2003.bmp");
            break;
        case 2:
            show_bmp("./2004.bmp");
            break;
        case 3:
            show_bmp("./2005.bmp");
            break;
        case 4:
            show_bmp("./2006.bmp");
            break;
        case 5:
            show_bmp("./2007.bmp");
            break;
        
        default:
            break;
  		}
		
}	

//密码解锁
void password_fun(void)
{
	printf("请输入正确密码\n");
	while(eng !=3 )
	{
		//读触摸屏的触摸数值,拿取x\y轴坐标值数据
		touch_lcd();	//调用一次函数，获取一次触摸数据
		printf("单次点击坐标点为：(%d , %d)\n", x, y);

		if (x > 322 && x < 478 && y > 404 && y < 480) //按钮0
        {
            printf("0\n");
            keyshow();
            key1[keynum1++] = 0;
        }

        if (x > 165 && x < 322 && y > 178 && y < 253) //按钮1
        {
            printf("1\n");
            keyshow();
            key1[keynum1++] = 1;
        }

        if (x > 322 && x < 478 && y > 178 && y < 253) //按钮2
        {
            printf("2\n");
            keyshow();
            key1[keynum1++] = 2;
        }

        if (x > 478 && x < 636 && y > 178 && y < 253) //按钮3
        {
            printf("3\n");
            keyshow();
            key1[keynum1++] = 3;
        }

        if (x > 165 && x < 322 && y > 253 && y < 328) //按钮4
        {
            printf("4\n");
            keyshow();
            key1[keynum1++] = 4;
        }

        if (x > 266 && x < 535 && y > 253 && y < 328) //按钮5
        {
            printf("5\n");
            keyshow();
            key1[keynum1++] = 5;
        }

        if (x > 536 && x < 800 && y > 253 && y < 328) //按6
        {
            printf("6\n");
            keyshow();
            key1[keynum1++] = 6;
        }

        if (x > 165 && x < 322 && y > 328 && y < 404) //按钮7
        {
            printf("7\n");
            keyshow();
            key1[keynum1++] = 7;
        }

        if (x > 266 && x < 535 && y > 328 && y < 404) //按钮8
        {
            printf("8\n");
            keyshow();
            key1[keynum1++] = 8;
        }

        if (x > 478 && x < 636 && y > 328 && y < 404) //按钮9
        {
            printf("9\n");
            keyshow();
            key1[keynum1++] = 9;
        }

        if (x > 165 && x < 322 && y > 404 && y < 480) //按钮0
        {
            printf("退格\n");
			if (keynum1>0)
			{
				keynum1--;
				keyshow();
			}
			else
			 {show_bmp("./2001.bmp");}
        }

        if (x > 478 && x < 636 && y > 404 && y < 480) //按钮0
        {
            printf("确定\n");
			keynum1=6;
            
        }


        if (keynum1 == 6) //退出条件
        {
            printf("密码达到6位\n");
            for ( panduan=0;panduan < 6; panduan++)//判断密码正确性
    		{
       		 if (key1[panduan] != truekey[panduan])
       		   {
				  break;
       		   }
   	    	}
			
			if (panduan == 6)//锁的的判断
   			 {
        		eng=3;
   			 }

			else
       		 {
				show_bmp("./2008.bmp");
				sleep(2);
				show_bmp("./2001.bmp");
       		 	keynum1=0;
				eng++;
			 }
		}
		
		

	 }
}

void touch_lcd(void)
{
	while(1)
	{
		//读取触摸屏接口，拿触摸屏数据，读取出来的数据是一个结构体，放在buf里。
		read(touch_lcdfd, &buf, sizeof(buf));
		
		//如果是绝对值坐标，且是x轴的绝对坐标
		if( (buf.type == EV_ABS) && (buf.code == ABS_X) )
		{
			//如果你的开发板的屏幕的边是黑色的就用以下获取坐标的代码：
			x = buf.value*800/1024;
			
			
			//如果你的开发板的屏幕的边是蓝色
			//x = buf.value;
		}
		
		//如果是绝对值坐标，且是y轴的绝对坐标
		if( (buf.type == EV_ABS) && (buf.code == ABS_Y) )
		{
			//如果你的开发板的屏幕的边是黑色的就用以下获取坐标的代码：
			y = buf.value*480/600;
			
			//如果你的开发板的屏幕的边是蓝色
			//y = buf.value;
		}
		
		//如果是触摸屏触摸事件，且为松开状态
		if((buf.code==BTN_TOUCH) && (buf.value==0))
		{
			//退出循环
			break;	
		}
	}
}


int pic_up(char *pathname, int* pic_up_fd)
{	
	int ret=0;
	int line=0;
	int block=0;
	int i=0, j=0, k=0;
	int bmp_fd=0;
	
	char bmp_buf[480*800*3];
	int mi_buf[480*800];	
	int temp_buf[480*800];	
	bzero(mi_buf,sizeof(mi_buf));
	bzero(bmp_buf,sizeof(bmp_buf));
	bzero(temp_buf,sizeof(temp_buf));

	bmp_fd = open(pathname , O_RDONLY);//1、打开BMP格式图片
	if(bmp_fd == -1)
	{
		printf("pic_up(), open bmp failed\n");
		return -1;
	}

	ret = lseek(bmp_fd, 54 , SEEK_SET);//2、跳过bmp图片的前54个位置
	if(ret == -1)
	{
		perror("pic_up(), lseek bmp failed\n");		
		return -1;
	}

	ret = read(bmp_fd , bmp_buf , sizeof(bmp_buf)); //4、取读图片像素
	if(ret == -1)
	{
		printf("pic_up(), read bmp failed\n");	
		return -1;
	}
	
	close(bmp_fd);//5、关闭图片

	for(i=0, j=0 ; i<800*480 ; i++, j+=3)//6、24bits 转 32bits控制变量
	{
		temp_buf[i] = bmp_buf[j+2]<<16 | bmp_buf[j+1]<<8 | bmp_buf[j] ;
	}

	for(line=0 ; line <=480 ;line++)//7、解决图片上下颠倒问题
	{
		for(i=0; i<=800 ; i++)
		mi_buf[800*line + i] = temp_buf[ (479-line)*800 + i ];
	}

	//8、特殊动画“向上飞入”效果算法
	for(i=479; i>=0; i--)
	{
		for(j=0; j<800; j++)
		{
			pic_up_fd[800*i+j] = mi_buf[800*i+j];	
		}
		usleep(500);
	} 	
	
	return 0;
}

//向上飞入”效果算法


int pic_mid_spread(char *pathname, int *pic_mid_spread_fd)
{	
	int ret=0;
	int line=0;
	int block=0;
	int i=0, j=0;
	int bmp_fd=0;
	
	char bmp_buf[480*800*3];
	int mi_buf[480*800];	
	int temp_buf[480*800];	
	bzero(mi_buf,sizeof(mi_buf));
	bzero(bmp_buf,sizeof(bmp_buf));
	bzero(temp_buf,sizeof(temp_buf));

	bmp_fd = open(pathname , O_RDONLY);//1、打开BMP格式图片
	if(bmp_fd == -1)
	{
		printf("pic_mid_spread(), open bmp failed\n");
		return -1;
	}

	ret = lseek(bmp_fd, 54 , SEEK_SET);//2、跳过bmp图片的前54个位置
	if(ret == -1)
	{
		perror("pic_mid_spread(), lseek bmp failed\n");		
		return -1;
	}

	ret = read(bmp_fd , bmp_buf , sizeof(bmp_buf)); //4、取读图片像素
	if(ret == -1)
	{
		printf("pic_mid_spread(), read bmp failed\n");	
		return -1;
	}
	
	close(bmp_fd);//5、关闭图片

	for(i=0, j=0 ; i<800*480 ; i++, j+=3)//6、24bits 转 32bits控制变量
	{
		temp_buf[i] = bmp_buf[j+2]<<16 | bmp_buf[j+1]<<8 | bmp_buf[j] ;
	}

	for(line=0 ; line <=480 ;line++)//7、解决图片上下颠倒问题
	{
		for(i=0; i<=800 ; i++)
		mi_buf[800*line + i] = temp_buf[ (479-line)*800 + i ];
	}	

	//8、特殊动画“中间展开”效果算法
	for(i=0; i<800/2; i++)
	{
		for(j=0; j<480; j++)
		{
			pic_mid_spread_fd[800/2+800*j+i] = mi_buf[800/2+800*j+i];
			pic_mid_spread_fd[800/2+800*j-i] = mi_buf[800/2+800*j-i];
		}
		usleep(500);
	}  

	return 0;
}

//中间展开”效果算法



//编译命令：arm-linux-gcc main.c  uart.c



int main()
{
	int card_id=0;
	//处理屏幕设备
	lcd_init();
	
	//打开触摸屏
	touch_lcdfd = open( "/dev/input/event0" , O_RDONLY);
	if(touch_lcdfd < 0)
	{
		perror("open fd error");
		return -1;
	}
	
	int fd_tty;
	fd_tty = open("/dev/ttySAC1", O_RDWR);
	if(fd_tty == -1)
	{
		printf("open serial fail!");
		return -1;
	}
	
	
	
	while(1)   //初始展示画面
	{
	
	pic_up("./kk1.bmp", lcd_fp);
	usleep(1000);	//延时1000微秒 == 1毫秒
	sleep(2);
	
	
	pic_up("./kk2.bmp", lcd_fp);
	usleep(1000);	//延时1000微秒 == 1毫秒
	sleep(2);
	
	pic_up("./kk3.bmp", lcd_fp);
	usleep(1000);	//延时1000微秒 == 1毫秒
	sleep(2);
	
	touch_lcd();
	
	if (x > 0 && x <800 && y > 0 && y < 480) //密码解锁
       {
		pic_up("./gk.bmp", lcd_fp);
		usleep(1000);
		show_bmp("./gk.bmp");   //解锁方式界面
		break;
	   }
}
	
	
	while(1)   //解锁方式判断界面
	{
		jiesuo=0;
		touch_lcd();	//调用一次函数，获取一次触摸数据
		printf("单次点击坐标点为：(%d , %d)\n", x, y);
		
		if (x > 0 && x < 400 && y > 0 && y < 480) //密码解锁
        {
            printf("密码解锁\n");
			pic_mid_spread("./2001.bmp", lcd_fp);
			usleep(1000);	//延时1000微秒 == 1毫秒
            show_bmp("./2001.bmp");
            jiesuo=1;
			break;
        }
		if (x > 400 && x < 800 && y > 0 && y < 480)
		{
			printf("刷卡解锁\n");
			pic_mid_spread("./sk.bmp", lcd_fp);
			usleep(1000);	//延时1000微秒 == 1毫秒
            show_bmp("./sk.bmp");
            jiesuo=2;
			break;
			
		}
	}
		
		switch (jiesuo)  //解锁界面
       {
        case 1:
			{
				show_bmp("./2001.bmp");
				eng=0;
				password_fun();
				
				if(keynum1==0)
					
					printf("输入密码错误次数过多，请采用刷卡形式解锁\n");
					show_bmp("./sk.bmp");
					while(1)
					{printf("请刷卡\n");
						init_tty(fd_tty);
	
							//：这里是配置读卡器读取卡片信息的
							send_A_command(fd_tty);//请求

							//防碰撞
	
							card_id = send_B_command(fd_tty);
								//获取到了卡号（只作信息的获取）
							printf("\nIC id: %#X\n", card_id);
							//判断卡号是否为 标记的卡号 
							if(card_id == 0XADC13953)
							{
								//提示:这是正确的卡号
								printf("这是正确的卡号\n");
								show_bmp(".123.bmp");
								break;
								//显示一张图片提示
			
							}
							else
							{
								show_bmp(".sks.bmp");
								sleep(1);
								printf("密码错误，重新刷卡\n");
							}
					}
					
					
				break;
			}
        case 2:
            show_bmp("./sk.bmp");
			eng=0;
			keynum1==0;
			while(1)
			{
				printf("请刷卡\n");
			
				init_tty(fd_tty);
		
				//：这里是配置读卡器读取卡片信息的
				send_A_command(fd_tty);//请求

				//防碰撞
		
				card_id = send_B_command(fd_tty);
				//获取到了卡号（只作信息的获取）
				printf("\nIC id: %#X\n", card_id);
				//判断卡号是否为 标记的卡号 
				if(card_id == 0XADC13953)
				{
				//提示:这是正确的卡号
				printf("这是正确的卡号\n");
				show_bmp("./123.bmp");
				break;
				//显示一张图片提示
				
				}
				else
				{printf("密码错误，重新刷卡\n");
					show_bmp(".sks.bmp");
					sleep(1);
					eng++;
					
				}
				
				if(eng==3)
				{printf("刷卡密码错误次数过多，请采用输入密码形式解锁\n");
					show_bmp("./2001.bmp");
					while(keynum1!=6)
					{
						eng=0;
						password_fun();
						
					}	
					
							
				}			
							
						
			}
			break;	
			
          
	   }
	
	
	
	pic_up("./123.bmp", lcd_fp);
	usleep(1000);	//延时1000微秒 == 1毫秒
	//pic_up("./123.bmp", lcd_fp);
	show_bmp("./123.bmp");
	printf("进入系统\n");
	
	
	while(1)   //进入系统操作显示图片
	{
		//读触摸屏的触摸数值,拿取x\y轴坐标值数据
		touch_lcd();	//调用一次函数，获取一次触摸数据
		
		printf("单次点击坐标点为：(%d , %d)\n", x, y);
		
		if(x>0 && x < 100 && y >0 && y<100)
		{
			printf("点击了左上角\n");
			//显示指定路径的图片
			show_bmp("./kk2.bmp");
		}
		if(x>0 && x < 100 && y >380 && y<480)
		{
			printf("点击了左下角\n");
			show_bmp("./kk1.bmp");
		}
		if(x>700 && x < 800 && y >0 && y<100)
		{
			printf("点击了右上角\n");
			show_bmp("./kk3.bmp");
		}
		if(x>700 && x < 800 && y >380 && y<480)
		{
			printf("点击了右下角\n");
			show_bmp("./985.bmp");
		}
	}
	
	
	


	//解除映射内存
	munmap(lcd_fp, 800*480*4);	//内存空间800*480*4总字节
	//关闭屏幕
	close(lcd_fd);
	
	return 0;

}
