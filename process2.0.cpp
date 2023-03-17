#include <iostream>
#include <queue>
#include <stack>
#include <windows.h>

using namespace std;

const int ROM_SIZE = 3;
const int HARD_DISC_SIZE = 100;

// 状态枚举
enum State
{
    READY = 0,
    RUN = 1,
    BLOCK = 2
};

enum State_Pos
{
    NOT_IN_ROM = 0,
    IN_ROM = 1,
    IN_HARD_DISC = 2,
    NOT_IN_HARD_DISC = 3
};

// 进程结构
typedef struct Process
{
    int p_id;            // pid
    int pri_level;       // 优先级
    int size;            // 占内存的大小
    State state;         // 状态
    State_Pos pos_state; // 位置状态
    string info;         // 内容
} Process;

#pragma region ProcessTreeDeclare // 进程树相关声明
// 树节点定义
class ProcessTreeNode
{
public:
    Process data;
    ProcessTreeNode *left_chlid;
    ProcessTreeNode *right_chlid;
    ProcessTreeNode *parent;
    ProcessTreeNode();
};

// 进程树
class ProcessTree
{
private:
    ProcessTreeNode *traver_ptr; // 根节点
    ProcessTreeNode *root;       // 遍历指针

public:
    ProcessTree();
    ~ProcessTree();

    ProcessTreeNode *get_root() { return root; }
    ProcessTreeNode *get_address(int parent_pid);
    bool is_empty();                                      // 判空
    void insert(ProcessTreeNode *base, Process &process); // 插入进程
    void kill(ProcessTreeNode *root);                     // 终止进程
    void remove_sub_tree(ProcessTreeNode *root);          // 删除子树
    void display();                                       // 显示进程树
    void print(ProcessTreeNode _root, int nlayer = 1);    // 打印二叉树
};
#pragma endregion ProcessTreeDeclare

ProcessTree process_tree;
Process ROM[ROM_SIZE];
Process HARD_DISC[HARD_DISC_SIZE];
queue<Process> ready_queue;
queue<Process> run_queue;
queue<Process> block_queue;

// 主操作函数
void create();             // 创建新的进程
void get_runing_process(); // 査看运行进程
void swap_out();           // 换出某个进程
void kill();               // 杀死运行进程
void communicate();        // 进程之间通信
void exit();               // 退出系统

// 命令行操作函数
void main_menu();

// 其他函数
inline void print(string text);
void init();              // 初始化函数
bool ROM_is_full();       // 内存判满
int get_free_ROM();       // 获取空闲内存
int get_free_HARD_DISC(); // 获取空闲硬盘
bool HARD_DISC_is_full(); // 硬盘判满

bool is_exist(int p_id);    // pid查重
void set_pos(int x, int y); // 设置光标
void hide_pos();            // 隐藏光标

// 显示函数
void show_process_tree();
void show_ROM();
void show_HARD_DISC();
void show_ready_queue();
void show_run_queue();
void show_block_queue();

int main()
{

    init();

    while (1)
    {
        main_menu();
    }

    getchar();
    return 0;
}

#pragma region ProcessTreeFunction // 进程树相关函数定义
// 获取父地址
ProcessTreeNode *ProcessTree::get_address(int parent_pid)
{
    stack<ProcessTreeNode *> s;
    traver_ptr = root;

    while (traver_ptr || !s.empty())
    {
        if (traver_ptr)
        {
            s.push(traver_ptr);
            if (s.top()->data.p_id == parent_pid)
            {
                return s.top();
            }
            traver_ptr = traver_ptr->left_chlid;
        }
        else
        {
            traver_ptr = s.top()->right_chlid;
            s.pop();
        }
    }
    traver_ptr = root;
    return NULL;
}

// 树节点初始化
ProcessTreeNode::ProcessTreeNode()
{
    left_chlid = NULL;
    right_chlid = NULL;
    parent = NULL;
}

// 树初始化
ProcessTree::ProcessTree()
{
    root = NULL;
    traver_ptr = NULL;
}

// 释放
ProcessTree::~ProcessTree()
{
    kill(root);
}

// 树判空
bool ProcessTree::is_empty()
{
    if (root == NULL)
        return true;
    else
        return false;
}

