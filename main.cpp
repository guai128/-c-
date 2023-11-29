#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <deque>
#include <set>
#include <unordered_set>
#include <fstream>
#include <stack>

using namespace std;

const int PageSizeTable[4] {5, 11, 17, 31};
const int PageMaxSize = 100;
int ticks = 0;
struct Process
{
    explicit Process(string _name)
        : name {std::move(_name)}, old_reference {0}
    {
        this->size = PageSizeTable[rand() % 4];
        this->server_time = (rand() % 10) + 1;
        this->ownedPages = vector<int>(this->size, -1);
        // cout << this->name << "\t" << size << "\t" << server_time << endl;
    }

    string name;
    int size;
    int server_time;
    int old_reference;
    vector<int> ownedPages;
};

struct Page
{
    int number;
    bool used;
    int last_visited;
    int used_times;
};

class PageManager
{
public:
    explicit PageManager(int size)
        : pages(size)
    {
        for (int i=0; i!=size; ++i)
        {
            pages[i].number = i;
            pages[i].used   = false;
        }
    }

public: // 功能函数
    int freePages()
    {
        int cnt = 0;
        for (auto& elem : pages)
            if (! elem.used)
                ++ cnt;
        return cnt;
    }

    void allocate(Process& pro)
    {
        assert(this->freePages() >= 4);

        int pos = 0, left = 4;
        vector<int> temp;
        while (left > 0)
        {
            if (! this->pages[pos].used)
            {
                -- left;
                alloca_deque.push_back(&this->pages[pos]);
                this->pages[pos].used = true;
                this->pages[pos].used_times = 0;
                this->pages[pos].last_visited = 0; //
                temp.push_back(pos);
            }
            ++ pos;
        }
        for (int i=0; i!=4; ++i)
            pro.ownedPages[i] = this->pages[temp[i]].number;
        this->running_process.emplace_back(pro);
        cout << ticks << "\t" << pro.name << "\t" << "进入" << "\t" << pro.size
        << "\t" << pro.server_time << "\t";
        for (auto val : temp)
            cout << val << ' ';
        cout << endl;
    }

    void release(Process& pro)
    {
        for (auto pageNumber : pro.ownedPages)
            if (pageNumber != -1)
                this->pages[pageNumber].used = false;
        cout << ticks << "\t" << pro.name << "\t" << "退出" << "\t" << pro.size
             << "\t" << pro.server_time << "\t";
        cout << endl;
    }

    void check()
    {
        for (auto pro=running_process.begin(); pro!=running_process.end(); )
        {
            pro->server_time --;
            if (pro->server_time == 0)
            {
                this->release(*pro);
                pro = this->running_process.erase(pro);
            }
            else
            {
                ++ pro;
            }
        }
    }

    void allocate_one(int process_index, int reference)
    {
        for (auto& page : pages)
        {
            if (! page.used)
            {
                page.used = true;
                running_process[process_index].ownedPages[reference] = page.number;
                return;
            }
        }
    }

    vector<Process>& get_running_process()
    {
        return this->running_process;
    }

    bool finish()
    {
        return pages.empty();
    }

    void update_visit_time(int index, int time_add)
    {
        pages[index].last_visited = ticks * 1000 + time_add;
    }

    void update_used_times(int index)
    {
        pages[index].last_visited ++;
    }

public: // 置换算法
    void fifo_allocate_page(int process_index, int reference)
    {
        auto page = alloca_deque.front();
        for (auto& pro : running_process)
            for (auto& val : pro.ownedPages)
                if (val == page->number)
                    val = -1;
        running_process[process_index].ownedPages[reference] = page->number;

        alloca_deque.pop_front();
        alloca_deque.push_back(page);
    }

