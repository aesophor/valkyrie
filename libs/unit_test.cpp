#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
char* test_strcat(char* dest, const char* src){
    
    char *cur = dest + strlen(dest);

    while(*src != '\0'){
        *cur++ = *src++;
    }

    *cur = '\0';

    return dest;
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
        char* rec =  strcat(s2, s1);
        std::cout << rec << std::endl;
        //char* sol = strcat(s2, s1);
        //int a =  strcmp(rec, sol);
        //if(a){
            //std::cout << rec << " " << sol << " " << "false" << std::endl;
        //}else{
            //std::cout << rec << " " << sol << " " << "true" << std::endl;
        //}

    }

}