// 根据父进程地址，创建子进程
void ProcessTree::insert(ProcessTreeNode *base, Process &process)
{
    ProcessTreeNode *new_process = new ProcessTreeNode;
    new_process->data = process;

    if (base == NULL)
    {
        root = new_process;
        return;
    }
    if (base->left_chlid == NULL)
    {
        base->left_chlid = new_process;
        base->left_chlid->parent = base;
        return;
    }
    if (base->right_chlid == NULL)
    {
        base->right_chlid = new_process;
        base->right_chlid->parent = base;
        return;
    }
    traver_ptr = root;
}

// 根据地址，终止进程
void ProcessTree::kill(ProcessTreeNode *_root)
{
    if (_root == root)
    {
        root = NULL;
        traver_ptr = root;
    }
    else
    {
        traver_ptr = _root->parent;
        if (_root == _root->parent->left_chlid)
            traver_ptr->left_chlid = NULL;
        else
            traver_ptr->right_chlid = NULL;
    }
    ProcessTree::remove_sub_tree(_root);
    traver_ptr = root;
}

// kill中递归部分实现
void ProcessTree::remove_sub_tree(ProcessTreeNode *_root)
{
    if (_root != NULL)
    {
        remove_sub_tree(_root->left_chlid);
        remove_sub_tree(_root->right_chlid);
        delete _root;
    }
    return;
}

// 显示该树
void ProcessTree::display()
{
    if (is_empty())
    {
        return;
    }

    stack<ProcessTreeNode *> s;
    traver_ptr = root;

    while (traver_ptr || !s.empty())
    {
        if (traver_ptr)
        {
            s.push(traver_ptr);
            cout << s.top()->data.p_id << " ";
            traver_ptr = traver_ptr->left_chlid;
        }
        else
        {
            traver_ptr = s.top()->right_chlid;
            s.pop();
        }
    }
    traver_ptr = root;
}

// 打印该二叉树
void print(ProcessTreeNode *_root, int nlayer = 0)
{
    if (_root == NULL)
        return;
    print(_root->right_chlid, nlayer + 1);
    for (int i = 0; i <= nlayer; i++)
        printf("    ");
    cout << "(" << _root->data.p_id << ")" << endl;
    print(_root->left_chlid, nlayer + 1);
}

#pragma endregion ProcessTreeFunction

#pragma region MainFuction // 主操作函数定义

void create() // 创建新的进程
{
    system("cls");
    print("[创建新的进程]");
    if (ROM_is_full())
    {
        print("内存已满,请先换出或杀死进程");
        system("pause");
        return;
    }
    else
    {
        size_t create_pos = get_free_ROM();
        int temp_pid;
        int temp_state;

        print("请输入新进程的PID");
        cout << ">>";
        cin >> temp_pid;
        if (is_exist(temp_pid))
        {
            print("!!该进程已存在,将返回主菜单!!");
            system("pause");
            return;
        }
        ROM[create_pos].p_id = temp_pid;

        print("请输入该进程优先级");
        cout << ">>";
        cin >> ROM[create_pos].pri_level;

        print("请输入该进程的状态(0:就绪,1:执行 ,2:阻塞)");
    agin_state:
        cout << ">>";
        cin >> temp_state;
        if (temp_state != 0 && temp_state != 1 && temp_state != 2)
        {
            print("请重新输入该进程的状态(0:就绪,1:执行 ,2:阻塞)");
            goto agin_state;
        }

        print("请输入该进程所占大小");
        cout << ">>";
        cin >> ROM[create_pos].size;

        print("请输入该进程的内容");
        cout << ">>";
        cin >> ROM[create_pos].info;

        ROM[create_pos].pos_state = IN_ROM;

        switch (temp_state)
        {
        case 0:
            ROM[create_pos].state = READY;
            ready_queue.push(ROM[create_pos]);
            break;
        case 1:
            ROM[create_pos].state = RUN;
            run_queue.push(ROM[create_pos]);
            break;
        case 2:
            ROM[create_pos].state = BLOCK;
            block_queue.push(ROM[create_pos]);
            break;
        default:
            break;
        }

        if (process_tree.is_empty())
        {
            process_tree.insert(NULL, ROM[create_pos]);
        }
        else if (!process_tree.get_root()->left_chlid && !process_tree.get_root()->right_chlid)
        {
            process_tree.insert(process_tree.get_root(), ROM[create_pos]);
        }
        else
        {
            int parent_pid;
        parent_pid_agin:
            print("进程树(直观)");
            cout << ">>" << endl;
            print(process_tree.get_root());
            print("请输入该进程的父进程p_id");

            cout << ">>";
            cin >> parent_pid;
            ProcessTreeNode *temp_ptr = process_tree.get_address
            (parent_pid);
            if (temp_ptr)
            {
                if (temp_ptr->left_chlid && temp_ptr->right_chlid)
                {
                    print("!!子进程数量已满!!");
                    goto parent_pid_agin;
                }
                process_tree.insert((temp_ptr), ROM[create_pos]);
            }
            else
            {
                print("!!未找到该pid!!");
                goto parent_pid_agin;
                system("pause");
            }
        }
    }
    print("进程创建成功");
    char YorN;
    print("是否继续创建进程？(Y/N)");
    cout << ">>";
    cin >> YorN;
    if (YorN == 'Y' || YorN == 'y')
    {
        create();
    }
    else
    {
        return;
    }
}

