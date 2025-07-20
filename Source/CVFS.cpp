#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<io.h>

#define MAXINODE 50

#define READ 1
#define WRITE 2

#define MAXFILESIZE 1024

#define REGULAR 1
#define SPECIAL 2

#define START 0
#define CURRENT 1
#define END 2

typedef struct superblock
{
    int FreeInodes;
    int TotalInodes;
}SUPERBLOCK , *PSUPERBLOCK;


typedef struct inode
{
    char FileName[50];
    int InodeNumber;
    int FileSize;
    int FileActualSize;
    int FileType;
    char *Buffer;
    int LinkCount;
    int ReferenceCount;
    int Permission;
    struct inode *next;
} INODE , *PINODE , **PPINODE;


typedef struct  filetable
{
    int readoffset;
    int writeoffset;
    int Count;
    int mode;
    PINODE ptrinode;
}FILETABLE , *PFILETABLE;

typedef struct ufdt
{
    PFILETABLE ptrfiletable;
}UFDT;

UFDT UFDTArr[50];
SUPERBLOCK superblockobj;
PINODE head = NULL;

void InitializeSuperblock()
{
    int i = 0;

    while(i < MAXINODE)
    {
        UFDTArr[i].ptrfiletable = NULL;
        i++;
    }

    superblockobj.TotalInodes = MAXINODE;
    superblockobj.FreeInodes = MAXINODE;
}

PINODE Get_Inode(char *name)
{
    PINODE temp = head;

    if(name  == NULL)
        return NULL;

    while (temp != NULL)
    {
        if(strcmp(name , temp->FileName) == 0)
            break;
        temp = temp->next;    
    }
    return temp;
}

int GetFdFromName(char *name)
{
    int fd = 0;

    while(fd < 50)
    {
        if(UFDTArr[fd].ptrfiletable != NULL)
        {
             if(strcmp((UFDTArr[fd].ptrfiletable->ptrinode->FileName ), name) == 0)
                {
                    break;
                }
        }       
        fd++;
    }

    if( fd == 50)
        return -1;
    else
        return fd;
}



void CreateDILB()
{
    int i = 1;
    PINODE newn = NULL;
    PINODE temp = head;

    while(i <= MAXINODE)
    {
        newn = (PINODE)malloc(sizeof(INODE));
        
        newn->LinkCount = 0;
        newn->ReferenceCount = 0;
        newn->FileSize = 0;
        newn->FileType = 0;

        newn->Buffer = NULL;
        newn->next = NULL;

        newn->InodeNumber = i;

        if(temp == NULL)
        {
            head = newn;
            temp = head;
        }
        else
        {
            temp->next = newn;
            temp = temp->next;
        }
        i++;
    }
    printf("DILB Created Succefully \n");
}

void DislayHelp()
{
    printf("ls : To List Out all files\n");
    printf("clear : to clear console\n");
    printf("open : To open the file\n");
    printf("open : To open the file\n");
    printf("closeall : to colle all the opened files\n");
    printf("read : To Read contents from file\n");
    printf("write : To Write contents in the file \n");
    printf("stat : TO Display information of file using name\n");
    printf("fstat : TO Desplay information of file using File descriptor\n");
    printf("truncate : To Remove all Data from the file\n");
    printf("rm : To Delete the file \n");
}

