#include "dc_common.h"

int gen_json(const char *path)
{
    int rval = 0;
    FILE *fp = NULL;
    char buf[512];
    int i;
    node_s root, n1, n2, n3, n4, n5, n6, n7, n8;
    node_s m1, m2, m3, m4, m5;
    node_s s1, s2, s3;

    fp = fopen(path, "w");
    if(fp == NULL){
        perror("gen_json: open file failed");
        rval = 1;
        goto func_exit;
    }
    i = 0;
    buf[0] = 0;

    root.type = JSON_BRACKET;
    n1.type = JSON_NORMAL;
    n2.type = JSON_NORMAL;
    n3.type = JSON_NORMAL;
    n4.type = JSON_STRING;
    n5.type = JSON_STRING;
    n6.type = JSON_STRING;
    n7.type = JSON_STRING;
    n8.type = JSON_ARRAY;

    root.child_h = (node_l*)malloc(sizeof(node_l));
    root.child_h->pnode = &n1;
    root.child_h->next = (node_l*)malloc(sizeof(node_l));
    root.child_h->next->pnode = &n2;
    root.child_h->next->next = (node_l*)malloc(sizeof(node_l));
    root.child_h->next->next->pnode = &s1;
    root.child_h->next->next->next = NULL;
    root.child_t = root.child_h->next->next;

    strcpy(n1.name, "flags");
    n1.child_h = (node_l*)malloc(sizeof(node_l));
    n1.child_h->pnode = &n3;
    n1.child_h->next = (node_l*)malloc(sizeof(node_l));
    n1.child_h->next->pnode = &n4;
    n1.child_h->next->next = NULL;
    n1.child_t = n1.child_h->next;

    strcpy(n2.name, "status");
    n2.child_h = (node_l*)malloc(sizeof(node_l));
    n2.child_h->pnode = &n5;
    n2.child_h->next = (node_l*)malloc(sizeof(node_l));
    n2.child_h->next->pnode = &n8;
    n2.child_h->next->next = NULL;
    n2.child_t = n2.child_h->next;

    strcpy(n3.name, "online");
    n3.child_h = (node_l*)malloc(sizeof(node_l));
    n3.child_h->pnode = &n6;
    n3.child_h->next = (node_l*)malloc(sizeof(node_l));
    n3.child_h->next->pnode = &n7;
    n3.child_h->next->next = NULL;
    n3.child_t = n3.child_h->next;

    strcpy(n4.name, "default");
    n4.length = 2;
    n4.pvalue = (char*)malloc(n4.length);
    strcpy(n4.pvalue, "0");
    n4.child_t = n4.child_h = NULL;

    strcpy(n5.name, "test3");
    n5.length = 3;
    n5.pvalue = (char*)malloc(n5.length);
    strcpy(n5.pvalue, "15");
    n5.child_t = n5.child_h = NULL;

    strcpy(n6.name, "test1");
    n6.length = 2;
    n6.pvalue = (char*)malloc(n6.length);
    strcpy(n6.pvalue, "1");
    n6.child_t = n6.child_h = NULL;

    strcpy(n7.name, "test2");
    n7.length = 2;
    n7.pvalue = (char*)malloc(n7.length);
    strcpy(n7.pvalue, "1");
    n7.child_t = n7.child_h = NULL;

    strcpy(n8.name, "remoteStatus");
    n8.child_h = (node_l*)malloc(sizeof(node_l));
    n8.child_h->pnode = &m1;
    n8.child_h->next = (node_l*)malloc(sizeof(node_l));
    n8.child_h->next->pnode = &m2;
    n8.child_h->next->next = NULL;
    n8.child_t =n8.child_h->next;

    m1.type = JSON_BRACKET;
    m2.type = JSON_BRACKET;
    m3.type = JSON_STRING;
    m4.type = JSON_STRING;
    m5.type = JSON_STRING;

    m1.child_h = (node_l*)malloc(sizeof(node_l));
    m1.child_h->pnode = &m3;
    m1.child_h->next = (node_l*)malloc(sizeof(node_l));
    m1.child_h->next->pnode = &m4;
    m1.child_h->next->next = NULL;
    m1.child_t = m1.child_h->next;

    m2.child_h = (node_l*)malloc(sizeof(node_l));
    m2.child_h->pnode = &m5;
    m2.child_h->next = NULL;
    m2.child_t = m2.child_h;

    strcpy(m3.name, "test4");
    m3.length = 2;
    m3.pvalue = (char*)malloc(m3.length);
    strcpy(m3.pvalue, "1");
    m3.child_t = m3.child_h = NULL;

    strcpy(m4.name, "test5");
    m4.length = 2;
    m4.pvalue = (char*)malloc(m4.length);
    strcpy(m4.pvalue, "1");
    m4.child_t = m4.child_h = NULL;

    strcpy(m5.name, "test6");
    m5.length = 2;
    m5.pvalue = (char*)malloc(m5.length);
    strcpy(m5.pvalue, "1");
    m5.child_t = m5.child_h = NULL;

    s1.type = JSON_ARRAY;
    s2.type = JSON_CUSTOM1;
    s3.type = JSON_CUSTOM1;

    strcpy(s1.name, "sigQualityTable");
    s1.child_h = (node_l*)malloc(sizeof(node_l));
    s1.child_h->pnode = &s2;
    s1.child_h->next = (node_l*)malloc(sizeof(node_l));
    s1.child_h->next->pnode = &s3;
    s1.child_h->next->next = NULL;
    s1.child_t =s1.child_h->next;

    s2.length = 256;
    s2.pvalue = (char*)malloc(s2.length);
    strcpy(s2.pvalue, "1, 123, 123, 123, 123, 123, 123");
    s2.child_t = s2.child_h = NULL;

    s3.length = 256;
    s3.pvalue = (char*)malloc(s3.length);
    strcpy(s3.pvalue, "142, 22, 2, 213, 213, 232, 233");
    s3.child_t = s3.child_h = NULL;

    first_tree(fp, 0, &root);

func_exit:

    if(fp != NULL){
        fclose(fp);
        fp = NULL;
    }

    return rval;
}

