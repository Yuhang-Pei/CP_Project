int a(int arg1, int arg2) {
    int u = 200, v = u;
    return v + 2 + arg2;
}

int main(void) {
    char c = 'c';
    printInt(1 + 10 + 100);
    printInt(a(2, 20));
    printChar('a');
    printChar(c);

    return 0;
}