void man(char *name)
{
    if(name == NULL)
        return;
    if(strcmp(name , "create") == 0)
    {
        printf("Description : Used to create new regular file\n");
        printf("Usage : create FIle_name Permission(1,2,3)\n");
    }
    else if(strcmp(name , "read") == 0)
    {
        printf("Description : Used to read data from refular file\n");
        printf("Usage : red File_name No_of_Bytes_To_Read\n");
    }
    else if(strcmp(name , "write") == 0)
    {
        printf("Description : used to write int regular file \n");
        printf("Usage : write FIle_name\n After this enter the data that we want to write\n");
    }
    else if(strcmp(name , "ls") == 0)
    {
        printf("Description : Used to  List all information of files\n");
        printf("Usage : ls\n");
    }
    else if(strcmp(name , "stat") == 0)
    {
        printf("Description : use to display the information of file\n");
        printf("Usage : stat File_name\n");
    }
    else if(strcmp(name , "fstat") == 0)
    {
        printf("Description : use to display the information of file\n");
        printf("Usage : stat File_Descriptor(3 to 50)\n");
    }
    else if(strcmp(name , "truncate") == 0)
    {
        printf("Description : use to remove data from the file \n");
        printf("Usage :truncate File_name\n");
    }
    else if(strcmp(name , "open") == 0)
    {
        printf("Description : used to open te existing file \n");
        printf("Usage : open File_name mode( 1 2 3)\n");
    }
    else if(strcmp(name , "close") == 0)
    {
        printf("Description : used to close opened file \n");
        printf("Usage :close File_name\n");
    }
    else if(strcmp(name , "closeall") == 0)
    {
        printf("Description : Used to close all opened file\n");
        printf("Usage :closeall\n");
    }
    else if(strcmp(name , "lseek") == 0)
    {
        printf("Description : used to change the file offset \n");
        printf("Usage : lseek File_name ChangeInoffSet StartPoint\n");
    }
    else if(strcmp(name , "rm") == 0)
    {
        printf("Description : used to delete the file \n");
        printf("Usage :rm File_Name\n");
    }
    else
    {
        printf("ERROR : No Manual entry available\n");
    }
}

int CreatFile(char *name , int permission)
{
    int  i = 3;
    PINODE temp = head;

    if((name == NULL) || (permission == 0) || (permission > 3))
        return -1;

    if(superblockobj.FreeInodes == 0)
        return -2;

    (superblockobj.FreeInodes)--;

    if(Get_Inode(name) != NULL)
        return -3;

    while(temp != NULL)// Gettin the Inode for our file
    {
        if(temp ->FileType == 0)
            break;
        temp = temp->next;
    }

    while(i < 50)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
            break;
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

    UFDTArr[i].ptrfiletable->Count = 1;
    UFDTArr[i].ptrfiletable->readoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;
    UFDTArr[i].ptrfiletable->mode = permission;

    UFDTArr[i].ptrfiletable->ptrinode = temp;
    
    strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName , name);
    UFDTArr[i].ptrfiletable->ptrinode->FileType = REGULAR;
    UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->LinkCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->FileSize = MAXFILESIZE;
    UFDTArr[i].ptrfiletable->ptrinode->FileActualSize = 0;
    UFDTArr[i].ptrfiletable->ptrinode->Permission = permission;    
    UFDTArr[i].ptrfiletable->ptrinode->Buffer = (char *)malloc(MAXFILESIZE);

    return i;
}

void ls_file()
{
    PINODE temp = head;+
    printf("LS");

    if(superblockobj.FreeInodes == MAXINODE)
       { printf("There are No Files\n");
         return;}

    printf("\nFileName \t Inode numer \tFile Size\t Link Count\n");
    printf("\n--------------------------------------------------------------------------\n");
    while(temp != NULL)
    {
        if(temp->FileType != 0)
        {
            printf("%s\t\t%d\t\t%d\t\t%d\t\n" , temp->FileName , temp->InodeNumber , temp->FileActualSize , temp->LinkCount);
        }
        temp = temp->next;
    }
    printf("\n--------------------------------------------------------------------------\n");

}

