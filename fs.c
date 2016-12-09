#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define Dir 'd'
#define File '-'
#define Ind 'I'
int cur_inum=0, cur_dnum=0;	//현재 inode, data block 위치
int cnt_file=0;	//directory의 갯수
int cnt[512]={0};
int sw=0;
int tmp=1;
char data_state[1024];	//union data block이 file인지 indirect인지 directory인지 구분
char a[100], b[100];	//명령을 저장
typedef struct file_data_block{	//union data block의 멤버1(file)
	int dnum;
	char name[5];
	char data[129];
	union data_block *next;
} file;
typedef struct indirect_data_block{	//union data block의 멤버2(indirect block)
	int cnt;
	int num[102];
} indirect;
typedef struct directory_data_block{	//union data block의 멤버3(directory)
	int cnt_file;
	char d_name[24][5];
	int d_inum[24];
} directory;
union data_block{	//data block
	file file;
	indirect indirect;
	directory directory;
};
struct inode_list{	//inode
	char dir_or_file;
	int year, mon, day, hour, min, sec;
	int size;
	int direct;
	int single_indirect;
	int double_indirect;
};
struct super_block{	//super block
	_Bool inode[512];
	_Bool data[1024];
};
struct my_file_system{	//파일 시스템 구조체
	unsigned boot_block : 16;
	struct super_block super;
	struct inode_list inode[512];
	union data_block data[1024];
};
struct directory_tree{	//tree구조를 저장시킨 구조체
	char name[5];
	int cnt_file;
	int inum;
	struct directory_tree *next;	//tree구조체의 next에 상위 디렉토리의 주소가 오도록 저장(ex : /a, /a/b가 있을때, a의 next에는 /가, b의 next에 a가 오도록 저장)
};

struct my_file_system mfs;
struct directory_tree tree[512];
void path(int );
void cut_path(char [], char [], char []);
void myls();
int exist(char []);
void mymkdir(char []);
int super_i();
int super_d();
void inode_time(int );
int mycd(char []);
void myrmdir(char []);
void delete(int , char []);
void mystate();
void mypwd();
void mytree();
void mytouch(char []);
void myshowinode(int );
void divide(char [], char [], char []);
void mycpfrom(char [], char []);
void myshowblock(int );
void mycat(char []);
void delete_data(int );
void mycpto(char [], char []);
void mycp(char [], char []);
void mymv(char [], char []);
void myrm(char []);
void myshowfile(int , int, char []);
