#include <iostream>
using namespace std;

void fun(int **a) {
    *a = new int(11);
    return;
}
int main() {
    int *a = nullptr;
    fun(&a);
    cout << *a << endl;
    return 0;
}