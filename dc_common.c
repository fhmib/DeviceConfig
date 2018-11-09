#include "dc_common.h"

int gen_json(const char *path, node_s *root)
{
    int rval = 0;
    FILE *fp = NULL;
    char buf[512];
    int i;

    fp = fopen(path, "w");
    if(fp == NULL){
        perror("gen_json: open file failed");
        rval = 1;
        goto func_exit;
    }
    i = 0;
    buf[0] = 0;

    first_tree(fp, 0, root);

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

            p = &pnode->child_h;
            while(p->next != NULL){
                p = p->next;
                first_tree(fp, level + 1, p->pnode);
                if(p->next != NULL){
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

            p = &pnode->child_h;
            while(p->next != NULL){
                p = p->next;
                first_tree(fp, level + 1, p->pnode);
                if(p->next != NULL){
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

            p = &pnode->child_h;
            while(p->next != NULL){
                p = p->next;
                first_tree(fp, level + 1, p->pnode);
                if(p->next != NULL){
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
            if(pnode->pvalue != NULL)
                strcat(buf, pnode->pvalue);
            strcat(buf, "\"");
            fwrite(buf, strlen(buf), 1, fp);

            //printf("level = %d, type = %d\n", level, pnode->type);
            break;

        case JSON_CUSTOM1:
            strcat(buf, "[");
            if(pnode->pvalue != NULL)
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

node_s *create_node(int type, const char *pname, const char *pvalue)
{
    node_s *node = (node_s*)malloc(sizeof(node_s));

    node->type = (char)type;
    node->child_h.next = node->child_t.next = NULL;

    switch(node->type){
        case JSON_BRACKET:
            node->pvalue = NULL;
            return node;

        case JSON_NORMAL:
        case JSON_ARRAY:
            if(strlen(pname) > NAME_LEN-1){
                printf("name is too long\n");
                free(node);
                return NULL;
            }
            strcpy(node->name, pname);
            node->pvalue = NULL;
            return node;

        case JSON_STRING:
            if((strlen(pname) > NAME_LEN-1) || (strlen(pname) < 1)){
                printf("name is invalid\n");
                free(node);
                return NULL;
            }
            strcpy(node->name, pname);
            if(pvalue != NULL){
                node->pvalue = (char*)malloc(strlen(pvalue)+1);
                strcpy(node->pvalue, pvalue);
            }
            return node;

        case JSON_CUSTOM1:
            if(pvalue != NULL){
                node->pvalue = (char*)malloc(strlen(pvalue)+1);
                strcpy(node->pvalue, pvalue);
            }
            return node;

        default:
            printf("no such type\n");
            break;
    }

    if(node != NULL){
        free(node);
        node = NULL;
    }
    return NULL;
}

void insert_node(node_s *dst, node_s *src)
{
    node_l *p = &dst->child_t;

    if(dst->child_t.next == NULL){
        p = &dst->child_t;
    }else{
        p = dst->child_t.next;
    }
    assert(p->next == NULL);

    p->next = (node_l*)malloc(sizeof(node_l));
    p = p->next;
    p->pnode = src;
    p->next = NULL;
    dst->child_t.next = p;
    if(dst->child_h.next == NULL){
        dst->child_h.next = p;
    }

    return ;
}









