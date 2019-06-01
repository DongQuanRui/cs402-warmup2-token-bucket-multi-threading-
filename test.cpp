#include <sys/time.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[])
{
    FILE *fp;
    int a, b, c;
    int num;
    fp = fopen("f0.txt", "r");
    fscanf(fp, "%d", &num);
    printf("num = %d\n", num);
    while (!feof(fp)) {
        fscanf(fp, "%d %d %d", &a, &b, &c);
        printf("a = %d, b = %d, c= %d\n", a, b, c);
    }
    return 0;
}
