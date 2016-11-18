#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define Dir 1
#define File 0
#define SUPER_I(x) i##x
#define SUPER_D(x) d##x
int cur_inum, cur_dnum;	//현재 inode, data block 위치
int cnt_directory;	//directory의 갯수
typedef struct direct_data_block{
	char name[4];
	char data[128];
	union data_block *next;
} direct;
typedef struct indirect_data_block{
	int inum[12];
} indirect;
typedef struct directory_data_block{
	int cnt_file;
	char d_name[10][4];
	int d_inum[10];
} directory;
union data_block{
	direct direct;
	indirect indirect;
	directory directory;
};
struct inode_list{
	_Bool dir_or_file;
	int year, mon, day, hour, min, sec;
	int size;
	union data_block *direct;
	union data_block *single_indirect;
	union data_block *double_indirect;
};
struct super_block{
	unsigned i0 : 1, i1 : 1, i2 : 1, i3 : 1,
			 d0 : 1, d1 : 1, d2 : 1, d3 : 1,
			 d4 : 1, d5 : 1, d6 : 1, d7 : 1;
};
struct my_file_system{
	unsigned boot_block : 16;
	struct super_block super[128];
	struct inode_list inode[512];
	union data_block data[1024];
};
struct directory_tree{
	char name[4];
	struct directory_tree *next;
};

struct my_file_system mfs;
struct directory_tree tree[512];
void mymkdir(char *);
int super_i(int , int, int );
int super_d(int , int, int );
void inode_time(int );

