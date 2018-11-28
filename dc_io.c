#include "dc_io.h"

extern sdata_s status_data[];

#if 1
//for test
char *test1(int index)
{
    sdata_s *psd = &status_data[index];

    //fprintf(stderr, "I'm in %s, i = %d, name = %s\n", __func__, index, psd->name);
    if(psd->pvalue == NULL){
        psd->pvalue = (char*)malloc(16);
        sprintf(psd->pvalue, "hello test1");
        //printf("%s\n", psd->pvalue);
    }

    return NULL;
}

char *test2(int index)
{
    sdata_s *psd = &status_data[index];

    //fprintf(stderr, "I'm in %s, i = %d, name = %s\n", __func__, index, psd->name);
    if(psd->pvalue == NULL){
        psd->pvalue = (char*)malloc(16);
        sprintf(psd->pvalue, "hello test2");
        //printf("%s\n", psd->pvalue);
    }

    return NULL;
}
#endif


