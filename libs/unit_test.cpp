#include <stdio.h>
#include <string.h>
#include <iostream>
int test_strncmp(const char* s1, const char* s2, size_t n){
   if(n == 0)
       return 0;

    while(n--){
        if(*s1 != *s2)
            return *s1 - *s2;
        else{
            s1 ++;
            s2 ++;
        }
    }
    return 0;
}
int main(){
    char s1[100];
    char s2[100];
    int n;
    while(true){
        scanf("%s %s %d", s1, s2, &n);
        int a =  strncmp(s1, s2, n);
        int b =  test_strncmp(s1, s2, n);
        if(a != b){
            std::cout << a << " " << b << "false" << std::endl;
        }else{
            std::cout << a << " " << b << "true" << std::endl;
        }

    }

}
