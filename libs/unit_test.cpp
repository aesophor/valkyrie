#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
char* test_strncpy(char* dest, const char* src, size_t n){
    if(dest == NULL)
        return NULL;

    char *ret = dest;

    while(*src != '\0' && n --){
        *dest++ = *src++;
    }

    *dest = '\0';

    return ret;
}
int main(){
    //char *s1 = (char*)malloc(100);
    //char *s2 = (char*)malloc(100);
    char s1[100];
    char s2[100];
    int n;
    while(true){
        //scanf("%s %s %d", s1, s2, &n);
        scanf("%s %d", s1, &n);
        char* rec =  test_strncpy(s2, s1, n);
        int a =  strncmp(s1, rec, n);
        if(a){
            std::cout << s1 << " " << s2 << " " << "false" << std::endl;
        }else{
            std::cout << s1 << " " << s2 << " " << "true" << std::endl;
        }

    }

}
