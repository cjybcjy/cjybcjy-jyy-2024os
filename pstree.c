#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <getopt.h>

//定义进程结构体
//孩子兄弟表示法可以呈现二叉树的形态
// if in 64bits‘ machine：size of struct is 4B + 4B + 256B + 8B + 8B
typedef struct process {
    int pid;
    int ppid;
    char name[256];
    struct process *next_sibling;
    struct process *children;
} process_t;

//用数组储藏二叉树结构 2^15
process_t *processes[32768] = {0};

void add_child(process_t *parent, process_t *child) {
    child->next_sibling = parent->children;
    parent->children = child;
}

process_t *create_process(int pid, int ppid, const char *name) {
    process_t *proc = (process_t *) malloc (sizeof(process_t));
    if (proc == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    proc->pid = pid;
    proc->ppid = ppid;
    strncpy(proc->name, name, 255);
    proc->name[255] = '\0';//确保字符串以’\0‘结尾
    proc->next_sibling= NULL;
    proc->children = NULL;
    return proc;
}

//获取/proc目录
void parse_information(const char *path, int *pid, int *ppid, const char *name) {
    FILE *fd = fopen (path,  "r");
    if (fd) {
        //proc目录文件格式：pid (process name) state ppid ...
        //(%255[^)]) 用于解析包含在括号内的字符串
        fscanf(fd, "%d (%255[^)]) %*c %d", pid, name, ppid);//%*c 跳过一个字符（状态）
        fclose(fd);
    } else {
        perror("fopen");
    }
}


//处理读到的信息
void handle_process(int pid, int ppid, char *name) {//储存到指针数组processes中
    //cheak processes[pid] is created
    if (!processes[pid]) {
        processes[pid] = create_process(pid, ppid, name); 
    } else {//created but is a placeholder, update
        strcpy(processes[pid]->name, name);
        processes[pid]->ppid = ppid;
    }

    if (ppid > 0) {
        if (!processes[ppid]) {//if ppid proc isn't visited, create a placeholder
            processes[ppid] = create_process(ppid, 0, "parent_placeholder");
        }
        add_child(processes[ppid], processes[pid]);
    }
}

//构建进程树 fill processes-array 
void build_process_tree() {
    struct dirent *entry;
    DIR *dp = opendir("/proc");
    if (dp == NULL) {
        perror("opendir");
        return;
    }
    while ((entry = readdir(dp))) {// reads the next entry in the directory stream and returns a pointer to a struct dirent that describes the file.
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {//directories named with digits correspond to process IDs
            int pid, ppid;
            char name[256];
            char stat_path[256];
            snprintf(stat_path, "/pro/%s/statue", entry->d_name);//读取到缓冲区
            parse_information(stat_path, &pid, &ppid, name);
        //建立数据结构
            handle_process(pid, ppid, name);
        }
    }
    closedir(dp);
}

void quicksort(process_t **arr, int left, int right) {
    if (left > right) {
        return;
    }
    int pivot = arr[right]->pid;
    int i = left;
    int j = right -1;//right 元素为pivot

    while (i <= j) {
        //find two change position
        while(i <= j && arr[i]->pid <= pivot) {
            i++;
        }
        while(i <= j && arr[j]->pid > pivot) {
            j--;
        }
        if (i < j) {
            process_t *temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
            quicksort(arr, 0, i -1);
            quicksort(arr, i + 1, j);
        } 
    }
}
    

//打印进程树
//recursively print a process tree with indentation based on hierarchy levels,
// showing process IDs if requested, and optionally sorting child processes by their PID:
void print_process_tree(process_t *proc, int level, int slow_pids, int numeric_sort) {
    //层次结构
    for (int i = 0; i < level; i++) {
        printf("  ");
    }

    if (slow_pids) {
        printf("%s (%d)\n", proc->name, proc->pid);
        //	()：这两个圆括号是普通字符，将被直接输出到最终结果中，通常用于在视觉上区分或突出显示数值。
    } else {
        printf("%s", proc->name);
    } 

    //collect kids
    process_t *sorted_children[1024] = {0};
    int count = 0;
    for (process_t *child = proc->children; child; child = child->next_sibling) {
        sorted_children[count++] = child; 
    }

    if (numeric_sort) {//sort
        quicksort(sorted_children, 0, count - 1);
    }

    //打印子进程
    for (int i = 0; i < count; i++) {
        print_process_tree(sorted_children, level + 1, slow_pids, numeric_sort);
    }

}


//打印版本信息
void print_version() {
    printf("process tree version 1.0 \n");
}

//-V -n -p选项： 得到命令行参数，根据要求设置标志变量的数值
int main(int argc, char *argv[]) {
    int show_pids = 0;
    int numeric_sort = 0;
    int version = 0;

    // for (int i = 0; i < argc; i++) {
    //     assert(argv[i]);////根据 C 标准，命令行参数的数组 argv 中的每个元素都应该是一个有效的字符串指针
    //     if (strcmp(argv[i], "--show-pids") == 0 || strcmp(argv[i], "-p") == 0) {
    //         show_pids = 1;
    //     } else if (strcmp(argv[i], "--numeric-sort") == 0 || strcmp(argv[i], "-n") == 0) {
    //         numeric_sort = 1;
    //     } else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-V") == 0) {
    //         version = 1;
    //     } else {
    //         printf(stderr, "Usage: %s [--show-pids] [--numeric-sort] [--version]\n", argv[0]);//argv[0] is name of process
    //         //标准错误输出，Print instructions for use
    //         exit(EXIT_FAILURE);
    //         //When you call exit, it causes the program to finish, returning control to the operating system. 
    //     }
    // }
    // assert(!argv[argc]);//字符串指针以NULL结尾,解读完成

    struct  option long_options[]={//使程序能够解析命令行的长选项
        {"show-pids", 0, 0, 'p'},
        {"numeric_sort", 0, 0, 'n'},
        {"version", 0, 0, 'V'},
        {0, 0, 0, 0}//结束标志
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "pnV", long_options, NULL)) != -1) {
        switch (opt) {
        case 'p':
            show_pids = 1;
            break;
        case 'n':
            numeric_sort = 1;
            break;
        case 'V':
            version = 1;
            break;
        default:
            printf(stderr, "usage: %s [-p] [-n] [-V\n]", argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }
    

    if (version) {
        print_version();
    }

    if (numeric_sort || show_pids) {
        build_process_tree();
        //设 pid 为 1 的进程是树的根
        if (processes[1]) {
            print_process_tree(processes[1], 0, show_pids, numeric_sort);
        } else {
            printf(stderr, "processes[1] is not found");
            exit(EXIT_FAILURE);
        }   
    }
    return 0;
}