
int a(int arg1, int arg2) {
    int u = 200, v = u;
    return v / 2 / 10;
}

char returnChar() {
    return 'b';
}

int main(void) {
    char c = 'c';
    if(10<20)
    {
        printInt(111);
    }
    printInt(20-10*2);
    printInt(20/10);
    printInt(1 * 10 * 100);
    printInt(a(2, 20));
    printChar('a');
    printChar(returnChar());
    printChar(c);
    return 0;
}