int stat_file(char *name)
{
    PINODE temp = head;

    if(name == NULL)
        return -1;

    while(temp != NULL)
    {
        if(strcmp(name , temp->FileName) == 0)
        {
            break;
        }
        temp = temp->next;
    }

    if(temp == NULL)
        return -2;

    
    printf("----------------------------Statistical information of File ---------------------------\n");
    printf("FileName : %s\n",temp->FileName);
    printf("InodeNumber : %d\n",temp->InodeNumber);
    printf("FileSize :%d\n",temp->FileSize);
    printf("FileActualSize : %d\n",temp->FileActualSize);
    printf("LinkCount : %d\n",temp->LinkCount);
    printf("ReferenceCount : %d\n",temp->ReferenceCount);

    if(temp->Permission == 1)
        printf("File Permission : Read Only\n");
    if(temp->Permission == 2)
        printf("File Permission : Write Only\n");
    if(temp->Permission == 3)
        printf("File Permission : Read & Write Only\n");

    printf("-------------------------------------------------------------------------------------\n");

    return 0;
}

int fstat_file(int fd)
{
    PINODE temp = NULL;

    if(fd < 3)
        return -1;

    if(UFDTArr[fd].ptrfiletable == NULL)
        return -2;
    
    temp = UFDTArr[fd].ptrfiletable->ptrinode;

     printf("----------------------------Statistical information of File using File Descriptor(FD)---------------------------\n");
    printf("FileName : %s\n",temp->FileName);
    printf("InodeNumber : %d\n",temp->InodeNumber);
    printf("FileSize :%d\n",temp->FileSize);
    printf("FileActualSize : %d\n",temp->FileActualSize);
    printf("LinkCount : %d\n",temp->LinkCount);
    printf("ReferenceCount : %d\n",temp->ReferenceCount);

    if(temp->Permission == 1)
        printf("File Permission : Read Only\n");
    if(temp->Permission == 2)
        printf("File Permission : Write Only\n");
    if(temp->Permission == 3)
        printf("File Permission : Read & Write Only\n");

    printf("-------------------------------------------------------------------------------------\n");

    return 0;
}

void CloseAllFile()
{
    int i = 0;

    while(i < 50)
    {
        if(UFDTArr[i].ptrfiletable != NULL)
        {
            UFDTArr[i].ptrfiletable->readoffset = 0;
            UFDTArr[i].ptrfiletable->writeoffset = 0;
            (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
        }
        i++;
    }
}

int CloseFileByName(char *name)
{
    int fd = GetFdFromName(name);
    if(fd == -1)
        return -1;

    UFDTArr[fd].ptrfiletable->readoffset = 0;
    UFDTArr[fd].ptrfiletable->writeoffset = 0;

    (UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount)--;

    return 0;
}

int CloseFileByName(int fd)
{
    UFDTArr[fd].ptrfiletable->readoffset = 0;
    UFDTArr[fd].ptrfiletable->writeoffset = 0;

    (UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount)--;
}

int rm_File(char *name)
{
    int fd = 0;
    fd = GetFdFromName(name);
    if(fd == -1)
        return -1;

    (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;

    if((UFDTArr[fd].ptrfiletable->ptrinode->LinkCount) == 0)
    {
        UFDTArr[fd].ptrfiletable->ptrinode->FileType = 0;
        free(UFDTArr[fd].ptrfiletable);
    }
    UFDTArr[fd].ptrfiletable = NULL;
    (superblockobj.FreeInodes)++;
}

int WriteFile(int fd , char* arr , int isize)
{
    if((UFDTArr[fd].ptrfiletable->mode != WRITE) && (UFDTArr[fd].ptrfiletable->mode != READ + WRITE))
        return -1;
    
    if((UFDTArr[fd].ptrfiletable->ptrinode->Permission != WRITE) &&(UFDTArr[fd].ptrfiletable->ptrinode->Permission != READ + WRITE) )
        return -1;

    if(UFDTArr[fd].ptrfiletable->writeoffset == MAXFILESIZE)
        return -2;

    if(UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)
        return -3;

    strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->writeoffset) , arr , isize);
    (UFDTArr[fd].ptrfiletable->writeoffset) += isize;
    (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+=isize;

    return isize;
}

int truncate(char *name)
{
    int fd = GetFdFromName(name);
    if(fd == -1)
        return -1;
    
    memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer , 0 , 1024);// memory reset all the bytes will change to zero 0;

    UFDTArr[fd].ptrfiletable->readoffset = 0;
    UFDTArr[fd].ptrfiletable->writeoffset = 0;

    UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = 0;
}

