#include <iostream>
#define MODN 2147483648

using namespace std;

int main()
{
    while(true)
    {
        long long a, b, c;
        char iterator_c;

        cin >> a >> iterator_c >> b;

        switch(iterator_c) 
        {
            case '+':
                c = a + b;
                break;
            case '-':
                c = a - b;
                break;
            case '*':
                c = a * b;
                break;
            case '/':
                c = a / b;
                break;
        }
        cout << c % MODN << endl;
        
    }
    
    return 0;
}