    void lru_allocate_page(int process_index, int reference, int time_add) // 时间修正， 添加了毫秒部分
    {
        auto page = alloca_deque.front();
        for (auto p=++alloca_deque.begin(); p!=alloca_deque.end(); ++p)
            if ((ticks * 1000 + time_add - (*p)->last_visited) > (ticks * 1000 + time_add - page->last_visited))
                page = *p;
        for (auto& pro : running_process)
        {
            for (int i=0; i!=pro.size; ++i)
            {
                if (pro.ownedPages[i] == page->number)
                    pro.ownedPages[i] = -1;
            }
        }
        running_process[process_index].ownedPages[reference] = page->number;

        stack<Page*> temp;
        while (! alloca_deque.empty() && alloca_deque.front() != page)
        {
            temp.push(alloca_deque.front());
            alloca_deque.pop_front();
        }
        if (! alloca_deque.empty())
            alloca_deque.pop_front();
        while (! temp.empty())
        {
            alloca_deque.push_front(temp.top());
            temp.pop();
        }

        alloca_deque.push_back(page);
    }

    void lfu_allocate_page(int process_index, int reference)
    {
        auto page = alloca_deque.front();
        for (auto p=++alloca_deque.begin(); p!=alloca_deque.end(); ++p)
            if ((*p)->used_times > page->used_times)
                page = *p;
        for (auto& pro : running_process)
        {
            for (int i=0; i!=pro.size; ++i)
            {
                if (pro.ownedPages[i] == page->number)
                    pro.ownedPages[i] = -1;
            }
        }
        running_process[process_index].ownedPages[reference] = page->number;

        stack<Page*> temp;
        while (! alloca_deque.empty() && alloca_deque.front() != page)
        {
            temp.push(alloca_deque.front());
            alloca_deque.pop_front();
        }
        if (! alloca_deque.empty())
            alloca_deque.pop_front();
        while (! temp.empty())
        {
            alloca_deque.push_front(temp.top());
            temp.pop();
        }

        alloca_deque.push_back(page);
    }
    
private:
    vector<Page> pages;
    deque<Page*> alloca_deque;
    vector<Process> running_process;
};

list<Process> ready;
void iniProcessList(list<Process>& cur, int size)
{
    cur.clear();
    for (int i=0; i!=size; ++i)
        cur.emplace_back("process" + to_string(i));
}

int nextPageNumber()
{
    int flag = rand() % 11;
    if (flag < 7)
        return rand() % 3 - 1;
    else
        return ((rand() % 2 == 1) ? -1 : 1) * (rand() % 8 + 2);
}

#define LFU

string test() {
    // 生成150个初始进程
    iniProcessList(ready, 150);
    // 闲置页表列表
    PageManager pm(PageMaxSize);

    // 选择作业
    int all_cnt = 0, correct_cnt = 0;
    while (ticks < 60 && ! ready.empty() && ! pm.finish())
    {
        while (pm.freePages() >= 4 && ! ready.empty())
        {
            pm.allocate(ready.front());
            ready.pop_front();
        }

        // 每秒十次
        vector<Process>& running = pm.get_running_process();
        for (int i=0; i!=10; ++i)
        {
            for (int j=0; j!=running.size(); ++j)
            {
                Process& pro = running[j];
                int size = pro.size;
                int reference_pos = (nextPageNumber() + pro.old_reference + size * (int)1e4) % size;
                if (pro.ownedPages[reference_pos] != -1)
                {
                    ++ correct_cnt;
                }
                else // 选定算法用于分配页
                {
                    if (pm.freePages() > 0)
                        pm.allocate_one(j, reference_pos);
                    else
                    {
                    # ifdef FIFO
                        pm.fifo_allocate_page(j, reference_pos);
                    #elif defined LRU
                        pm.lru_allocate_page(j, reference_pos, i);
                    #elif defined LFU
                        pm.lfu_allocate_page(j, reference_pos);
                    #endif
                    }
                }
                pm.update_used_times(pro.ownedPages[reference_pos]);
                pm.update_visit_time(pro.ownedPages[reference_pos], i);
                ++ all_cnt;
                pro.old_reference = reference_pos;
            }
        }

        pm.check();
        ++ ticks;
    }

    cout << "命中率: " << (double) correct_cnt / all_cnt << endl;
    return to_string((double) correct_cnt / all_cnt);
}

int main()
{
    srand(time(nullptr));
# ifdef FIFO
    ofstream outFile("fifo.txt");
#elif defined LRU
    ofstream outFile("lcu.txt");
#elif defined LFU
    ofstream outFile("lfu.txt");
#endif
    for (int i=0; i!=5; ++i)
    {
        ticks = 0;
        outFile << (test() + "\n");
    }

    return 0;
}