int main()
{
	char a[100];
	char *p;
	while(1)
	{
		scanf("%s",a);
		if(strcmp(a,"byebye")==0)
			return 0;
		else if(a[0]=='m' && a[1]=='y')
		{
			if(strcmp(a,"mymkfs")==0)
			{
				if(mfs.super[0].i0==1)
					printf("이미 myfs파일이 있습니다.\n");
				else
				{
					system("touch myfs");
					cur_inum=0;
					mfs.super[0].i0=1;
					cnt_directory=1;
					tree[0].name[0]='/';
					tree[0].name[1]='\0';
					cur_dnum=0;
					mfs.inode[0].direct = &mfs.data[0];
					mfs.super[0].d0=1;
				}
			}
			else if(strcmp(a,"mymkdir")==0)
			{
				char name[10];
				scanf("%4s",name);
				mymkdir(name);
			}
		}
		else
		{
			p=a;
			system(p);
		}
	}
}
void mymkdir(char *name)
{
	int tmp=0, tmp2=0;
	++cnt_directory;
	for(int i=1; i<512; ++i)	//super block에서 사용하지 않는 inode 찾기
	{
		int a,b;
		a=(i-(i%4))/4;
		b=i%4;
		tmp=super_i(a,b,i);
		if(tmp==1)
		{
			tmp=i;
			break;
		}
	}
	for(int i=1; i<1024; ++i)	//super block에서 사용하지 않는 data block 찾기
	{
		int a,b;
		a=(i-(i%8))/8;
		b=i%8;
		tmp2=super_d(a,b,i);
		if(tmp2==1)
		{
			tmp2=i;
			break;
		}
	}
	strcpy(tree[tmp].name,name);	//tree에 이름 입력
	mfs.inode[tmp].dir_or_file = Dir;
	mfs.inode[tmp].direct = &mfs.data[tmp2];	//새로 만든 directory에 data block 연결
	mfs.inode[cur_inum].direct->directory.cnt_file++;	//현재 directory의 data blcok에 파일 개수 +1;
	strcpy(mfs.inode[cur_inum].direct->directory.d_name[mfs.inode[cur_inum].direct->directory.cnt_file], name);	//현재 directory의 data blcok에 새로만든 파일 이름 저장
	mfs.inode[cur_inum].direct->directory.d_inum[mfs.inode[cur_inum].direct->directory.cnt_file]=tmp;	//현재 directory의 data block에 새로만든 파일의 inode 번호 저장
	tree[tmp].next = &tree[cur_inum];	//현재 directory와 새로만든 directory를 tree구조로 연결 (새로만든 directory -> 현재 directory)
	for(int i=1; i<=mfs.inode[cur_inum].direct->directory.cnt_file;++i)
	{
		int t;
		t=mfs.data[0].directory.d_inum[i];
		printf("%s %d\n",mfs.data[0].directory.d_name[i], mfs.data[0].directory.d_inum[i]);
		printf("Dir_of_File : %d\n",mfs.inode[t].dir_or_file);
		printf("날짜 : %d년 %d월 %d일 %d시 %d분 %d초\n",mfs.inode[t].year, mfs.inode[t].mon, mfs.inode[t].day, mfs.inode[t].hour, mfs.inode[t].min, mfs.inode[t].sec);
		printf("크기 : %d\n",mfs.inode[t].size);
	}
}
int super_i(int a, int b, int i)	//사용중이 아닌 inode번호 찾기
{
	if(mfs.super[a].SUPER_I(0)==0)
	{
		mfs.super[a].SUPER_I(0)=1;
		++cnt_directory;
		inode_time(i);
		return 1;
	}
	if(b==0)
		return 0;
	else if(mfs.super[a].SUPER_I(1)==0)
	{
		mfs.super[a].SUPER_I(1)=1;
		++cnt_directory;
		inode_time(i);
		return 1;
	}
	if(b==1)
		return 0;
	else if(mfs.super[a].SUPER_I(2)==0)
	{
		mfs.super[a].SUPER_I(2)=1;
		++cnt_directory;
		inode_time(i);
		return 1;
	}
	if(b==2)
		return 0;
	else if(mfs.super[a].SUPER_I(3)==0)
	{
		mfs.super[a].SUPER_I(3)=1;
		++cnt_directory;
		inode_time(i);
		return 1;
	}
	return 0;
}
int super_d(int a, int b, int i)	//사용중이 아닌 data block찾기
{
	if(mfs.super[a].SUPER_D(0)==0)
	{
		mfs.super[a].SUPER_D(0)=1;
		++cnt_directory;
		return 1;
	}
	if(b==0)
		return 0;
	else if(mfs.super[a].SUPER_D(1)==0)
	{
		mfs.super[a].SUPER_D(1)=1;
		++cnt_directory;
		return 1;
	}
	if(b==1)
		return 0;
	else if(mfs.super[a].SUPER_D(2)==0)
	{
		mfs.super[a].SUPER_D(2)=1;
		++cnt_directory;
		return 1;
	}
	if(b==2)
		return 0;
	else if(mfs.super[a].SUPER_D(3)==0)
	{
		mfs.super[a].SUPER_D(3)=1;
		++cnt_directory;
		return 1;
	}
	if(b==3)
		return 0;
	else if(mfs.super[a].SUPER_D(4)==0)
	{
		mfs.super[a].SUPER_D(4)=1;
		++cnt_directory;
		return 1;
	}
	if(b==4)
		return 0;
	else if(mfs.super[a].SUPER_D(5)==0)
	{
		mfs.super[a].SUPER_D(5)=1;
		++cnt_directory;
		return 1;
	}
	if(b==5)
		return 0;
	else if(mfs.super[a].SUPER_D(6)==0)
	{
		mfs.super[a].SUPER_D(6)=1;
		++cnt_directory;
		return 1;
	}
	if(b==6)
		return 0;
	else if(mfs.super[a].SUPER_D(7)==0)
	{
		mfs.super[a].SUPER_D(7)=1;
		++cnt_directory;
		return 1;
	}
	return 0;
}

void inode_time(int a)
{
	struct tm *t;
	time_t now;
	now=time(NULL);
	t=localtime(&now);
	mfs.inode[a].year = t->tm_year + 1900;
	mfs.inode[a].mon = t->tm_mon + 1;
	mfs.inode[a].day = t->tm_mday;
	mfs.inode[a].hour = t->tm_hour;
	mfs.inode[a].min = t->tm_min;
	mfs.inode[a].sec = t->tm_sec;
}
