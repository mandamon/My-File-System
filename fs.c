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

int main()
{
	FILE *ifp, *ofp;
	char route[100], file_name[5];
	char name1[100], name2[5];
	char *p;
	char tmp_char[10];
	int e;
	int save_cur_inum=cur_inum;
	int start, end;
	cur_inum=0;
	cnt_file=1;
	tree[0].name[0]='/';
	tree[0].name[1]='\0';
	tree[0].inum=0;
	cur_dnum=0;
	ifp=fopen("myfs","rb");
	if(ifp!=NULL)	//myfs파일이 존재할때, myfs파일을 읽어 이전에 작업하던 내용을 불러온다
	{
		fread(&mfs, sizeof(struct my_file_system), 1, ifp);
		fread(&cnt_file, sizeof(int), 1, ifp);
		fread(&tree, sizeof(struct directory_tree), 512, ifp);
		fread(&data_state, sizeof(char), 1024, ifp);
	}
	fclose(ifp);
	while(1)//myfs_shell
	{
		ifp=fopen("myfs","rb");
		save_cur_inum=cur_inum;	//현재 디렉토리 위치를 저장
		memset(b,0,100);	//a 초기화
		memset(a,0,100);	//b 초기화
		path(cur_inum);	//현재 경로 출력
		gets(a);	//입력한 문자열을 읽어들임
		sw=0;
		for(int i=0; i<=strlen(a); ++i)	//문자열 a에서 명령어가 아닌 부분(옵션, 파일 이름 등)을 잘라내서 b에 저장(ex : mymkdir /a/b/c 에서 /a/b/c만 잘라내 b = "/a/b/c"가 된다)
		{
			if(a[i]==' ')
			{
				for(int j=i+1; j<=strlen(a); ++j)
				{
					b[j-i-1]=a[j];
				}
				break;
			}
		}
		if(strcmp(a,"byebye")==0)	//byebye입력시 종료
			return 0;
		if(ifp==NULL)	//myfs파일이 존재하지 않을때 mymkfs를 입력시 myfs파일을 만듬
		{
			if(a[0]=='m' && a[1]=='y' && a[2]=='m' && a[3]=='k' && a[4]=='f' && a[5]=='s')
			{
				mfs.super.inode[0]=1;
				system("touch myfs");
				mfs.inode[0].dir_or_file = Dir;
				inode_time(0);
				data_state[0]=Dir;
			}
		}
		else
		{
			if(a[0]=='m' && a[1]=='y')
			{
				if(a[2]=='m' && a[3]=='k' && a[4]=='f' && a[5]=='s')	//mymkfs입력시 myfs파일이 이미 있을때 메시지 출력
				{
					printf("이미 myfs파일이 있습니다.\n");
				}
				else if(a[2]=='l' && a[3]=='s')	//myls명령어
				{
					myls(b);
				}
				else if(a[2]=='m' && a[3]=='k' && a[4]=='d' && a[5]=='i' && a[6]=='r')	//mymkdir명령어
				{
					cut_path(b,route,file_name);	//b에 저장된 문자열을 경로와 파일이름으로 분리(ex : b="/a/b/c"일때, route="/a/b", file_name="c")
					if(mycd(route)==1)	//route에 저장된 경로로 이동 후 directory 생성
					{
						mymkdir(file_name);
						cur_inum=save_cur_inum;	//원래 경로로 돌아옴
					}
				}
				else if(a[2]=='c' && a[3]=='d')	//mycd명령어
				{
					mycd(b);
				}
				else if(a[2]=='r' && a[3]=='m' && a[4]=='d' && a[5]=='i' && a[6]=='r')	//myrmdir명령어
				{
					cut_path(b,route,file_name);	//b에 저장된 문자열을 경로와 파일이름으로 분리
					if(mycd(route)==1)	//route에 저장된 경로로 이동 후 directory삭제
					{
						myrmdir(b);
						cur_inum=save_cur_inum;	//원래 경로로 돌아옴
					}
				}
				else if(a[2]=='s' && a[3]=='t' && a[4]=='a' && a[5]=='t' && a[6]=='e')	//mystate명령어
				{
					mystate();
				}
				else if(a[2]=='p' && a[3]=='w' && a[4]=='d')	//mypwd명령어
				{
					mypwd();
				}
				else if(a[2]=='t' && a[3]=='r' && a[4]=='e' && a[5]=='e')	//mytree명령어
				{
					if(strlen(b)>=1)	//mytree에서 경로가 입력됬을 경우 해당 디렉토리로 이동
					{
						mycd(b);
					}
					mypwd();
					tmp=1;
					mytree();	//mytree 실행
					cur_inum=save_cur_inum;	//원래 디렉토리로 돌아옴
				}
				else if(a[2]=='t' && a[3]=='o' && a[4]=='u' && a[5]=='c' && a[6]=='h')	//mytouch명령어
				{
					cut_path(b,route,file_name);	//b에 저장된 문자열을 경로와 파일이름으로 분리
					if(mycd(route)==1)	//route에 저장된 경로로 이동 후 file생성
					{
						mytouch(file_name);
						cur_inum=save_cur_inum;	//원래 경로로 돌아옴
					}
				}
				else if(a[2]=='s' && a[3]=='h' && a[4]=='o' && a[5]=='w' && a[6]=='i' && a[7]=='n' && a[8]=='o' && a[9]=='d' && a[10]=='e')	//myshowinode명령어
				{
					myshowinode(atoi(b)-1);
				}
				else if(a[2]=='c' && a[3]=='p' && a[4]=='f' && a[5]=='r' && a[6]=='o' && a[7]=='m')	//mycpfrom명령어
				{
					divide(b,name1,name2);	//b를 내용을 읽을 파일과 내용을 저장할 파일로 나눔
					mycpfrom(name1, name2);
				}
				else if(a[2]=='s' && a[3]=='h' && a[4]=='o' && a[5]=='w' && a[6]=='b' && a[7]=='l' && a[8]=='o' && a[9]=='c' && a[10]=='k')	//myshowblock명령어
				{
					myshowblock(atoi(b)-1);
				}
				else if(a[2]=='c' && a[3]=='a' && a[4]=='t')	//mycat명령어
				{
					for(int i=0; i<strlen(b); ++i)
					{
						if(b[i]=='>')
						{
							sw=1;
						}
					}
					if(sw==0)	//mycat을 이용하여 파일의 내용을 출력할 경우
					{
						cut_path(b,route,file_name);	//b를 경로와 파일명으로 분리
						if(mycd(route)==1)	//route에 저장된 경로로 이동 후 mycat실행
						{
							mycat(file_name);
							cur_inum=save_cur_inum;	//원래 경로로 돌아옴
						}
					}
					else if(sw==1)	//mycat을 이용하여 파일을 만들 경우
					{
						mycat(b);
					}
				}
				else if(a[2] == 'c' && a[3] == 'p' && a[4] == 't' && a[5] == 'o')	//mycpto명령어
				{
					divide(b,name1,name2);	//b를 내용을 읽을 파일과 내용을 저장할 파일로 나눔
					mycpto(name1, name2);
				}
				else if(a[2]=='s' && a[3]=='h' && a[4]=='o' && a[5]=='w' && a[6]=='f' && a[7]=='i' && a[8]=='l' && a[9]=='e')	//myshowfile명령어
				{
					for(int i=0; i<strlen(b); ++i)	//출력을 시작할 바이트를 저장
					{
						if(b[i]==' ')
						{
							tmp=i;
							for(int j=0; j<i; ++j)
							{
								tmp_char[j]=b[j];
							}
							tmp_char[i]='\0';
							start=atoi(tmp_char);
							break;
						}
					}
					memset(tmp_char, 0, 10);
					for(int i=tmp+1; i<strlen(b); ++i)	//출력을 끝낼 바이트를 저장
					{
						if(b[i]==' ')
						{
							for(int j=tmp+1; j<i; ++j)
							{
								tmp_char[j-tmp-1]=b[j];
							}
							tmp_char[i-tmp-1]='\0';
							end = atoi(tmp_char);
							tmp=i;
							break;
						}
					}
					for(int i=0; i<=tmp; ++i)	//출력할 파일의 이름 저장
					{
						for(int j=0; j<=strlen(b); ++j)
						{
							b[j]=b[j+1];
						}
					}
					cut_path(b, route, file_name);	//b를 경로와 파일명으로 분리
					if(mycd(route)==1)	//route에 저장된 경로로 이동후 myshowfile명령어 실행
					{
						myshowfile(start, end, file_name);
						cur_inum = save_cur_inum;	//원래 경로로 돌아옴
					}
				}
				else if(a[2]=='c' && a[3]=='p')	//mycp명령어
				{
					divide(b, name1, name2);	//b를 원래 파일명, 새로 만들 파일명으로 분리
					mycp(name1, name2);
				}
				else if(a[2]=='r' && a[3]=='m')	//myrm명령어
				{
					cut_path(b,route,file_name);	//b를 경로와 파일명으로 분리
					if(mycd(route)==1)	//route에 저장된 경로로 이동 후 myrm 실행
					{
						myrm(file_name);
						cur_inum=save_cur_inum;	//원래 경로로 돌아옴
					}
				}
				else if(a[2]=='m' && a[3]=='v')	//mymv명령어
				{
					divide(b, name1, name2);	//b를 원래 파일명과 변경할 파일명(또는 이동할 directory이름)으로 구분
					mymv(name1, name2);
				}
			}
			else	//command명령어
			{
				p=a;
				system(p);
			}
		}
		fclose(ifp);
		ifp=fopen("myfs","rb");
		if(ifp!=NULL)	//myfs파일이 존재할 때 여태까지 작업한 내용 저장
		{
			fclose(ifp);
			ofp=fopen("myfs","wb");
			fwrite(&mfs, sizeof(struct my_file_system), 1, ofp);
			fwrite(&cnt_file, sizeof(int), 1, ofp);	
			fwrite(&tree, sizeof(struct directory_tree), 512, ofp);
			fwrite(&data_state, sizeof(char), 1024, ofp);
			fclose(ofp);
		}
		else
		{
			fclose(ifp);
		}
	}
}
void path(int a)	//경로를 출력하는 함수
{
	char route[100][5];
	int cnt=0;
	if(a==0)
	{
		printf("[/ ]$ ");
		return;
	}
	while(tree[a].next != NULL)	//상위 디렉토리가 존재하지 않을때까지 반복
	{
		strcpy(route[cnt],tree[a].name);	//route배열에 현재 디렉토리부터 루트 디렉토리까지 경로를 차례대로 저장
		++cnt;
		a=tree[a].next->inum;
	}
	printf("[");
	for(int i=cnt-1; i>=0; --i)	//route배열에 있는 문자열을 마지막부터 첫번째 배열까지 차례대로 출력
	{
		printf("/%s",route[i]);
	}
	printf("]$ ");
}
void cut_path(char name[100], char route[100], char file_name[5])	//name문자열을 경로(route)와 파일명(file_name)으로 분리하는 함수
{
	int sw=0;
	strcpy(route,".");	//route변수의 기본값을 현재디렉토리로 지정
	if(name[0]=='/')	//절대경로로 입력되었을 경우의 분리방법
	{
		for(int i=1; i<strlen(name); ++i)
		{
			if(name[i]=='/')
			{
				sw=1;
				break;
			}
		}
		if(sw==0)
		{
			strcpy(route,"/");
			for(int i=1; i<strlen(name); ++i)
			{
				file_name[i-1]=name[i];
			}
			file_name[4]='\0';
			return;
		}
	}
		
	if(strlen(name)==1)
	{
		strcpy(file_name,name);
		return;
	}
	for(int i=0; i<strlen(name); ++i)
	{
		if(name[i]=='/')
			sw=1;
	}
	if(sw==0)	//경로가 입력되지 않았을 때, file_name에 name을 그대로 저장한 후 리턴함
	{
		strcpy(file_name,name);
		strcpy(route, ".");
		file_name[4]='\0';
		return;
	}
	for(int i=strlen(name)-1; i>=0; --i)	//상대경로로 입력되었을 경우 분리방법
	{
		if(name[i]=='/')
		{
			for(int j=0; j<i; ++j)
			{
				route[j]=name[j];
			}
			route[i]='\0';
			for(int j=i+1; j<=strlen(name); ++j)
			{
				file_name[j-i-1]=name[j];
			}
			break;
		}
	}
	route[strlen(route)]='\0';
	if(route[0]=='\0')
		strcpy(route,".");
	file_name[4]='\0';
	return;
}
void myls(char ls[100])	//myls명령어
{
	char (*name)[5];
	char tmp_name[5];
	char route[100];
	int save_cur_inum=cur_inum;
	int tmp_inum;
	int cnt=0;
	int *inum;
	int n=tree[cur_inum].cnt_file;
	int tmp;
	inum=(int *) calloc(n, sizeof(int));	//현재 directory가 가지고 있는 파일의 개수만큼 inum변수의 동적메모리 할당
	name=(char (*)[5])calloc(n * 5, sizeof(char));	//현재 directory가 가지고 있는 파일의 개수만큼 name변수의 동적메모리 할당
	strcpy(route,".");
	if(strlen(a) != 4 && ls[0]=='-')	//myls의 옵션이 주어졌을 경우 ls배열에 옵션을 저장, route에 myls명령어를 실행할 경로를 저장
	{
		for(int i=0; i<=strlen(ls); ++i)
		{
			if(ls[i]==' ' && ls[i+1]!='-')
			{
				for(int j=i+1; j<=strlen(ls); ++j)
				{
					if(ls[j]==' ')
						break;
					route[j-i-1]=ls[j];
				}
				for(int j=0; j<=strlen(route); ++j)
					for(int k=i; k<strlen(ls); ++k)
						ls[k]=ls[k+1];
			}
		}
	}
	else if(strlen(a) != 4 && ls[0]!='-')
	{
		for(int i=0; i<=strlen(ls); ++i)
		{
			route[i]=ls[i];
		}
		for(int i=0; i<=strlen(route); ++i)
			for(int j=0; j<strlen(ls); ++j)
				ls[j]=ls[j+1];
	}
	mycd(route);	//route에 저장된 경로로 이동
	if(n==0)	//현재 경로에 저장된 파일이 없을경우 원래 경로로 돌아간 후 함수를 끝냄
	{
		cur_inum=save_cur_inum;
		return;
	}
	for(int i=1; i<512; ++i)	//tree구조체의 자기참조구조체인 next가 현재 디렉토리를 가리키고 있을 경우, name배열에 저장
	{
		if(tree[i].next != NULL)
		{
			if((tree[i].next->inum)==cur_inum)
			{
				strcpy(name[cnt],tree[i].name);
				inum[cnt]=tree[i].inum;
				++cnt;
			}
		}
		if(cnt==n)
			break;
	}
	if(n>1)
	{
		for(int i=n-1; i>=0; --i)	//name배열에 저장된 것들을 오름차순으로 정렬
		{
			for(int j=i-1; j>=0; --j)
			{
				if(strcmp(name[i],name[j])<0)
				{
					strcpy(tmp_name,name[i]);
					strcpy(name[i],name[j]);
					strcpy(name[j],tmp_name);
					tmp_inum=inum[i];
					inum[i]=inum[j];
					inum[j]=tmp_inum;
				}
			}
		}
	}
	if((ls[0]=='-' && ls[1]=='i' && ls[2]=='l') || (ls[0]=='-' && ls[1]=='l' && ls[2]=='i') || (ls[0]=='-' && ls[1]=='i' && ls[3]=='-' && ls[4]=='l') || (ls[0]=='-' && ls[1]=='l' && ls[3]=='-' && ls[4]=='i'))	//myls의 옵션으로 -l과 -i가 주어졌을 경우
	{
		for(int i=0; i<n; ++i)
		{
			if(mfs.inode[inum[i]].dir_or_file == Dir)
			{
				if(mfs.inode[inum[i]].size!=0)
				{
					tmp = mfs.inode[inum[i]].size + (8 - ((mfs.inode[inum[i]].size) % 8));
				}
				else
				{
					tmp = 0;
				}
				printf("%3d %c %4d %04d/%02d/%02d %02d:%02d:%02d %s\n",inum[i]+1, mfs.inode[inum[i]].dir_or_file, tmp/8, mfs.inode[inum[i]].year, mfs.inode[inum[i]].mon, mfs.inode[inum[i]].day, mfs.inode[inum[i]].hour, mfs.inode[inum[i]].min, mfs.inode[inum[i]].sec, name[i]);
			}
			else
			{
				printf("%3d %c %4d %04d/%02d/%02d %02d:%02d:%02d %s\n",inum[i]+1, mfs.inode[inum[i]].dir_or_file, mfs.inode[inum[i]].size, mfs.inode[inum[i]].year, mfs.inode[inum[i]].mon, mfs.inode[inum[i]].day, mfs.inode[inum[i]].hour, mfs.inode[inum[i]].min, mfs.inode[inum[i]].sec, name[i]);
			}
		}
	}
	else if(ls[0]=='-' && ls[1]=='i')	//myls의 옵션으로 -i가 주어졌을 경우
	{
		for(int i=0; i<n; ++i)
		{
			printf("%3d %s\n",inum[i]+1, name[i]);
		}
	}
	else if(ls[0]=='-' && ls[1]=='l')	//myls의 옵션으로 -l이 주어졌을 경우
	{
		for(int i=0; i<n; ++i)
		{
			if(mfs.inode[inum[i]].dir_or_file == Dir)
			{
				if(mfs.inode[inum[i]].size!=0)
				{
					tmp = mfs.inode[inum[i]].size + (8-((mfs.inode[inum[i]].size)%8));
				}
				else
				{
					tmp=0;
				}
				printf("%c %4d %04d/%02d/%02d %02d:%02d:%02d %s\n",mfs.inode[inum[i]].dir_or_file, tmp/8, mfs.inode[inum[i]].year, mfs.inode[inum[i]].mon, mfs.inode[inum[i]].day, mfs.inode[inum[i]].hour, mfs.inode[inum[i]].min, mfs.inode[inum[i]].sec, name[i]);
			}
			else
			{
				printf("%c %4d %04d/%02d/%02d %02d:%02d:%02d %s\n",mfs.inode[inum[i]].dir_or_file, mfs.inode[inum[i]].size, mfs.inode[inum[i]].year, mfs.inode[inum[i]].mon, mfs.inode[inum[i]].day, mfs.inode[inum[i]].hour, mfs.inode[inum[i]].min, mfs.inode[inum[i]].sec, name[i]);
			}
		}
	}
	else	//옵션이 주어지지 않았을 경우
	{
		for(int i=0; i<n; ++i)
		{
			printf("%s\n",name[i]);
		}
	}
	free(name);
	free(inum);
	cur_inum=save_cur_inum;	//원래 경로로 되돌아옴
}
int exist(char file_name[5])	//file_name과 같은 이름의 파일이 이미 존재하는지 확인하는 함수
{
	int save_cur_inum=cur_inum;
	int cnt=0, t=0;
	for(int i=1; i<512; ++i)
	{
		if(strcmp(tree[i].name, file_name)==0 && tree[i].next->inum==cur_inum)
		{
			return 1;	//존재할 경우 1을 리턴
		}
	}
	return 0;	//존재하지 않을 경우 0을 리턴
}
void mymkdir(char file_name[5])	//mymkdir명령어
{
	int sw=0;
	int tmp_i=0, tmp_d=0;
	int cnt=0, t=0;
	if(exist(file_name)==1)	//이미 같은 이름의 파일이 있을경우 메시지를 출력후 함수를 종료함
	{
		printf("같은 이름의 파일이 있습니다.\n");
		return;
	}
	tmp_i=super_i();	//super block에서 사용하지 않는 inode 찾기
	mfs.super.inode[tmp_i]=1;
	++cnt_file;
	if(mfs.super.data[0]==0)	//루트 directory에 data block을 할당
	{
		mfs.inode[0].direct=0;
		data_state[0]=Dir;
		mfs.super.data[0]=1;
	}
	if(mfs.inode[cur_inum].direct==0 && cur_inum!=0)	//현재 directory에 direct block이 할당되어있지 않을경우 data block 할당
	{
		tmp_d=super_d();
		mfs.inode[cur_inum].direct=tmp_d;
		data_state[tmp_d]=Dir;
		mfs.super.data[tmp_d]=1;
	}
	if(tree[cur_inum].cnt_file < 24)	//현재 directory의 파일 개수가 24개 미만일 경우 direct block에 저장
	{
		cur_dnum=mfs.inode[cur_inum].direct;
	}
	else if(tree[cur_inum].cnt_file >= 24)	//현재 directory의 파일 개수가 24개 이상일 경우
	{
		if(mfs.inode[cur_inum].single_indirect==0)	//현재 directory의 single indirect block이 할당되어있지 않을 경우 data block 할당
		{
			tmp_d=super_d();
			mfs.inode[cur_inum].single_indirect=tmp_d;
			data_state[tmp_d]=Ind;
			mfs.super.data[tmp_d]=1;
		}
		if(mfs.data[mfs.inode[cur_inum].direct].directory.cnt_file < 24)	//direct block이 가리키는 data block에서 빈공간이 생겼을 경우 정보를 저장할 data block을 변경
		{
			cur_dnum=mfs.inode[cur_inum].direct;
			sw=1;
		}
		if(sw==0)
		{
			for(int i=0; i<mfs.data[mfs.inode[cur_inum].single_indirect].indirect.cnt; ++i)	//여태까지 할당한 data block에 빈공간이 생겼을 경우 정보를 저장할 data block을 변경
			{
				if(mfs.data[mfs.data[mfs.inode[cur_inum].single_indirect].indirect.num[i]].directory.cnt_file < 24)
				{
					cur_dnum=mfs.data[mfs.inode[cur_inum].single_indirect].indirect.num[i];
					sw=1;
					break;
				}
			}
		}
		if(sw==0)	//모든 data block에 공간이 없을 경우 새로운 data block 할당
		{
			tmp_d=super_d();
			mfs.data[mfs.inode[cur_inum].single_indirect].indirect.num[mfs.data[mfs.inode[cur_inum].single_indirect].indirect.cnt]=tmp_d;
			data_state[tmp_d]=Dir;
			++mfs.data[mfs.inode[cur_inum].single_indirect].indirect.cnt;
			mfs.super.data[tmp_d]=1;
			cur_dnum=tmp_d;
		}
	}
	strcpy(tree[tmp_i].name, file_name);	//tree에 이름 입력
	mfs.data[cur_dnum].directory.d_inum[mfs.data[cur_dnum].directory.cnt_file]=tmp_i;	//data block에 inode번호 저장
	strcpy(mfs.data[cur_dnum].directory.d_name[mfs.data[cur_dnum].directory.cnt_file], file_name);	//data block에 이름 저장
	++mfs.data[cur_dnum].directory.cnt_file;	//현재 directory가 가지고 있는 파일의 개수 +1
	mfs.inode[tmp_i].dir_or_file=Dir;	//inode에 파일 종류를 directory라고 입력
	tree[tmp_i].next = &tree[cur_inum];	//tree에 새로만든 directory의 next포인터에 상위 directory 입력
	++tree[cur_inum].cnt_file;	//현재 directory의 파일 갯수 +1
	tree[tmp_i].inum=tmp_i;	//방금 만든 directory의 inode번호 저장
	inode_time(tmp_i);	//directory 생성 시간 설정
	mfs.inode[cur_inum].size+=42;	//현재 directory의 크기를 42비트 더함(이름 4글자 : 4바이트 = 32비트, inode번호 : 1024=2^10이므로 10비트)
}
int super_i()	//사용중이 아닌 inode번호 찾기
{
	for(int i=0; i<512; ++i)
	{
		if(mfs.super.inode[i]==0)
		{
			return i;
		}
	}
	printf("사용 가능한 inode가 없습니다\n");
	return 0;
}
int super_d()	//사용중이 아닌 data block찾기
{
	for(int i=0; i<1024; ++i)
	{
		if(mfs.super.data[i]==0)
		{
			return i;
		}
	}
	printf("사용 가능한 data block이 없습니다\n");
	return 0;
}
void inode_time(int a)	//현재 시간을 저장하는 함수
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
int mycd(char cd[100])	//mycd명령어
{
	if(strcmp(a,"mycd")==0)	//mycd만 입력했을경우 홈 디렉토리로 돌아감
	{
		cur_inum=0;
		return 1;
	}
	char route[100][5];
	int cnt=0,tmp=0;
	int save_cur_inum=cur_inum;
	for(int i=0; i<=strlen(cd); ++i, ++tmp)	//mycd뒤의 문자열을 '/'를 기준으로 잘라 route배열에 저장함(ex : a/b/c를 route[0]="a", route[1]="b", route[2]="c"로 분리)
	{
		if(cd[0]=='/' && i==0)
		{
			strcpy(route[0],"/");
			if(cd[1]=='\0')
				break;
			++cnt;
			tmp=-1;
		}
		else
		{
			if(cd[i]!='/')
				route[cnt][tmp]=cd[i];
			else
			{
				route[cnt][tmp]='\0';
				tmp=-1;
				++cnt;
			}
		}
	}
	for(int i=0; i<=cnt; ++i)	//route배열에 입력된 directory로 순서대로 이동함
	{
		for(int j=1; j<512; ++j)
		{
			if(strcmp(route[i],"..")==0)
			{
				cur_inum=tree[cur_inum].next->inum;
				break;
			}
			else if(strcmp(route[i],".")==0)
				break;
			else if(strcmp(route[i],"/")==0 || strcmp(route[i],"~")==0)
			{
				cur_inum=0;
				break;
			}
			else if(strcmp(tree[j].name, route[i])==0 && strcmp(tree[j].next->name, tree[cur_inum].name)==0 && mfs.inode[j].dir_or_file == Dir)
			{
				cur_inum=j;
				break;
			}
			if(j==511)	//디렉토리를 찾지 못하면 오류메시지를 출력 후 원래 경로로 돌아감
			{
				printf("해당 디렉토리를 찾을 수 없습니다.\n");
				cur_inum=save_cur_inum;
				return 0;
			}
		}
	}
	return 1;
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
void mystate()	//mystate명령어
{
	int free_inode=0;
	int free_data=0;
	for(int i=0; i<512; ++i)
	{
		if(mfs.super.inode[i]==0)
			++free_inode;
	}
	for(int i=0; i<1024; ++i)
	{
		if(mfs.super.data[i]==0)
			++free_data;
	}
	printf("free inode : %d\nfree data block : %d\n",free_inode, free_data);
}
void mypwd()	//mypwd명령어
{
	char route[100][5];
	int cnt=0;
	int a=cur_inum;
	if(a==0)	//현재 디렉토리가 홈 디렉토리일 경우 /를 출력하고 끝냄
	{
		printf("/\n");
		return;
	}
	while(tree[a].next != NULL)	//상위 디렉토리가 존재하지 않을 때까지 반복
	{
		strcpy(route[cnt],tree[a].name);	//상위 디렉토리의 이름을 route배열에 저장함
		++cnt;
		a=tree[a].next->inum;	//위치를 상위 디렉토리로 옮김
	}
	for(int i=cnt-1; i>=0; --i)	//저장한 route 배열을 거꾸로 출력함
	{
		printf("/%s",route[i]);
	}
	printf("\n");
}
void mytree()	//mytree명령어
{
	int save_cur_inum=save_cur_inum;
	int cnt=0;
	if(tree[cur_inum].cnt_file == 0)
	{
		--tmp;
		if(cur_inum != 0)
			mycd("..");
		return;
	}
	for(int i=1; i<512; ++i)
	{
		if(tree[i].next != NULL)
		{
			if(tree[i].next->inum == cur_inum)	//하위 디렉토리를 찾은 후
			{
				++cnt;
				for(int j=1; j<=tmp*3; ++j)	//현재 위치에 따라서 '-'과 '*'를 출력하여 파일의 위치를 표시해줌
				{
					if(j==tmp*3)
					{
						printf("*");
						break;
					}
					printf("-");
				}
				printf(" %s\n",tree[i].name);	//발견한 파일명을 출력
				if(mfs.inode[i].dir_or_file == Dir)	//발견한 파일의 종류가 directory일 경우 해당 디렉토리로 이동 후 mytree를 실행 
				{
					++tmp;
					mycd(tree[i].name);
					mytree(tmp);
				}
			}
		}
		if(cnt==tree[cur_inum].cnt_file)	//현재 directory가 가지고 있는 파일을 모두 찾았을 경우 상위 디렉토리로 돌아옴
		{
			--tmp;
			if(cur_inum != 0)
				mycd("..");
			return;
		}
	}
	return;
}
				break;
			}
		}	
	}
	return;
}
void mytree()	//mytree명령어
{
	int save_cur_inum=save_cur_inum;
	int cnt=0;
	if(tree[cur_inum].cnt_file == 0)
	{
		--tmp;
		if(cur_inum != 0)
			mycd("..");
		return;
	}
	for(int i=1; i<512; ++i)
	{
		if(tree[i].next != NULL)
		{
			if(tree[i].next->inum == cur_inum)	//하위 디렉토리를 찾은 후
			{
				++cnt;
				for(int j=1; j<=tmp*3; ++j)	//현재 위치에 따라서 '-'과 '*'를 출력하여 파일의 위치를 표시해줌
				{
					if(j==tmp*3)
					{
						printf("*");
						break;
					}
					printf("-");
				}
				printf(" %s\n",tree[i].name);	//발견한 파일명을 출력
				if(mfs.inode[i].dir_or_file == Dir)	//발견한 파일의 종류가 directory일 경우 해당 디렉토리로 이동 후 mytree를 실행 
				{
					++tmp;
					mycd(tree[i].name);
					mytree(tmp);
				}
			}
		}
		if(cnt==tree[cur_inum].cnt_file)	//현재 directory가 가지고 있는 파일을 모두 찾았을 경우 상위 디렉토리로 돌아옴
		{
			--tmp;
			if(cur_inum != 0)
				mycd("..");
			return;
		}
	}
	return;
}
void myshowinode(int i)	//myshowinode명령어
{
	int tmp=mfs.inode[i].size;
	if(mfs.inode[i].dir_or_file == Dir)
	{
		if(tmp % 8 != 0)
			tmp+= (8 - (tmp % 8));
		printf("file type : directory\n");
		printf("file size : %d byte\n",tmp / 8);
	}
	else if(mfs.inode[i].dir_or_file == File)
	{
		printf("file type : regular file\n");
		printf("file size : %d byte\n",mfs.inode[i].size);
	}
	printf("modified time : %d/%02d/%02d %02d:%02d:%02d\n",mfs.inode[i].year, mfs.inode[i].mon, mfs.inode[i].day, mfs.inode[i].hour, mfs.inode[i].min, mfs.inode[i].sec);
	printf("data block list : ");
	if(mfs.inode[i].size != 0)	//data block들을 출력
	{
		if(mfs.inode[i].direct != 0 || i==0)
		{
			printf("%d",mfs.inode[i].direct+1);
			if(mfs.inode[i].single_indirect != 0)
			{
				for(int j=0; j<mfs.data[mfs.inode[i].single_indirect].indirect.cnt; ++j)
				{
					printf(", %d",mfs.data[mfs.inode[i].single_indirect].indirect.num[j]+1);
				}
				if(mfs.inode[i].double_indirect != 0)
				{
					for(int j=0; j<mfs.data[mfs.inode[i].double_indirect].indirect.cnt; ++j)
					{
						for(int k=0; k<mfs.data[mfs.data[mfs.inode[i].double_indirect].indirect.num[j]].indirect.cnt; ++k)
						{
							printf(", %d",mfs.data[mfs.data[mfs.inode[i].double_indirect].indirect.num[j]].indirect.num[k]+1);
						}
					}
				}
			}
		}
	}
	printf("\n");
	return;
}
void myshowblock(int i)	//myshowblock명령어
{
	if(data_state[i]==Dir)	//data block의 종류가 directory일 경우
	{
		for(int j=0; j<mfs.data[i].directory.cnt_file; ++j)
		{
			printf("%s %d\n",mfs.data[i].directory.d_name[j], mfs.data[i].directory.d_inum[j]);
		}
	}
	else if(data_state[i]==Ind)	//data block의 종류가 indirect block일 경우
	{
		for(int j=0; j<mfs.data[i].indirect.cnt; ++j)
		{
			printf("%d\n",mfs.data[i].indirect.num[j]+1);
		}
	}
	else if(data_state[i]==File)	//data block의 종류가 file일 경우
	{
		printf("%s",mfs.data[i].file.data);
	}
	printf("\n");
	return;
}
void mystate()	//mystate명령어
{
	int free_inode=0;
	int free_data=0;
	for(int i=0; i<512; ++i)
	{
		if(mfs.super.inode[i]==0)
			++free_inode;
	}
	for(int i=0; i<1024; ++i)
	{
		if(mfs.super.data[i]==0)
			++free_data;
	}
	printf("free inode : %d\nfree data block : %d\n",free_inode, free_data);
}
void mycat(char file_name[100])	//mycat명령어
{
	int tmp_i, tmp_d, k;
	int cur_dnum;
	int sw=0;
	int cnt=0, cnt2=0, len=0;
	int tmp=0;
	char s[131072];
	char cut_s[1024][129];
	char before_file[100][100];
	char after_file[100];
	memset(s,0,131072);
	memset(cut_s,0,1024*129);
	for(int i=0; i<strlen(file_name); ++i)
	{
		if(file_name[i] == '>')
		{
			sw=1;
			break;
		}
	}
	if(sw==0)	//mycat을 이용하여 파일의 내용을 출력할 경우
	{

		for(int i=1; i<512; ++i)	//입력한 파일을 찾음
		{
			if(strcmp(tree[i].name, file_name)==0 && tree[i].next->inum == cur_inum && mfs.inode[tree[i].next->inum].dir_or_file == Dir)
			{
				tmp_i=i;
				break;
			}
		}
		cur_dnum=mfs.inode[tmp_i].direct;
		if(tree[tmp_i].next != NULL && mfs.inode[tmp_i].dir_or_file == Dir)	//찾은 파일의 종류가 directory일 경우 오류메시지 출력
		{
			printf("%s는 파일이 아닙니다\n",file_name);
			return;
		}
		else if(tree[tmp_i].next == NULL)	//파일을 찾지 못했을경우 오류메시지 출력
		{
			printf("%s라는 파일이 존재하지 않습니다\n",file_name);
			return;
		}
		if(mfs.inode[tmp_i].size != 0)
		{

			while(1)	//파일의 내용을 출력함
			{
				printf("%s",mfs.data[cur_dnum].file.data);
				if(mfs.data[cur_dnum].file.next == NULL)
				{
					break;
				}
				cur_dnum = mfs.data[cur_dnum].file.next->file.dnum;
			}
		}
	}
	else if(sw==1)
	{
		for(int i=0; i<strlen(file_name); ++i)	//'>'전에 입력된 파일의 갯수만큼 before_file변수에 파일명을 저장함
		{
			if(file_name[i] == ' ')
			{
				for(int j=tmp; j<i; ++j)
				{
					before_file[cnt][j-tmp]=file_name[j];
				}
				before_file[cnt][5]='\0';	
				++cnt;
				tmp = i+1;
			}
			else if(file_name[i] == '>')
			{
				for(int j=i+2; j<strlen(file_name); ++j)
				{
					after_file[j-i-2]=file_name[j];
				}
				after_file[5]='\0';
				break;
			}
		}
		mytouch(after_file);	//새로운 파일을 만듬
		for(int i=0; i<cnt; ++i)	//before_file에 저장되어 있는 파일들의 내용을 s변수에 이어서 저장함(이후 mycpfrom함수와 유사함)
		{
			cnt2=0;
			for(int j=1; j<512; ++j)
			{
				if(strcmp(tree[j].name, before_file[i]) == 0 && tree[j].next->inum == cur_inum)
				{
					tmp_i = j;
					break;
				}
			}
			cur_dnum = mfs.inode[tmp_i].direct;
			while(1)
			{
				if(mfs.data[cur_dnum].file.data[cnt2] == '\0')
					break;
				s[len]=mfs.data[cur_dnum].file.data[cnt2];
				++cnt2;
				++len;
				if(cnt2 == 128)
				{
					cnt2=0;
					cur_dnum = mfs.data[cur_dnum].file.next->file.dnum;
				}
			}
		}
		s[len]='\0';
		for(int i=1; i<512; ++i)
		{
			if(strcmp(tree[i].name, after_file) == 0 && tree[i].next->inum == cur_inum)
			{
				tmp_i=i;
				break;
			}
		}
		mfs.inode[tmp_i].size = len;
		sw=0;
		cnt2=0;
		cnt=0;
		for(int i=0; i<len; ++i)
		{
			if(cnt==128)
			{
				cut_s[cnt2][128]='\0';
				cnt=0;
				++cnt2;
			}
			cut_s[cnt2][cnt]=s[i];
			++cnt;
		}
		cnt=0;
		cnt2++;
		if(cnt2>=1)
		{
			tmp_d=super_d();
			data_state[tmp_d] = File;
			mfs.data[tmp_d].file.dnum = tmp_d;
			mfs.inode[tmp_i].direct = tmp_d;		//direct block에 번호 할당
			strcpy(mfs.data[tmp_d].file.data, cut_s[cnt]);		//direct block이 가리키는 data block에 정보 저장
			++cnt;
			cur_dnum=tmp_d;
			mfs.super.data[tmp_d]=1;
	
			if(cnt2>=2)
			{
				tmp_d=super_d();
				data_state[tmp_d]=Ind;
				mfs.inode[tmp_i].single_indirect=tmp_d;		//single indirect block에 번호 할당
				mfs.super.data[tmp_d]=1;
				for(int i=0; i<102; ++i)		//single indirect block이 가리키는 data block에 data block 번호 102개 저장
				{
					tmp_d=super_d();
					data_state[tmp_d]=File;
					mfs.data[tmp_d].file.dnum = tmp_d;
					mfs.data[mfs.inode[tmp_i].single_indirect].indirect.num[mfs.data[mfs.inode[tmp_i].single_indirect].indirect.cnt] = tmp_d;
					++mfs.data[mfs.inode[tmp_i].single_indirect].indirect.cnt;
					mfs.data[cur_dnum].file.next = &mfs.data[tmp_d];		//이전 data block과 현재 data block을 연결
					mfs.super.data[tmp_d]=1;
					strcpy(mfs.data[tmp_d].file.data, cut_s[cnt]);
					cur_dnum = tmp_d;
					++cnt;
					if(cnt == cnt2)
						break;
				}
				if(cnt2>=104)
				{
					tmp_d=super_d();
					data_state[tmp_d]=Ind;
					mfs.inode[tmp_i].double_indirect=tmp_d;		//double indirect block에 번호 할당
					mfs.super.data[tmp_d]=1;
					sw=0;
					for(int i=0; i<102; ++i)	//double indirect block이 가리키는 data block에 single indirect block들을 할당
					{
						tmp_d=super_d();
						data_state[tmp_d]=Ind;
						mfs.data[mfs.inode[tmp_i].double_indirect].indirect.num[i]=tmp_d;
						k=tmp_d;
						++mfs.data[mfs.inode[tmp_i].double_indirect].indirect.cnt;
						mfs.super.data[tmp_d]=1;
						for(int j=0; j<102; ++j)	//data block에 내용 저장
						{
							tmp_d=super_d();
							data_state[tmp_d]=File;
							mfs.data[tmp_d].file.dnum = tmp_d;
							mfs.data[k].indirect.num[j]=tmp_d;
							++mfs.data[k].indirect.cnt;
							mfs.super.data[tmp_d]=1;
							mfs.data[cur_dnum].file.next = &mfs.data[tmp_d];
							strcpy(mfs.data[tmp_d].file.data, cut_s[cnt]);
							cur_dnum = tmp_d;
							++cnt;
							if(cnt==cnt2)
							{
								sw=1;
								break;
							}
						}
						if(sw==1)
							break;
					}
				}
			}
		}
	}
	return;
}
int exist(char file_name[5])	//file_name과 같은 이름의 파일이 이미 존재하는지 확인하는 함수
{
	int save_cur_inum=cur_inum;
	int cnt=0, t=0;
	for(int i=1; i<512; ++i)
	{
		if(strcmp(tree[i].name, file_name)==0 && tree[i].next->inum==cur_inum)
		{
			return 1;	//존재할 경우 1을 리턴
		}
	}
	return 0;	//존재하지 않을 경우 0을 리턴
}
void mymkdir(char file_name[5])	//mymkdir명령어
{
	int sw=0;
	int tmp_i=0, tmp_d=0;
	int cnt=0, t=0;
	if(exist(file_name)==1)	//이미 같은 이름의 파일이 있을경우 메시지를 출력후 함수를 종료함
	{
		printf("같은 이름의 파일이 있습니다.\n");
		return;
	}
	tmp_i=super_i();	//super block에서 사용하지 않는 inode 찾기
	mfs.super.inode[tmp_i]=1;
	++cnt_file;
	if(mfs.super.data[0]==0)	//루트 directory에 data block을 할당
	{
		mfs.inode[0].direct=0;
		data_state[0]=Dir;
		mfs.super.data[0]=1;
	}
	if(mfs.inode[cur_inum].direct==0 && cur_inum!=0)	//현재 directory에 direct block이 할당되어있지 않을경우 data block 할당
	{
		tmp_d=super_d();
		mfs.inode[cur_inum].direct=tmp_d;
		data_state[tmp_d]=Dir;
		mfs.super.data[tmp_d]=1;
	}
	if(tree[cur_inum].cnt_file < 24)	//현재 directory의 파일 개수가 24개 미만일 경우 direct block에 저장
	{
		cur_dnum=mfs.inode[cur_inum].direct;
	}
	else if(tree[cur_inum].cnt_file >= 24)	//현재 directory의 파일 개수가 24개 이상일 경우
	{
		if(mfs.inode[cur_inum].single_indirect==0)	//현재 directory의 single indirect block이 할당되어있지 않을 경우 data block 할당
		{
			tmp_d=super_d();
			mfs.inode[cur_inum].single_indirect=tmp_d;
			data_state[tmp_d]=Ind;
			mfs.super.data[tmp_d]=1;
		}
		if(mfs.data[mfs.inode[cur_inum].direct].directory.cnt_file < 24)	//direct block이 가리키는 data block에서 빈공간이 생겼을 경우 정보를 저장할 data block을 변경
		{
			cur_dnum=mfs.inode[cur_inum].direct;
			sw=1;
		}
		if(sw==0)
		{
			for(int i=0; i<mfs.data[mfs.inode[cur_inum].single_indirect].indirect.cnt; ++i)	//여태까지 할당한 data block에 빈공간이 생겼을 경우 정보를 저장할 data block을 변경
			{
				if(mfs.data[mfs.data[mfs.inode[cur_inum].single_indirect].indirect.num[i]].directory.cnt_file < 24)
				{
					cur_dnum=mfs.data[mfs.inode[cur_inum].single_indirect].indirect.num[i];
					sw=1;
					break;
				}
			}
		}
		if(sw==0)	//모든 data block에 공간이 없을 경우 새로운 data block 할당
		{
			tmp_d=super_d();
			mfs.data[mfs.inode[cur_inum].single_indirect].indirect.num[mfs.data[mfs.inode[cur_inum].single_indirect].indirect.cnt]=tmp_d;
			data_state[tmp_d]=Dir;
			++mfs.data[mfs.inode[cur_inum].single_indirect].indirect.cnt;
			mfs.super.data[tmp_d]=1;
			cur_dnum=tmp_d;
		}
	}
	strcpy(tree[tmp_i].name, file_name);	//tree에 이름 입력
	mfs.data[cur_dnum].directory.d_inum[mfs.data[cur_dnum].directory.cnt_file]=tmp_i;	//data block에 inode번호 저장
	strcpy(mfs.data[cur_dnum].directory.d_name[mfs.data[cur_dnum].directory.cnt_file], file_name);	//data block에 이름 저장
	++mfs.data[cur_dnum].directory.cnt_file;	//현재 directory가 가지고 있는 파일의 개수 +1
	mfs.inode[tmp_i].dir_or_file=Dir;	//inode에 파일 종류를 directory라고 입력
	tree[tmp_i].next = &tree[cur_inum];	//tree에 새로만든 directory의 next포인터에 상위 directory 입력
	++tree[cur_inum].cnt_file;	//현재 directory의 파일 갯수 +1
	tree[tmp_i].inum=tmp_i;	//방금 만든 directory의 inode번호 저장
	inode_time(tmp_i);	//directory 생성 시간 설정
	mfs.inode[cur_inum].size+=42;	//현재 directory의 크기를 42비트 더함(이름 4글자 : 4바이트 = 32비트, inode번호 : 1024=2^10이므로 10비트)
}
int super_i()	//사용중이 아닌 inode번호 찾기
{
	for(int i=0; i<512; ++i)
	{
		if(mfs.super.inode[i]==0)
		{
			return i;
		}
	}
	printf("사용 가능한 inode가 없습니다\n");
	return 0;
}
int super_d()	//사용중이 아닌 data block찾기
{
	for(int i=0; i<1024; ++i)
	{
		if(mfs.super.data[i]==0)
		{
			return i;
		}
	}
	printf("사용 가능한 data block이 없습니다\n");
	return 0;
}
void inode_time(int a)	//현재 시간을 저장하는 함수
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
int mycd(char cd[100])	//mycd명령어
{
	if(strcmp(a,"mycd")==0)	//mycd만 입력했을경우 홈 디렉토리로 돌아감
	{
		cur_inum=0;
		return 1;
	}
	char route[100][5];
	int cnt=0,tmp=0;
	int save_cur_inum=cur_inum;
	for(int i=0; i<=strlen(cd); ++i, ++tmp)	//mycd뒤의 문자열을 '/'를 기준으로 잘라 route배열에 저장함(ex : a/b/c를 route[0]="a", route[1]="b", route[2]="c"로 분리)
	{
		if(cd[0]=='/' && i==0)
		{
			strcpy(route[0],"/");
			if(cd[1]=='\0')
				break;
			++cnt;
			tmp=-1;
		}
		else
		{
			if(cd[i]!='/')
				route[cnt][tmp]=cd[i];
			else
			{
				route[cnt][tmp]='\0';
				tmp=-1;
				++cnt;
			}
		}
	}
	for(int i=0; i<=cnt; ++i)	//route배열에 입력된 directory로 순서대로 이동함
	{
		for(int j=1; j<512; ++j)
		{
			if(strcmp(route[i],"..")==0)
			{
				cur_inum=tree[cur_inum].next->inum;
				break;
			}
			else if(strcmp(route[i],".")==0)
				break;
			else if(strcmp(route[i],"/")==0 || strcmp(route[i],"~")==0)
			{
				cur_inum=0;
				break;
			}
			else if(strcmp(tree[j].name, route[i])==0 && strcmp(tree[j].next->name, tree[cur_inum].name)==0 && mfs.inode[j].dir_or_file == Dir)
			{
				cur_inum=j;
				break;
			}
			if(j==511)	//디렉토리를 찾지 못하면 오류메시지를 출력 후 원래 경로로 돌아감
			{
				printf("해당 디렉토리를 찾을 수 없습니다.\n");
				cur_inum=save_cur_inum;
				return 0;
			}
		}
	}
	return 1;
}
void mycp(char name1[100], char name2[100])	//mycp명령어(name1 : 복사할 파일, name2 : 새로 만들 파일)
{
	int tmp_i, tmp_d, cur_dnum, sw=0;
	char cut_s[1024][129];
	int cnt=0, cnt2=0, k, len=0;
	mytouch(name2);	//name2파일을 새로 만듬
	for(int i=1; i<512; ++i)
	{
		if(strcmp(tree[i].name, name1)==0 && tree[i].next->inum == cur_inum)
		{
			tmp_i=i;
			break;
		}
	}
	cur_dnum = mfs.inode[tmp_i].direct;	//읽을 data block의 시작점을 name1의 첫번쨰 data block으로 저장
	for(int i=1; i<512; ++i)
	{
		if(strcmp(tree[i].name, name2)==0 && tree[i].next->inum == cur_inum)
		{
			tmp_i=i;
			break;
		}
	}
	while(1)	//복사할 파일의 data block들의 내용을 cut_s에 128바이트로 분리해서 저장(이후 mycpfrom함수와 동일)
	{
		strcpy(cut_s[cnt], mfs.data[cur_dnum].file.data);
		len+=strlen(cut_s[cnt]);
		++cnt;
		if(mfs.data[cur_dnum].file.next == NULL)
		{
			break;
		}
		cur_dnum = mfs.data[cur_dnum].file.next->file.dnum;
	}
	mfs.inode[tmp_i].size = len;
	cnt2=cnt;
	cnt=0;
	if(cnt2>=1)
	{
		tmp_d=super_d();
		data_state[tmp_d] = File;
		mfs.data[tmp_d].file.dnum = tmp_d;
		mfs.inode[tmp_i].direct = tmp_d;		//direct block에 번호 할당
		strcpy(mfs.data[tmp_d].file.data, cut_s[cnt]);		//direct block이 가리키는 data block에 정보 저장
		++cnt;
		cur_dnum=tmp_d;
		mfs.super.data[tmp_d]=1;

		if(cnt2>=2)
		{
			tmp_d=super_d();
			data_state[tmp_d]=Ind;
			mfs.inode[tmp_i].single_indirect=tmp_d;		//single indirect block에 번호 할당
			mfs.super.data[tmp_d]=1;
			for(int i=0; i<102; ++i)		//single indirect block이 가리키는 data block에 data block 번호 102개 저장
			{
				tmp_d=super_d();
				data_state[tmp_d]=File;
				mfs.data[tmp_d].file.dnum = tmp_d;
				mfs.data[mfs.inode[tmp_i].single_indirect].indirect.num[mfs.data[mfs.inode[tmp_i].single_indirect].indirect.cnt] = tmp_d;
				++mfs.data[mfs.inode[tmp_i].single_indirect].indirect.cnt;
				mfs.data[cur_dnum].file.next = &mfs.data[tmp_d];		//이전 data block과 현재 data block을 연결
				mfs.super.data[tmp_d]=1;
				strcpy(mfs.data[tmp_d].file.data, cut_s[cnt]);
				cur_dnum = tmp_d;
				++cnt;
				if(cnt == cnt2)
					break;
			}
			if(cnt2>=104)
			{
				tmp_d=super_d();
				data_state[tmp_d]=Ind;
				mfs.inode[tmp_i].double_indirect=tmp_d;		//double indirect block에 번호 할당
				mfs.super.data[tmp_d]=1;
				sw=0;
				for(int i=0; i<102; ++i)
				{
					tmp_d=super_d();
					data_state[tmp_d]=Ind;
					mfs.data[mfs.inode[tmp_i].double_indirect].indirect.num[i]=tmp_d;
					k=tmp_d;
					++mfs.data[mfs.inode[tmp_i].double_indirect].indirect.cnt;
					mfs.super.data[tmp_d]=1;
					for(int j=0; j<102; ++j)
					{
						tmp_d=super_d();
						data_state[tmp_d]=File;
						mfs.data[tmp_d].file.dnum = tmp_d;
						mfs.data[k].indirect.num[j]=tmp_d;
						++mfs.data[k].indirect.cnt;
						mfs.super.data[tmp_d]=1;
						mfs.data[cur_dnum].file.next = &mfs.data[tmp_d];
						strcpy(mfs.data[tmp_d].file.data, cut_s[cnt]);
						cur_dnum = tmp_d;
						++cnt;
						if(cnt==cnt2)
						{
							sw=1;
							break;
						}
					}
					if(sw==1)
						break;
				}
			}
		}
	}
	return;
}