int OpenFile(char *name , int mode)
{
    int i = 3;
    PINODE temp = NULL;

    if(name == NULL || mode < 0)
        return -1;

    temp = Get_Inode(name);
    if(temp == NULL)
        return -2;
    
    if(temp->Permission < mode) // 1 < 2  = denied || 1 < 3 = denied || 2<3 = denied
        return -3;

    while(i < 50)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
        {
            break;
        }
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
    if(UFDTArr[i].ptrfiletable == NULL)
        return -4;

    UFDTArr[i].ptrfiletable->Count = 1;
    UFDTArr[i].ptrfiletable->mode = mode;

    if(mode == READ + WRITE)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
        UFDTArr[i].ptrfiletable->writeoffset = 0;        
    }
    else if(mode == READ)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;      
    }
    else if(mode == WRITE)
    {
        UFDTArr[i].ptrfiletable->writeoffset = 0;      
    }

    UFDTArr[i].ptrfiletable->ptrinode = temp;

    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)++;

    return i ;
}

int ReadFile(int fd , char *arr , int isize)
{
    int read_size = 0;

    if(UFDTArr[fd].ptrfiletable == NULL)
        return -1;
    
    if(UFDTArr[fd].ptrfiletable->mode != READ && UFDTArr[fd].ptrfiletable->mode != READ + WRITE )
        return -2;
    
    if(UFDTArr[fd].ptrfiletable->ptrinode->Permission != READ && UFDTArr[fd].ptrfiletable->ptrinode->Permission != READ  + WRITE )
        return -2;
    
    if(UFDTArr[fd].ptrfiletable->readoffset ==  UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
        return -3;

    if(UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)
        return -4;

    read_size = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) - (UFDTArr[fd].ptrfiletable->readoffset);

    if(read_size < isize)
    {
        strncpy(arr , (UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset) , read_size);

        (UFDTArr[fd].ptrfiletable->readoffset) = ((UFDTArr[fd].ptrfiletable->readoffset) + read_size);

        arr[read_size] = '\0';
        return read_size;
    }
    else
    {
        strncpy(arr , (UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset) , isize);

        (UFDTArr[fd].ptrfiletable->readoffset) = ((UFDTArr[fd].ptrfiletable->readoffset) + isize);
    }
    
    arr[isize] = '\0';
    return isize;
}

