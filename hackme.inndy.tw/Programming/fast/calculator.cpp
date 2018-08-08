#include <iostream>
#define maxn 1000

using namespace std;

int nc = 0;     //節點數

struct node_type  //0可能將來會當作null
{
    int type = 0;   //此節點的資料型別 1 = char 2 = number
    int lch;   //左子節點
    int rch;   //右子節點
    char op_char;
    int op_number;
    
};

node_type node[maxn];

int string_to_int(string s,int x,int y)
{
    int n = y - x;
    int total = 0;
    for(int i = n;i > 0;i--)
    {
        int temp = s[x - i + n] - '0';
        for(int j = 1;j < i;j++)
        {
            temp *= 10;
        }
        total += temp;
    }
    return total;
}

int build_tree(string s, int x, int y)
{
    int c1 = -1, c2 = -1, p = 0;
    int u;                          //自身節點編號
    bool only_number = 1;

    for(int i = x;i < y;i++)
    {
        if(!('0' <= s[i] && s[i] <= '9'))
            only_number = 0;

        switch(s[i])
        {
            case '(': 
                p++;
                break;
            case ')':
                p--;
                break;
            case '+': case '-':         
                if(!p)                  //如果不在括號內
                    c1 = i;             //最後一個加或減字元位置
                break;
            case '*': case '/':
                if(!p)                  
                    c2 = i;             //最後一個乘或除字元位置
                break;
        }
    }

    if(y - x == 1 || only_number)      //僅一個字元 or 只有數字 建立單獨節點
    {
        u = ++nc;
        node[u].lch = 0;     //沒有子節點
        node[u].rch = 0;     //沒有子節點
        if(only_number)
        {
            node[u].type = 2;
            node[u].op_number = string_to_int(s, x, y); 
        }
        else
        {
            node[u].type = 1;
            node[u].op_char = s[x];
        }
        return u;
    }

    if(c1 < 0)          //找不到括號外的加減號
    {
        c1 = c2;   
        if(c1 < 0)          //整個運算式最外為一對括號
            return build_tree(s, x + 1, y - 1);
    }
    u = ++nc;
    node[u].lch = build_tree(s, x, c1);
    node[u].rch = build_tree(s, c1 + 1, y);
    node[u].type = 1;
    node[u].op_char = s[c1];
    return u;
}

double solve_math(int u)
{
    if(node[u].type = 1)
    {
        switch(node[u].op_char)
        {
            case '+': 
                return solve_math(node[u].lch) + solve_math(node[u].rch);
            case '-': 
                return solve_math(node[u].lch) - solve_math(node[u].rch);
            case '*': 
                return solve_math(node[u].lch) * solve_math(node[u].rch);
            case '/': 
                return solve_math(node[u].lch) / solve_math(node[u].rch);
        }
    }

    return node[u].op_number;

}

int main()
{
    string s;
    getline(cin, s);
    build_tree(s, 0, s.length());
    /*
    for(int i = 1;i < 12;i++)
    {
        if(node[i].type == 1)
            cout << node[i].op_char << " ";
        else
            cout << node[i].op_number << " ";
    }
    */
    
    cout << solve_math(1) << endl;


    return 0;
}
