#include <ctype.h>

// 該当文字列str, 配列vec, 配列の最大格納数max
int tokenize (char *str, char **vec, int max) {
    int num, skip;
    if (max < 1 || vec == 0) {
        skip = 1;
    } else {
        skip = 0;
    }
    num = 0;
    while (1) {
        if (!skip && num >= max-1){
            vec[num] = 0;
            skip = 1; 
        }
        // 単語先頭を探す
        while (*str != '\0' && isspace(*str)) str++;
        if (*str == '\0') break;
        if (!skip) vec[num] = str;
        num++;
        // 単語末尾を探す
        while (*str != '\0' && !isspace(*str)) str++;
        if (*str = '\0') break;
        *str = '\0';
        str++;
    }
    if (!skip) vec[num] = 0;
    num++;
    return num;
}