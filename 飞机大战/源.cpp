#include<stdio.h>
#include<graphics.h>
#include<Windows.h>
#include<time.h>
#define BJ_width 1408
#define	 BJ_height  800
#define  WJ_bullet  15//一次发射的子弹数量
#define  DR_enemy   10//敌机的数量
#define  DR_bullet    10//敌机射出的子弹

IMAGE bk;//背景
IMAGE me;//我方飞机
IMAGE bull;//子弹
IMAGE emy;//敌机
IMAGE emybull;//敌机子弹
IMAGE happyend;//通关
IMAGE badend;//失败
int score = 0;
int life = 10;
int i, x;
int y = 0;
int code = 123456;

DWORD t1, t2;
//定义飞机结构体
struct  Plane
{
	int x;
	int y;
	bool flag;
	int width;
	int height;
}flyme, bullet[WJ_bullet], enemy[DR_enemy], enemybullet[DR_bullet], kunge, kungege;

//设置定时器
bool Timer(int ms, int id)
{
	static int start[5];
	int end = clock();
	if(end - start[id] >= ms)
	{
		start[id] = end;
		return true;
	}
	return false;
}

//初始化界面
void GameInit() 
{ 
    initgraph(BJ_width, BJ_height,SHOWCONSOLE);//创建一个图形化界面
	srand(GetTickCount());
	loadimage(&bk, "./tp/beijin.png");
	//loadimage(&bk, "./tp/shejie.jpg");//将图片放入图形化界面中
	loadimage(&me, "./tp/fly2.png");//将自己飞机放入图形化界面中
	loadimage(&bull, "./tp/zidan1.jfif");//将子弹放入图形化界面中
	loadimage(&emy, "./tp/diren1.png");//将敌机放入图形化界面中
	loadimage(&emybull, "./tp/zidan3.jfif");//将敌机子弹放入图形化界面中
	loadimage(&happyend, "./tp/kunge.jpg");//将胜利背景放入
	loadimage(&badend, "./tp/kungege.jpg");//将失败背景放入
	flyme.height = 55;
	flyme.width = 38; 
	flyme.x = BJ_width / 2 - flyme.width / 2;//飞机x位置
	flyme.y = BJ_height - flyme.height;//飞机y位置
	flyme.flag = true;
	for(int i=0 ; i < WJ_bullet ; i++ )//子弹的初始化
	{
		bullet[i].flag = false;
		bullet[i].height = 25;
		bullet[i].width = 20;
	}
	t1 = t2 = GetTickCount(); //初始化子弹发出时间
	for(int i =0 ; i < DR_enemy ; i++)//初始化敌机
	{
		enemy[i].flag = false;
		enemy[i].width = 57;
		enemy[i].height = 74;

	}
	for (int i = 0; i < DR_bullet; i++)//敌机子弹初始化
	{
		enemybullet[i].flag = false;
		enemybullet[i].height = 17;
		enemybullet[i].width = 10;
	}
	kunge.x = 270;
	kunge.y = 100;
	kungege.x = 400;
	kungege.y = 100;

}

//输出图形，绘制函数
void GameDarw()
{
	BeginBatchDraw();
	putimage(0,  0,  &bk);//输出背景图片
	if (flyme.flag) //判断飞机是否存活，存活进入
	{
		putimage(flyme.x, flyme.y, &me);//输出自己飞机图片
	}
	for(int i = 0 ; i< WJ_bullet ; i++)//发射全部子弹
	{
		if (bullet[i].flag)//判断子弹是否存活，存活进入
		{
			putimage(bullet[i].x, bullet[i].y, &bull);//输出子弹图片
		}
	}
	for(int i =0 ; i < DR_enemy; i++)//产生所有敌机
	{
		if (enemy[i].flag)//判断敌机是否存活，存活进入
		{
			putimage(enemy[i].x, enemy[i].y, &emy);//输出敌军图片
		}
	}
	for (int i = 0; i < DR_bullet; i++) //发射敌机子弹
	{
		if(enemybullet[i].flag)
		{
			putimage(enemybullet[i].x, enemybullet[i].y, &emybull);
		}
	}
	outtextxy(0, 0, "得分:");//将得分输出到游戏中
	char s[5];
	sprintf_s(s, "%d", score);
	outtextxy(36, 0, s);
	if(score>=66)
	{
		cleardevice();
		putimage(kunge.x, kunge.y, &happyend);
		outtextxy(610, 590, "游戏结束你已通关！");
		enemy->flag = false;
		enemybullet->flag = false;
		flyme.flag = false;
	}
	outtextxy(0, 17, "生命:");//将生命输出到游戏中
	char c[5];
	sprintf_s(c, "%d", life);
	outtextxy(36, 17,c);
	if(life<=0)
	{
		cleardevice();
		putimage(kungege.x, kungege.y, &badend);
		outtextxy(610, 590, "很遗憾游戏失败");
		enemy->flag = false;
		enemybullet->flag = false;
		flyme.flag = false;
	}
	EndBatchDraw();
}

//小窗口
void Window()
{
	HWND hnd = GetHWnd();
	MessageBox(hnd, "密码正确，是否开始游戏", "提示", MB_OK);
}

//生成子弹
void CreatBullet()
{
	for (int i = 0; i < WJ_bullet; i++)
	{
		if (!bullet[i].flag)//如果子弹不存活，产生子弹
		{
			bullet[i].x = flyme.x + 10;
			bullet[i].y = flyme.y;
			bullet[i].flag = true;
			break;
		}
	}
}