int LseekFile(int fd , signed int isize , int from)
{
    if(fd <  0 || from > 2)
        return -1;
    
    if(UFDTArr[fd].ptrfiletable == NULL)
        return -2;

    if((UFDTArr[fd].ptrfiletable->mode == READ )|| (UFDTArr[fd].ptrfiletable->mode) == READ + WRITE )
    {
        if(from == CURRENT)
        {
            if(((UFDTArr[fd].ptrfiletable->readoffset ) + isize) > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
            {
                return -3;
            }

            if(((UFDTArr[fd].ptrfiletable->readoffset) + isize) < 0)
                return -1;
                (UFDTArr[fd].ptrfiletable->readoffset ) = ((UFDTArr[fd].ptrfiletable->readoffset ) + isize);
        }
        else if(from == START)
        {
            if(isize > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
                return -3;
            if(isize < 0)
                return -1;
            
            UFDTArr[fd].ptrfiletable->readoffset = isize;
        }
        else if(from == END)
        {
            if((isize + (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)) > MAXFILESIZE)
                return -3;
    
            if(((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + isize) < 0)
                return -1;
            
           (UFDTArr[fd].ptrfiletable->readoffset) = ((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + isize); // this i size could be negative;
        }
    }
    if(UFDTArr[fd].ptrfiletable->mode == WRITE || (UFDTArr[fd].ptrfiletable->mode) == READ + WRITE )
    {
        if(from == CURRENT)
        {
            if((isize + (UFDTArr[fd].ptrfiletable->writeoffset)) > MAXFILESIZE)
                return -3;
            
            if((isize + (UFDTArr[fd].ptrfiletable->writeoffset)) < 0)
                return -3;
            if((isize + (UFDTArr[fd].ptrfiletable->writeoffset)) >(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
            {
                (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (isize + (UFDTArr[fd].ptrfiletable->writeoffset));
            }
            else
            {
                (UFDTArr[fd].ptrfiletable->writeoffset)= (isize + (UFDTArr[fd].ptrfiletable->writeoffset));
            }
        }
        if(from  == START)
        {
            if(isize > MAXFILESIZE || isize < 0)
                return -3;
            else if(isize > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
                (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = isize;
            else 
                (UFDTArr[fd].ptrfiletable->writeoffset) = isize;
        }
        if(from == END)
        {
            if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) > MAXFILESIZE)
                return -3;
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + isize)  < 0) 
                return -3;
            
            (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + isize;
        }
    }
}

int main()
{
    char *ptr = NULL;
    int ret = 0 , fd = 0 , count = 0;
    char str[80] , command[4][80] , arr[MAXFILESIZE];
    InitializeSuperblock();
    CreateDILB();
    printf("enter help for help\n ");

    while(1)
    {
        fflush(stdin);
        strcpy(str , "");
        printf("////////////////////////////////////////////////////////////////////////////////////////////////////////////\n");
        printf("Sandeshs CVFS : > ");

        fgets(str , 80 , stdin);

        count = sscanf(str ,"%s %s %s %s" , command[0] , command[1] , command[2] , command[3]);

        if(count == 1)
        {
            if(strcmp(command[0] , "ls") == 0)
            {
                ls_file();
            }
            else if(strcmp(command[0] , "closeall") == 0)
            {
                CloseAllFile();
                printf("All Files Closed Successfully\n");
                continue;               
            }
            else if(strcmp(command[0] , "clear") == 0)
            {
                system("cls");
                continue;
            }
            else if(strcmp(command[0] , "help") == 0)
            {
                DislayHelp();
                continue;
            }
            else if(strcmp(command[0] , "exit") == 0)
            {
                printf("Terminating the Sandeshs Virtual File System !!!!!!!!!!\n");
                break;
            }
            else
            {
                printf("ERROR : Command Not found !!!!\n");
                continue;
            }
        }
        else if(count == 2)
        {
            if(strcmp(command[0] , "stat") == 0)
            {
                ret = stat_file(command[1]);
                if(ret == -1)
                    printf("ERROR : Incorrect Parameters\n");
                else if(ret == -2)
                    printf("ERROR : File Not Found\n");
                continue;
            }
            else if(strcmp(command[0] , "fstat") == 0)
            {
                ret = fstat_file(atoi(command[1]));
                if(ret == -1)
                    printf("ERROR : Incorrect Parameters\n");
                else if(ret == -2)
                    printf("ERROR : File Not Found\n");
                continue;
            }
            else if(strcmp(command[0] , "close") == 0)
            {
                ret = CloseFileByName(command[1]);
                if(ret == -1)
                    printf("ERROR : there is no such File\n");
                continue;
            }
            else if(strcmp(command[0] , "rm") == 0)
            {
                ret = rm_File(command[1]);
                if(ret == -1)
                    printf("ERROR : there is no such File\n");
                continue;
            }
            else if(strcmp(command[0] , "man") == 0)
            {
                man(command[1]);
            }
            else if(strcmp(command[0] , "write") == 0)
            {
                fd = GetFdFromName(command[1]);
                if(fd == -1)
                {
                    printf("ERROR : Incorrect Parameters\n");
                    continue;
                }

                printf("Enter the Data : \n");
                scanf("%[^'\n']s" , arr);

                ret = strlen(arr);
                if(ret == 0)
                {
                    printf("Incorrect Parameters\n");
                    continue;
                }

                ret = WriteFile(fd , arr , ret);
                if(ret == -1)
                    printf("ERROR : Permission Denied\n");
                if(ret == -2)
                    printf("ERROR : No Suffcient Memory\n");
                if(ret == -3)
                    printf("ERROR : It is Not a Regular File\n");                    
            }
            else if(strcmp(command[0] , "truncate") == 0)
            {
                ret = truncate(command[1]);
                if(ret == -1)
                    printf("ERROR: File Not Found\n");
                else
                    printf("File Cleared Successfully\n");
                continue;
            }
            else
            {
                printf("ERROR : Command Not found !!!!\n");
                continue;
            }

        }
        else if(count == 3)
        {
            if(strcmp(command[0] , "create") == 0)
            {
                ret = CreatFile(command[1] , atoi(command[2]));
                if(ret >= 0)
                {
                    printf("File is SucceFully Created with file Descriptor: %d \n",ret);
                }
                else if(ret == -1)
                {
                    printf(" ERROR : Incorrect Parameters\n");
                }
                else if(ret == -2)
                {
                    printf("ERROR : There is no any Free Inodes\n");
                }
                else if(ret == -3)
                {
                    printf("ERROR : File already Exits with entered name\n");
                }
                else if(ret == -4)
                {
                    printf("ERROR : Memory Allocation Failure\n");
                }
                continue;
            }
            else if(strcmp(command[0] , "open") == 0)
            {
                ret = OpenFile(command[1] , atoi(command[2]));
                if(ret == -1)
                    printf("ERROR: Incorrect Paremeters");
                else if(ret == -2)
                    printf("ERROR : File Does Not Exist\n");
                else if(ret == -3)
                    printf("ERROR : Permission Denied\n");
                else if(ret == -4)
                    printf("ERROR : Unable to Allocate the memory\n");
                else
                    printf("FIle Succefully opened with FD : %d\n",ret);
                continue;
            }
            else if(strcmp(command[0] , "read") == 0 )
            {
                fd = GetFdFromName(command[1]);
                if(fd == -1 )
                {
                     printf("Incorect Parameter\n");
                    continue;
                }
                   

                ptr = (char *)malloc(atoi(command[2]) + 1);
                if(ptr == NULL)
                {
                    printf("ERROR : Memory Allocation Failure\n");
                    continue;
                }

                ret = ReadFile(fd , ptr , atoi(command[2]));
                if(ret == -1)
                    printf("File Not Found\n");
                if(ret == -2)
                    printf("Permission Denied\n");
                if(ret == -2)
                    printf("Permission Denied\n");
                if(ret == -3)
                    printf("Reached At the End Of File\n");
                if(ret == -4)
                    printf("File Type is Not Regular\n");
                if(ret == 0)
                    printf("ERROR : File is Empty\n");
                if(ret > 0)
                {
                    printf("Printing the File Data : \n");
                    printf("%s\n",ptr);
                    write(1,ptr,ret);
                    printf("\n");
                }
                continue;
            }
            else
            {
                printf("ERROR : Command Not found !!!!\n");
                continue;
            }

        }
        else if(count == 4)
        {
            if(strcmp(command[0] , "lseek") == 0)
            {
                fd = GetFdFromName(command[1]);
                if(fd == -1)
                {
                    printf("Incorrect Parameter\n");
                    continue;
                }
                ret = LseekFile(fd , atoi(command[2]) , atoi(command[3]));
                if(ret == -1)
                    printf("ERROR : Incorrect Parameter\n");
                if(ret == -2)
                    printf("ERROR : File not Present\n");
                if(ret == -3)
                    printf("ERROR : Exceedding the File size\n");
                

            }


        }
    }




    return 0;
}