void get_runing_process() // 査看运行进程
{
    print("[査看运行进程]");
    system("cls");
    show_process_tree();
    show_ROM();
    show_HARD_DISC();
    show_ready_queue();
    show_run_queue();
    show_block_queue();
    system("pause");
}

void swap_out() // 换出某个进程
{
    int temp_pid;
    system("cls");
    print("[换出某个进程]");
    if (HARD_DISC_is_full())
    {
        print("硬盘已满无法换出");
        system("pause");
    }
    else
    {
        print("请输入换出进程的PID");
        cout << ">>";
        cin >> temp_pid;
        for (int i = 0; i <= ROM_SIZE; i++)
        {
            if (ROM[i].p_id == temp_pid && ROM[i].pos_state == IN_ROM)
            {
                ROM[i].pos_state = NOT_IN_ROM;
                int temp_pos = get_free_HARD_DISC();
                HARD_DISC[temp_pos].pos_state = IN_HARD_DISC;
                HARD_DISC[temp_pos].p_id = temp_pid;
                HARD_DISC[temp_pos].info = ROM[i].info;
                HARD_DISC[temp_pos].pri_level = ROM[i].pri_level;
                HARD_DISC[temp_pos].size = ROM[i].size;
                HARD_DISC[temp_pos].state = ROM[i].state;

                cout << "\n进程: " << temp_pid << "已换出" << endl;
                system("pause");
                return;
            }
        }
    }
    print("该进程不存在或已换出");
    system("pause");
}

void kill() // 杀死运行进程
{
    system("cls");
    int temp_pid;
    print("[杀死运行进程]");
    print("请输入你想杀死的进程的PID");
    cout << ">>";
    cin >> temp_pid;
    for (int i = 0; i <= ROM_SIZE; i++)
    {
        if (ROM[i].p_id == temp_pid && ROM[i].pos_state == IN_ROM)
        {

            system("pause");
            return;
        }
    }

    system("pause");
}

void communicate() // 进程之间通信
{
    system("cls");
    print("[进程之间通信]");

    system("pause");
}

void exit() // 退出系统
{

}

#pragma endregion MainFuction

#pragma region ShowFuction // 命令行操作函数
void main_menu()           // 主菜单
{
    system("cls");
    cout << "\n*******************************************"
         << "\n              进程演示系统                 "
         << "\n*******************************************"
         << "\n    1.创建新的进程       2.査看运行进程      "
         << "\n    3.换出某个进程       4.杀死运行进程      "
         << "\n    5.进程之间通信       6.退出系统          "
         << "\n*******************************************"
         << "\n[请选择(1~6)]\n";
    int user_input;
    cin >> user_input;
    switch (user_input)
    {
    case 1:
        create();
        break;
    case 2:
        get_runing_process();
        break;
    case 3:
        swap_out();
        break;
    case 4:
        kill();
        break;
    case 5:
        communicate();
        break;
    default:
        exit();
        break;
    }
}

#pragma endregion ShowFuction

#pragma region OtherFuction // 其他函数定义
inline void print(string text)
{
    cout << "\n"
         << text << "\n";
}

