#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

int compare(const char* x, const char* y){
    while(*x && *y){
        if(*x != *y)
            return 0;
        x++;
        y++;
    }
    return (*y == '\0');
}
char* test_strstr(const char* haystack, const char* needle){
    while(*haystack != '\0'){
        if(*haystack == *needle && compare(haystack, needle))
            return ((char*)haystack);
        haystack ++;
    }
    return NULL;
}
int main(){
    //char *s1 = (char*)malloc(100);
    //char *s2 = (char*)malloc(100);
    char s1[100];
    char s2[100];
    int n;
    while(true){
        //scanf("%s %s %d", s1, s2, &n);
        scanf("%s %s", s1, s2);
        std::cout << s1 << " " << s2 << std::endl;
        char* rec = test_strstr(s1, s2);
        if(rec)
            std::cout << rec << std::endl;
        char* sol = strstr(s1,s2);
        std:: cout << sol << std::endl;
        int a =  strcmp(rec, sol);
        if(a){
            std::cout << rec << " " << sol << " " << "false" << std::endl;
        }else{
            std::cout << rec << " " << sol << " " << "true" << std::endl;
        }

    }
}