//子弹移动
void Bulltemove(int p)
{
	for (int i = 0; i < WJ_bullet; i++)
	{
		if (bullet[i].flag)
		{
			p = bullet[i].y--;
			if(bullet[i].y < 0)//如果子弹超过了y=0，子弹变为不存活
			{
				bullet[i].flag = false;
			}
		}
	}
}

//生成敌机
void CreatEnemy() 
{
	for (int i = 0; i < DR_enemy; i++)
	{
		if (!enemy[i].flag)
		{
			if(Timer(100,0))
			{
				enemy[i].x = rand() % (BJ_width - enemy[i].width);
				enemy[i].y = 0;
				enemy[i].flag = true;
				break;
			}	
		}
	}
}

//敌机移动
void Enemymove(int p)
{
	if(Timer(10,1))
	{ 
		for (int i = 0; i < DR_enemy; i++)
		{
			if (enemy[i].flag)
			{
				p = enemy[i].y++;
				if (enemy[i].y > BJ_height)
					{
						enemy[i].flag = false;
					}
			}
		}
	}
}

//生成敌机子弹
void Enemybullet()
{
	for (int i = 0; i < DR_bullet; i++)
	{
		if (!enemybullet[i].flag)
		{
			if(Timer(100,3))
			{
				enemybullet[i].x = enemy[i].x + 22;
				enemybullet[i].y = enemy[i].y + 67;
				enemybullet[i].flag = true;
				break;
			}
		}
	}
}

//敌机子弹移动
void Enemybulletmove(int p)
{
	if (Timer(2, 2)) 
	{
		for (int i = 0; i < DR_bullet; i++)
		{
			if (enemybullet[i].flag)
			{
				p = enemybullet[i].y++;
				if (enemybullet[i].y > BJ_height)
				{
					enemybullet[i].flag = false;
				}
			}
		}
	}
	
}

//飞机移动
void Control( int p)
{
	if (GetAsyncKeyState(VK_UP) && flyme.y>=0) //向上
	{
		p = flyme.y--;
	}
	if (GetAsyncKeyState(VK_DOWN) && flyme.y + flyme.height<= BJ_height)//向下
	{
		p = flyme.y++;
	}
	if (GetAsyncKeyState(VK_LEFT) && flyme.x + flyme.width/2>=0)//向左
	{
		p = flyme.x--;
	}
	if (GetAsyncKeyState(VK_RIGHT) && flyme.x + flyme.width/2<= BJ_width)//向右
	{
		p = flyme.x++;
	}
	if (GetAsyncKeyState(VK_SPACE) && t2-t1 >200)//发射
	{
		CreatBullet();
		printf_s("11111");
		t1 = t2;
	}
	t2 = GetTickCount();
}

//射击敌机
void Bullteenemy()
{
	for (int i = 0; i < DR_enemy; i++) 
	{
		if (!enemy[i].flag)//判断敌机是否已消失，消失则跳出
			continue;
		for (int j = 0; j < WJ_bullet; j++)
		{
			if (!bullet[j].flag)//判断子弹是否已消失，消失则跳出
				continue;
			if (bullet[j].x > enemy[i].x && bullet[j].x < enemy[i].x + enemy[i].width &&
				bullet[j].y > enemy[i].y && bullet[j].y < enemy[i].y + enemy[i].height)//判断子弹是否击中敌机
			{
				enemy[i].flag = false;
				bullet[i].flag = false;
				score = score + 2;
			}
		}
	}

}

//敌机射击
void Gamelife()
{
	for(int i =0;i<DR_bullet; i++)
	{
		if (!enemybullet[i].flag)
			continue;
		if (enemybullet[i].x > flyme.x && enemybullet[i].x < flyme.x + flyme.width &&
			enemybullet[i].y > flyme.y && enemybullet[i].y < flyme.y + flyme.height)//判断敌机子弹是否击中我方
		{
			enemybullet[i].flag = false;
			life = life - 1;
		}
	}
}

//相互碰撞
void Ganmelifeenemy()
{
	for(int i =0;i<DR_enemy;i++)
	{
		if (!enemy[i].flag)
			continue;
		if (flyme.x < enemy[i].x + enemy[i].width &&  flyme.x +flyme.width > enemy[i].x  &&
			flyme.y < enemy[i].y + enemy[i].height && flyme.y +flyme.height >enemy[i].y)//判断敌机是否撞到我方
		{
			enemy[i].flag = false;
			life = life - 1;
		}
	}
}

//子弹相撞
void Gameenemybull()
{
	for(int i =0;i<DR_bullet;i++)
	{
		if (!enemybullet[i].flag)
			continue;
		if (bullet->x-6 < enemybullet[i].x +enemybullet[i].width && bullet->x + bullet->width +5 > enemybullet[i].x  &&
			bullet->y <= enemybullet[i].y +enemybullet[i].height )
		{
			enemybullet[i].flag = false;
			bullet->flag = false;
		}
	}
}

int main() 
{
	printf("**************************欢迎来到飞机大战*************************\n");
	for (i = 0; i < 3; i++)
	{
		printf("用户名称:611313651313\n");
		printf("请输入密码:");
		scanf_s("%d", &x);
		if (x == code)
		{          
			printf("登陆成功\n");
			GameInit();
			Window();
			getchar();
			while (1)
			{
				GameDarw();
				Control(1);
				Bulltemove(1);
				CreatEnemy();
				Enemybullet();
				Enemymove(1);
				Enemybulletmove(1);
				Bullteenemy();
				Gamelife();
				Ganmelifeenemy();
				Gameenemybull();
			}
			y = 1;  
			break;
		}
		else {
			printf("密码错误请重新输入\n");
		}
	}
	if (y == 0) {
		printf("\n");
		printf("输入密码次数已到,请退出");
	}
	getchar();
	return 0;
}