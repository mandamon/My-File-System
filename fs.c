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

void myshowfile(int start, int end, char file_name[5])	//myshowfile명령어(start : 읽기 시작할 바이트, end : 읽는걸 끝낼 바이트, file_name : 읽을 파일)
{
	int cnt=0;
	int cur_dnum;
	int tmp_i;
	start-=1;
	end-=1;
	while(start >= 128)	//start의 크기로 몇번째 data block부터 읽기 시작할지를 cnt에 저장한다
	{
		++cnt;
		start-=128;
		end-=128;
	}
	for(int i=1; i<512; ++i)
	{
		if(strcmp(tree[i].name, file_name) == 0 && tree[i].next->inum == cur_inum)
		{
			tmp_i=i;
			break;
		}
	}
	cur_dnum = mfs.inode[tmp_i].direct;	//cur_dnum에 첫번째 data block의 위치를 저장한다
	for(int i=0; i<cnt; ++i)	//cnt만큼 cur_dnum을 옮긴다
	{
		cur_dnum = mfs.data[cur_dnum].file.next->file.dnum;
	}
	if(end<128)	//파일의 내용을 출력한다
	{
		for(int i=start; i<=end; ++i)
		{
			printf("%c",mfs.data[cur_dnum].file.data[i]);
		}
	}
	else if(end>=128)
	{
		while(1)
		{
			if(end>=128)
			{
				for(int i=start; i<128; ++i)
				{
					printf("%c",mfs.data[cur_dnum].file.data[i]);
				}
				cur_dnum=mfs.data[cur_dnum].file.next->file.dnum;
				start=0;
				end-=128;
			}
			else if(end<=128)
			{
				for(int i=start; i<=end; ++i)
				{
					printf("%c",mfs.data[cur_dnum].file.data[i]);
				}
				break;
			}
		}
	}
	printf("\n");
	return;
}
void myrmdir(char file_name[5])	//myrmdir 명령어
{	
	int sw=0;
	for(int i=1; i<512; ++i)	//입력된 directory명과 일치하는 directory를 찾음
	{
		if(strcmp(tree[i].name, file_name)==0 && tree[i].next->inum == cur_inum && mfs.inode[i].dir_or_file == Dir)
		{
			if(tree[i].cnt_file != 0)	//해당 directory의 하위파일이 존재할 경우 오류메시지 출력
			{
				printf("해당 디렉토리가 비어있지 않습니다.\n");
				return;
			}
			delete(i, file_name);	//입력된 inode번호로 delete함수 실행
			mfs.inode[cur_inum].size -= 42;	//현재 directory의 크기를 42줄임
			--cnt_file;
			break;
		}
		if(i==511)	//마지막까지 입력된 directory를 찾지 못했을 경우 오류 메시지 출력
			printf("해당 디렉토리가 존재하지 않습니다.\n");
	}
	return;
}
void delete(int a, char file_name[5])	//a번 아이노드와 관련된 정보를 지우는 함수(a : 삭제할 파일의 inode번호, file_name : 삭제할 파일의 이름)
{
	int sw=0;
	int cur_dnum;
	if(mfs.inode[a].direct !=0)
	{
		delete_data(mfs.inode[a].direct);	//direct block가 가리키는 data block을 삭제함
		if(mfs.inode[a].single_indirect != 0)	//single indirect block이 가리키는 data block이 있을 경우 관련된 data block들을 삭제함
		{
			for(int i=0; i<mfs.data[mfs.inode[a].single_indirect].indirect.cnt; ++i)
			{
				delete_data(mfs.data[mfs.inode[a].single_indirect].indirect.num[i]);
			}
			mfs.data[mfs.inode[a].single_indirect].indirect.cnt=0;
			delete_data(mfs.inode[a].single_indirect);
			if(mfs.inode[a].double_indirect != 0)	//double indirect block이 가리키는 data block이 있을 경우 관련된 data block들을 삭제함
			{
				for(int i=0; i<mfs.data[mfs.inode[a].double_indirect].indirect.cnt; ++i)
				{
					cur_dnum = mfs.data[mfs.inode[a].double_indirect].indirect.num[i];
					for(int j=0; j<mfs.data[cur_dnum].indirect.cnt; ++j)
					{
						delete_data(mfs.data[cur_dnum].indirect.num[j]);
					}
					delete_data(mfs.data[mfs.inode[a].double_indirect].indirect.num[i]);
					mfs.data[mfs.data[mfs.inode[a].double_indirect].indirect.num[i]].indirect.cnt=0;
				}
				mfs.data[mfs.inode[a].double_indirect].indirect.cnt=0;
				delete_data(mfs.inode[a].double_indirect);
			}
		}
	}
	cur_dnum = mfs.inode[cur_inum].direct;
	sw=0;
	for(int i=0; i<mfs.data[cur_dnum].directory.cnt_file; ++i)
	{
		if(strcmp(mfs.data[cur_dnum].directory.d_name[i], file_name)==0)
		{
			sw=1;
			for(int j=i; j<mfs.data[cur_dnum].directory.cnt_file - 1 ; ++j)	//삭제한 파일과 관련된 현재 directory의 data block의 내용을 삭제함
			{
				strcpy(mfs.data[cur_dnum].directory.d_name[j], mfs.data[cur_dnum].directory.d_name[j+1]);
				mfs.data[cur_dnum].directory.d_inum[j] = mfs.data[cur_dnum].directory.d_inum[j+1];
			}	
			mfs.data[cur_dnum].directory.d_inum[mfs.data[cur_dnum].directory.cnt_file-1] = 0;
			memset(mfs.data[cur_dnum].directory.d_name[mfs.data[cur_dnum].directory.cnt_file-1], 0, sizeof(char) * 5);
			--mfs.data[cur_dnum].directory.cnt_file;
			if(mfs.data[cur_dnum].directory.cnt_file == 0)	//direct block이 가리키는 data block이 비었을 때 할당했던 data block을 돌려받음
			{
				mfs.super.data[mfs.inode[cur_inum].direct]=0;
				mfs.inode[cur_inum].direct = 0;
			}
		}
		if(sw==1)
		{
			break;
		}
	}
	if(sw==0 && mfs.inode[cur_inum].single_indirect != 0)	//삭제한 파일과 관련된 현재 directory의 data block의 내용을 삭제함
	{
		for(int i=0; i<mfs.data[mfs.inode[cur_inum].single_indirect].indirect.cnt; ++i)
		{
			cur_dnum = mfs.data[mfs.inode[cur_inum].single_indirect].indirect.num[i];
			for(int j=0; j<mfs.data[cur_dnum].directory.cnt_file; ++j)
			{
				if(strcmp(mfs.data[cur_dnum].directory.d_name[j], file_name)==0)
				{
					sw=1;
					for(int k=j; k<mfs.data[cur_dnum].directory.cnt_file - 1; ++k)
					{
						strcpy(mfs.data[cur_dnum].directory.d_name[k], mfs.data[cur_dnum].directory.d_name[k+1]);
						mfs.data[cur_dnum].directory.d_inum[k] = mfs.data[cur_dnum].directory.d_inum[k+1];
					}
					mfs.data[cur_dnum].directory.d_inum[mfs.data[cur_dnum].directory.cnt_file-1] = 0;
					memset(mfs.data[cur_dnum].directory.d_name[mfs.data[cur_dnum].directory.cnt_file - 1], 0, sizeof(char) * 5);
					--mfs.data[cur_dnum].directory.cnt_file;
					if(mfs.data[cur_dnum].directory.cnt_file == 0)
					{
						mfs.super.data[cur_dnum]=0;
						for(int k=0; k<mfs.data[mfs.inode[cur_inum].single_indirect].indirect.cnt; ++k)
						{
							if(mfs.data[mfs.inode[cur_inum].single_indirect].indirect.num[k] == cur_dnum)
							{
								for(int l=k; l<mfs.data[mfs.inode[cur_inum].single_indirect].indirect.cnt-1; ++l)
								{
									mfs.data[mfs.inode[cur_inum].single_indirect].indirect.num[l] = mfs.data[mfs.inode[cur_inum].single_indirect].indirect.num[l+1];
								}
								mfs.data[mfs.inode[cur_inum].single_indirect].indirect.num[mfs.data[mfs.inode[cur_inum].single_indirect].indirect.cnt-1]=0;
								--mfs.data[mfs.inode[cur_inum].single_indirect].indirect.cnt;
							}
						}
						if(mfs.data[mfs.inode[cur_inum].single_indirect].indirect.cnt == 0)	//single indirect block이 가리키는 data block의 내용이 비었을 경우 할당했던 data block을 돌려받음
						{
							mfs.super.data[mfs.inode[cur_inum].single_indirect]=0;
							mfs.inode[cur_inum].single_indirect = 0;
						}
					}
					break;
				}
			}
			if(sw==1)
				break;
		}
	}
	mfs.inode[a].year = 0;
	mfs.inode[a].mon = 0;
	mfs.inode[a].day = 0;
	mfs.inode[a].hour = 0;
	mfs.inode[a].min = 0;
	mfs.inode[a].sec = 0;
	mfs.inode[a].size = 0;
	mfs.super.inode[a] = 0;
	memset(tree[a].name, 0, sizeof(char) * 5);
	--tree[a].next->cnt_file;
	tree[a].inum = 0;
	tree[a].cnt_file = 0;
	tree[a].next = NULL;
	mfs.inode[a].dir_or_file = 0;
	return;
}
void delete_data(int a)	//a번째 data block의 내용을 삭제함
{
	if(data_state[a] == Dir)	//a번 data block이 directory일 경우
	{
		for(int i=0; i<mfs.data[a].directory.cnt_file; ++i)
		{
			memset(mfs.data[a].directory.d_name[i], 0, 5);
			mfs.data[a].directory.d_inum[i] = 0;
		}
		mfs.data[a].directory.cnt_file=0;
	}
	else if(data_state[a] == File)	//a번 data block이 file일 경우
	{
		mfs.data[a].file.dnum = 0;
		memset(mfs.data[a].file.data, 0, 129);
		mfs.data[a].file.next = NULL;
	}
	else if(data_state[a] == Ind)	//a번 data block이 indirect data block일 경우
	{
		for(int i=0; i<mfs.data[a].indirect.cnt; ++i)
		{
			mfs.data[a].indirect.num[i]=0;
		}
		mfs.data[a].indirect.cnt=0;
	}
	data_state[a]=0;
	mfs.super.data[a]=0;
}
void mytouch(char file_name[5])	//mytouch며령어
{
	int save_cur_inum=cur_inum;
	if(exist(file_name) == 1)		//현재 존재하는 파일의 시간 변경
	{
		for(int i=1; i<512; ++i)
		{
			if(strcmp(tree[i].name, file_name)==0 && tree[i].next->inum==cur_inum)
			{
				inode_time(i);
			}
		}
		return;
	}
	else if(exist(file_name)==0)	//새로 파일 생성(mymkdir명령어 실행 후 파일의 종류만 file로 변경)
	{
		mymkdir(file_name);
		for(int i=1; i<512; ++i)
		{
			if(strcmp(tree[i].name, file_name) == 0 && tree[i].next->inum == cur_inum)
			{
				mfs.inode[i].dir_or_file = File;
			}
		}
	}
}
