#include <iostream>
#include <queue>
#include <stack>
#include <windows.h>

using namespace std;

const int ROM_SIZE = 3;
const int HARD_DISC_SIZE = 100;

// ״̬ö��
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

// ���̽ṹ
typedef struct Process
{
    int p_id;            // pid
    int pri_level;       // ���ȼ�
    int size;            // ռ�ڴ�Ĵ�С
    State state;         // ״̬
    State_Pos pos_state; // λ��״̬
    string info;         // ����
} Process;

#pragma region ProcessTreeDeclare // �������������
// ���ڵ㶨��
class ProcessTreeNode
{
public:
    Process data;
    ProcessTreeNode *left_chlid;
    ProcessTreeNode *right_chlid;
    ProcessTreeNode *parent;
    ProcessTreeNode();
};

// ������
class ProcessTree
{
private:
    ProcessTreeNode *traver_ptr; // ���ڵ�
    ProcessTreeNode *root;       // ����ָ��

public:
    ProcessTree();
    ~ProcessTree();

    ProcessTreeNode *get_root() { return root; }
    ProcessTreeNode *get_address(int parent_pid);
    bool is_empty();                                      // �п�
    void insert(ProcessTreeNode *base, Process &process); // �������
    void kill(ProcessTreeNode *root);                     // ��ֹ����
    void remove_sub_tree(ProcessTreeNode *root);          // ɾ������
    void display();                                       // ��ʾ������
    void print(ProcessTreeNode _root, int nlayer = 1);    // ��ӡ������
};
#pragma endregion ProcessTreeDeclare

ProcessTree process_tree;
Process ROM[ROM_SIZE];
Process HARD_DISC[HARD_DISC_SIZE];
queue<Process> ready_queue;
queue<Process> run_queue;
queue<Process> block_queue;

// ����������
void create();             // �����µĽ���
void get_runing_process(); // �˿����н���
void swap_out();           // ����ĳ������
void kill();               // ɱ�����н���
void communicate();        // ����֮��ͨ��
void exit();               // �˳�ϵͳ

// �����в�������
void main_menu();

// ��������
inline void print(string text);
void init();              // ��ʼ������
bool ROM_is_full();       // �ڴ�����
int get_free_ROM();       // ��ȡ�����ڴ�
int get_free_HARD_DISC(); // ��ȡ����Ӳ��
bool HARD_DISC_is_full(); // Ӳ������

bool is_exist(int p_id);    // pid����
void set_pos(int x, int y); // ���ù��
void hide_pos();            // ���ع��

// ��ʾ����
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

#pragma region ProcessTreeFunction // ��������غ�������
// ��ȡ����ַ
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

// ���ڵ��ʼ��
ProcessTreeNode::ProcessTreeNode()
{
    left_chlid = NULL;
    right_chlid = NULL;
    parent = NULL;
}

// ����ʼ��
ProcessTree::ProcessTree()
{
    root = NULL;
    traver_ptr = NULL;
}

// �ͷ�
ProcessTree::~ProcessTree()
{
    kill(root);
}

// ���п�
bool ProcessTree::is_empty()
{
    if (root == NULL)
        return true;
    else
        return false;
}

// ���ݸ����̵�ַ�������ӽ���
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

// ���ݵ�ַ����ֹ����
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

// kill�еݹ鲿��ʵ��
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

// ��ʾ����
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

// ��ӡ�ö�����
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

#pragma region MainFuction // ��������������

void create() // �����µĽ���
{
    system("cls");
    print("[�����µĽ���]");
    if (ROM_is_full())
    {
        print("�ڴ�����,���Ȼ�����ɱ������");
        system("pause");
        return;
    }
    else
    {
        size_t create_pos = get_free_ROM();
        int temp_pid;
        int temp_state;

        print("�������½��̵�PID");
        cout << ">>";
        cin >> temp_pid;
        if (is_exist(temp_pid))
        {
            print("!!�ý����Ѵ���,���������˵�!!");
            system("pause");
            return;
        }
        ROM[create_pos].p_id = temp_pid;

        print("������ý������ȼ�");
        cout << ">>";
        cin >> ROM[create_pos].pri_level;

        print("������ý��̵�״̬(0:����,1:ִ�� ,2:����)");
    agin_state:
        cout << ">>";
        cin >> temp_state;
        if (temp_state != 0 && temp_state != 1 && temp_state != 2)
        {
            print("����������ý��̵�״̬(0:����,1:ִ�� ,2:����)");
            goto agin_state;
        }

        print("������ý�����ռ��С");
        cout << ">>";
        cin >> ROM[create_pos].size;

        print("������ý��̵�����");
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
            print("������(ֱ��)");
            cout << ">>" << endl;
            print(process_tree.get_root());
            print("������ý��̵ĸ�����p_id");

            cout << ">>";
            cin >> parent_pid;
            ProcessTreeNode *temp_ptr = process_tree.get_address
            (parent_pid);
            if (temp_ptr)
            {
                if (temp_ptr->left_chlid && temp_ptr->right_chlid)
                {
                    print("!!�ӽ�����������!!");
                    goto parent_pid_agin;
                }
                process_tree.insert((temp_ptr), ROM[create_pos]);
            }
            else
            {
                print("!!δ�ҵ���pid!!");
                goto parent_pid_agin;
                system("pause");
            }
        }
    }
    print("���̴����ɹ�");
    char YorN;
    print("�Ƿ�����������̣�(Y/N)");
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

void get_runing_process() // �˿����н���
{
    print("[�˿����н���]");
    system("cls");
    show_process_tree();
    show_ROM();
    show_HARD_DISC();
    show_ready_queue();
    show_run_queue();
    show_block_queue();
    system("pause");
}

void swap_out() // ����ĳ������
{
    int temp_pid;
    system("cls");
    print("[����ĳ������]");
    if (HARD_DISC_is_full())
    {
        print("Ӳ�������޷�����");
        system("pause");
    }
    else
    {
        print("�����뻻�����̵�PID");
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

                cout << "\n����: " << temp_pid << "�ѻ���" << endl;
                system("pause");
                return;
            }
        }
    }
    print("�ý��̲����ڻ��ѻ���");
    system("pause");
}

void kill() // ɱ�����н���
{
    system("cls");
    int temp_pid;
    print("[ɱ�����н���]");
    print("����������ɱ���Ľ��̵�PID");
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

void communicate() // ����֮��ͨ��
{
    system("cls");
    print("[����֮��ͨ��]");

    system("pause");
}

void exit() // �˳�ϵͳ
{

}

#pragma endregion MainFuction

#pragma region ShowFuction // �����в�������
void main_menu()           // ���˵�
{
    system("cls");
    cout << "\n*******************************************"
         << "\n              ������ʾϵͳ                 "
         << "\n*******************************************"
         << "\n    1.�����µĽ���       2.�˿����н���      "
         << "\n    3.����ĳ������       4.ɱ�����н���      "
         << "\n    5.����֮��ͨ��       6.�˳�ϵͳ          "
         << "\n*******************************************"
         << "\n[��ѡ��(1~6)]\n";
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

#pragma region OtherFuction // ������������
inline void print(string text)
{
    cout << "\n"
         << text << "\n";
}

// ��ʼ������
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

// �ڴ�����
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

// ��ȡ�����ڴ�
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

// ��ȡ����Ӳ��
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

// Ӳ������
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

// pid_����
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

// ���ù�굽x,y
void set_pos(int x, int y)
{
    COORD position;
    HANDLE handle;
    position.X = x;
    position.Y = y;
    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(handle, position);
}

// ���ع��
void hide_pos()
{
    CONSOLE_CURSOR_INFO cursor_info = {1, FALSE};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}

#pragma endregion OtherFuction

#pragma region ShowFunction // ��ʾ����

void show_process_tree()
{
    print("������(ֱ��)");
    cout << ">>" << endl;
    print(process_tree.get_root());
    print("������(�ȸ�)");
    cout << ">>";
    process_tree.display();
    cout << endl;
}

void show_ROM()
{
    print("�ڴ��еĽ���");
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
    print("Ӳ���еĽ���");
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
    print("��������");
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
    print("ִ�ж���:");
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
    print("��������:");
    cout << ">>";
    for (int i = 0; i < block_queue.size(); i++)
    {
        cout << temp_queue.front().p_id << " ";
        temp_queue.pop();
    }
    cout << endl;
}

#pragma endregion ShowFunction