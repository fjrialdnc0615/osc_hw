int str_compare(const char* a, const char* b){
        int sentinel_value = 0;
        while(*a != '\0' || *b != '\0')
        {
                if(*a=='\n'){
                        a++;
                        continue;
                }
                if(*a==*b) sentinel_value = 1;
                else return 0;
                a++;
                b++;
        }
        return sentinel_value;
}