// 初始化函数
void init()
{
    for (size_t i = 0; i < ROM_SIZE; i++)
    {
        ROM[i].pos_state = NOT_IN_ROM;
    }
    for (size_t i = 0; i < HARD_DISC_SIZE; i++)
    {
        HARD_DISC[i].pos_state = NOT_IN_HARD_DISC;
    }
}

// 内存判满
bool ROM_is_full()
{
    for (size_t i = 0; i < ROM_SIZE; i++)
    {
        if (ROM[i].pos_state == NOT_IN_ROM)
        {
            return false;
        }
    }
    return true;
}

// 获取空闲内存
int get_free_ROM()
{
    if (ROM_is_full())
    {
        return -1;
    }
    else
    {
        for (size_t i = 0; i < ROM_SIZE; i++)
        {
            if (ROM[i].pos_state == NOT_IN_ROM)
            {
                return i;
            }
        }
    }
    return -1;
}

// 获取空闲硬盘
int get_free_HARD_DISC()
{
    if (HARD_DISC_is_full())
    {
        return -1;
    }
    else
    {
        for (size_t i = 0; i < HARD_DISC_SIZE; i++)
        {
            if (HARD_DISC[i].pos_state == NOT_IN_HARD_DISC)
            {
                return i;
            }
        }
    }
    return -1;
}

// 硬盘判满
bool HARD_DISC_is_full()
{
    for (size_t i = 0; i < HARD_DISC_SIZE; i++)
    {
        if (HARD_DISC[i].pos_state == NOT_IN_HARD_DISC)
        {
            return false;
        }
    }
    return true;
}

// pid_查重
bool is_exist(int p_id)
{
    for (size_t i = 0; i < ROM_SIZE; i++)
    {
        if (ROM[i].p_id == p_id && ROM[i].pos_state == IN_ROM)
        {
            return true;
        }
    }

    for (size_t i = 0; i < HARD_DISC_SIZE; i++)
    {
        if (HARD_DISC[i].p_id == p_id && HARD_DISC[i].pos_state == IN_HARD_DISC)
        {
            return true;
        }
    }

    return false;
}

// 设置光标到x,y
void set_pos(int x, int y)
{
    COORD position;
    HANDLE handle;
    position.X = x;
    position.Y = y;
    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(handle, position);
}

// 隐藏光标
void hide_pos()
{
    CONSOLE_CURSOR_INFO cursor_info = {1, FALSE};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}

#pragma endregion OtherFuction

#pragma region ShowFunction // 显示函数

void show_process_tree()
{
    print("进程树(直观)");
    cout << ">>" << endl;
    print(process_tree.get_root());
    print("进程树(先根)");
    cout << ">>";
    process_tree.display();
    cout << endl;
}

void show_ROM()
{
    print("内存中的进程");
    cout << ">>";
    for (int i = 0; i < ROM_SIZE; i++)
    {
        if (ROM[i].pos_state == IN_ROM)
            cout << ROM[i].p_id << " ";
    }
    cout << endl;
}

void show_HARD_DISC()
{
    print("硬盘中的进程");
    cout << ">>";
    for (int i = 0; i < HARD_DISC_SIZE; i++)
    {
        if (HARD_DISC[i].pos_state == IN_HARD_DISC)
            cout << HARD_DISC[i].p_id << " ";
    }
    cout << endl;
}

void show_ready_queue()
{
    queue<Process> temp_queue(ready_queue);
    print("就绪队列");
    cout << ">>";
    for (int i = 0; i < ready_queue.size(); i++)
    {
        cout << temp_queue.front().p_id << " ";
        temp_queue.pop();
    }
    cout << endl;
}

void show_run_queue()
{
    queue<Process> temp_queue(run_queue);
    print("执行队列:");
    cout << ">>";
    for (int i = 0; i < run_queue.size(); i++)
    {
        cout << temp_queue.front().p_id << " ";
        temp_queue.pop();
    }
    cout << endl;
}

void show_block_queue()
{
    queue<Process> temp_queue(block_queue);
    print("阻塞队列:");
    cout << ">>";
    for (int i = 0; i < block_queue.size(); i++)
    {
        cout << temp_queue.front().p_id << " ";
        temp_queue.pop();
    }
    cout << endl;
}

#pragma endregion ShowFunction