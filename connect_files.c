#include <stdio.h>
#include<sys/types.h>
#include<dirent.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define PRINT_ASSEMBLE_DEBUG

//const char *dir = "./files_698_print/";
const char *original_dir = "./files_single/";
#define MAX_FILE_NO (1024)
unsigned char files_exist_bit[MAX_FILE_NO/8];
int big_no;


int max(int a, int b)
{
    if(a>b) return a;
    return b;
}


#define SET_BIT_IN_ARRAY(arr, bitpos)       (arr[(bitpos) >> 3] |= 1 << ((bitpos) & 0x07))
#define CLR_BIT_IN_ARRAY(arr, bitpos)       (arr[(bitpos) >> 3] &= ~(1 << ((bitpos) & 0x07)))
#define IS_BIT_SET_IN_ARRAY(arr, bitpos)    (arr[(bitpos) >> 3] & (1 << ((bitpos) & 0x07)))


void del_old_file(char *sub_dir)
{
    unsigned char buf[0x80];

    sprintf(buf, "rm -fr %sconnect-*", sub_dir);
#ifdef PRINT_ASSEMBLE_DEBUG
    printf("%s\n", buf);
#endif
    system(buf);
}


void cp_connect_file(char *sub_dir)
{
    unsigned char buf[0x80];

    sprintf(buf, "cp %sconnect-* ./files_698_print/", sub_dir);
#ifdef PRINT_ASSEMBLE_DEBUG
    printf("%s\n", buf);
#endif
    system(buf);
}

static void check_one_original_dir(char *sub_dir)
{
    DIR *dirptr = NULL;  
    struct dirent *entry; 
    int no;

    memset(files_exist_bit, 0, sizeof(files_exist_bit));
    big_no = 0;
    if(NULL == (dirptr = opendir(sub_dir)))
    {  
        printf("open dir !\n");  
    }  
    else  
    {  
        del_old_file(sub_dir);
        while (entry = readdir(dirptr))  
        {  
            if((strstr(entry->d_name,".txt")) && (isdigit(entry->d_name[0])))
            {
                sscanf(entry->d_name,"%d.txt", &no);
                if(no > MAX_FILE_NO)
                {
                    printf("error, no(%d) is bigger than MAX_FILE_NO(%d)\n", no, MAX_FILE_NO);
                    break;
                }
                SET_BIT_IN_ARRAY(files_exist_bit, no-1);
                big_no = max(no, big_no);

            }
        }  
        closedir(dirptr);  
    }  
}





void assemble_copy_file_name(char *ori_name, int file_no, char *sub_dir, char *jzq_no, char *whole_name, char *target_name)
{
    char name[0x80];
    int begin, end;

    strcpy(whole_name, sub_dir);
    strcat(whole_name, ori_name);


    strcpy(target_name, sub_dir);
    strcat(target_name, "connect-");
    strcat(target_name, jzq_no);
    begin = ((file_no-1)/10)*10+1;
    end = ((file_no-1)/10)*10+10;
    if(big_no <= end)
    {
        sprintf(name, "(%d-end).txt", ((file_no-1)/10)*10+1);
    }
    else
    {
        sprintf(name, "(%d-%d).txt", begin, end);
    }
    strcat(target_name, name);

#ifdef PRINT_ASSEMBLE_DEBUG
    printf("whole name: %s\n", whole_name);
    printf("target name: %s\n", target_name);
#endif
}



static void copy_one_file(char *file_name, char *target_name)
{
    unsigned char buf[0x800];
    FILE *fps = NULL, *fpt = NULL;

    fps = fopen(file_name, "r");
    if(NULL == fps)
    {
        return;
    }


    fpt = fopen(target_name, "at+");
    if(NULL == fpt)
    {
        return;
    }

    while (1)
    {
        fgets(buf,sizeof(buf),fps);
        fputs(buf, fpt);
        if (feof(fps))
        {
            break;
        }
    }
    fclose(fps);
    fclose(fpt);
}

static void connect_one_file(char *sub_dir, char *jzq_no, int file_no)
{
    char file_name[0x100], target_name[0x100];
    char name[0x80];
    sprintf(name, "%d.txt", file_no);

    assemble_copy_file_name(name,file_no,sub_dir,jzq_no,file_name, target_name);
    copy_one_file(file_name,target_name);
}

static void connect_the_last_file(char *sub_dir, char *jzq_no)
{
    char file_name[0x100], target_name[0x100];

    assemble_copy_file_name("flh_sys_print.txt",big_no,sub_dir,jzq_no,file_name, target_name);
    copy_one_file(file_name,target_name);
}


static void connect_original_files(char *sub_dir, char *jzq_no)
{
    int i;

    for(i=0; i<big_no; i++)
    {
        if(IS_BIT_SET_IN_ARRAY(files_exist_bit, i))
        {
            connect_one_file(sub_dir, jzq_no, i+1);
        }
    }
    connect_the_last_file(sub_dir, jzq_no);

}


static void check_original_dirs(const char *dirs)
{
    DIR *dirptr = NULL;  
    struct dirent *entry; 
    char sub_dir[0x100];

    if(NULL == (dirptr = opendir(dirs)))
    {  
        printf("open dir !\n");  
    }  
    else  
    {  
        while (entry = readdir(dirptr))  
        {  
            if(strstr(entry->d_name,"jzq"))
            {
                sprintf(sub_dir, "%s%s", dirs, entry->d_name);
                strcat(sub_dir, "/");
                check_one_original_dir(sub_dir);
                connect_original_files(sub_dir, entry->d_name);
                cp_connect_file(sub_dir);
            }
        }  
        closedir(dirptr);  
    }  
}


int main (int argc, char *argv[])
{
    //unzip_file();

    check_original_dirs(original_dir);
    return(0);
}







