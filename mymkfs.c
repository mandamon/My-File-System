#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define Dir 'd'
typedef struct file_data_block{
	int dnum;
	char name[5];
	char data[129];
	union data_block *next;
} file;
typedef struct indirect_data_block{
	int cnt;
	int num[102];
} indirect;
typedef struct directory_data_block{
	int cnt_file;
	char d_name[24][5];
	int d_inum[24];
} directory;
union data_block{
	file file;
	indirect indirect;
	directory directory;
};
struct inode_list{
	char dir_or_file;
	int year, mon, day, hour, min, sec;
	int size;
	int direct;
	int single_indirect;
	int double_indirect;
};
struct super_block{
	_Bool inode[512];
	_Bool data[1024];
};
struct my_file_system{
	unsigned boot_block : 16;
	struct super_block super;
	struct inode_list inode[512];
	union data_block data[1024];
};
struct directory_tree{
	char name[5];
	int cnt_file;
	int inum;
	struct directory_tree *next;
};
struct my_file_system mfs;
struct directory_tree tree[512];
char data_state[1024];
int cnt_file=0;

int main()
{
	FILE *ofp;
	mfs.super.inode[0]=1;
	mfs.inode[0].dir_or_file=Dir;
	data_state[0]=Dir;
	tree[0].inum = 0;
	strcpy(tree[0].name, "/");
	ofp=fopen("myfs","wb");
	fwrite(&mfs, sizeof(struct my_file_system), 1, ofp);
	fwrite(&cnt_file, sizeof(int), 1, ofp);
	fwrite(&tree, sizeof(struct directory_tree), 512, ofp);
	fwrite(&data_state, sizeof(char), 1024, ofp);
	fclose(ofp);

}