void first_tree(FILE *fp, int level, node_s *pnode)
{
    node_l *p = NULL;
    int i;
    char buf[512];

    buf[0] = 0;
    for(i = level; i > 0; i--){
        strcat(buf, "\t");
    }

    switch(pnode->type){
        case JSON_BRACKET:
            strcat(buf, "{\n");
            fwrite(buf, strlen(buf), 1, fp);

            p = pnode->child_h;
            while(p != NULL){
                first_tree(fp, level + 1, p->pnode);
                p = p->next;
                if(p != NULL){
                    strcpy(buf, ",\n");
                    fwrite(buf, strlen(buf), 1, fp);
                }else{
                    strcpy(buf, "\n");
                    fwrite(buf, strlen(buf), 1, fp);
                }
            }

            buf[0] = 0;
            for(i = level; i > 0; i--){
                strcat(buf, "\t");
            }
            strcat(buf, "}");
            fwrite(buf, strlen(buf), 1, fp);

            //printf("level = %d, type = %d\n", level, pnode->type);
            break;

        case JSON_NORMAL:
            strcat(buf, "\"");
            strcat(buf, pnode->name);
            strcat(buf, "\": {\n");
            fwrite(buf, strlen(buf), 1, fp);

            p = pnode->child_h;
            while(p != NULL){
                first_tree(fp, level + 1, p->pnode);
                p = p->next;
                if(p != NULL){
                    strcpy(buf, ",\n");
                    fwrite(buf, strlen(buf), 1, fp);
                }else{
                    strcpy(buf, "\n");
                    fwrite(buf, strlen(buf), 1, fp);
                }
            }

            buf[0] = 0;
            for(i = level; i > 0; i--){
                strcat(buf, "\t");
            }
            strcat(buf, "}");
            fwrite(buf, strlen(buf), 1, fp);

            //printf("level = %d, type = %d\n", level, pnode->type);
            break;

        case JSON_ARRAY:
            strcat(buf, "\"");
            strcat(buf, pnode->name);
            strcat(buf, "\": [\n");
            fwrite(buf, strlen(buf), 1, fp);

            p = pnode->child_h;
            while(p != NULL){
                first_tree(fp, level + 1, p->pnode);
                p = p->next;
                if(p != NULL){
                    strcpy(buf, ",\n");
                    fwrite(buf, strlen(buf), 1, fp);
                }else{
                    strcpy(buf, "\n");
                    fwrite(buf, strlen(buf), 1, fp);
                }
            }

            buf[0] = 0;
            for(i = level; i > 0; i--){
                strcat(buf, "\t");
            }
            strcat(buf, "]");
            fwrite(buf, strlen(buf), 1, fp);

            //printf("level = %d, type = %d\n", level, pnode->type);
            break;

        case JSON_STRING:
            strcat(buf, "\"");
            strcat(buf, pnode->name);
            strcat(buf, "\": ");
            strcat(buf, "\"");
            strcat(buf, pnode->pvalue);
            strcat(buf, "\"");
            fwrite(buf, strlen(buf), 1, fp);

            //printf("level = %d, type = %d\n", level, pnode->type);
            break;

        case JSON_CUSTOM1:
            strcat(buf, "[");
            strcat(buf, pnode->pvalue);
            strcat(buf, "]");
            fwrite(buf, strlen(buf), 1, fp);

            //printf("level = %d, type = %d\n", level, pnode->type);
            break;

        default:
            printf("level = %d\n", level);
            break;
    }

    return ;
}










