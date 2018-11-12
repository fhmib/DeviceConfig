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

/*
 * func:
 *      delete a child node by name
 * ret:
 *      0:              success
 *      other:          failure
 */
int del_node(node_s *node, char *name)
{
    int rval = 1;
    node_l *p, *temp;

    p = &node->child_h;
    while(p->next != NULL){
        if((0 == strcmp(p->next->pnode->name, name))){
            temp = p->next;
            p->next = temp->next;
            free(temp);
            temp = NULL;
            rval = 0;
            goto func_exit;
        }else{
            p = p->next;
        }
    }

func_exit:
    return rval;
}

/*
 * func:
 *      modify value according to the name
 */
void mod_node(node_s *node, const char *pvalue)
{
    int len;

    len = strlen(pvalue);
    if(len > (int)strlen(node->pvalue)){
        free(node->pvalue);
        node->pvalue = (char*)malloc(len + 1);
        strcpy(node->pvalue, pvalue);
    }else{
        strcpy(node->pvalue, pvalue);
    }

    return ;
}

/*
 * func:
 *      search a node by name use Pre-order
 * ret:
 *      NULL:           failure
 *      other:          pointer to node
 */
node_s *search_node(node_s *node, const char *name)
{
    node_l *p = NULL;
    node_s *res;

    if(0 == (strcmp(node->name, name))){
        return node;
    }else{
        p = &node->child_h;
        while(p->next != NULL){
            res = search_node(p->next->pnode, name);
            if(res == NULL){
                p = p->next;
            }else{
                return res;
            }
        }
    }

    return NULL;
}

/*
 * func:
 *      read paramters from appointed file
 *      it also can match specific parameter and return it
 * ret:
 *      pointer to the node of head or specific parameter, NULL means cannot find out the parameter
 */
rdata_s *read_json(const char *path, const char *option)
{
    FILE *fp;
    char buf[256];
    char *name, *value;
    rdata_s *h = NULL;
    rdata_s *t = NULL;

    fp = fopen(path, "r");
    while(NULL != fgets(buf, 256, fp)){
        name = strtok(buf, "{[\t\n \"");
        if(name == NULL) continue;
        if(strlen(name) >= 64){
            printf("name is too long\n");
            continue;
        }

        if((value = strtok(NULL, "{[\t\n \"")) == NULL) continue;
        if(0 != strcmp(value, ":")) continue;

        value = strtok(NULL, "{[\t\n \"");
        if(value == NULL) continue;

        //printf("%s: %s\n", name, value);

        if(option == NULL){
            if(h == NULL){
                h = t = (rdata_s*)malloc(sizeof(rdata_s));
            }else{
                t->next = (rdata_s*)malloc(sizeof(rdata_s));
                t = t->next;
            }
            strcpy(t->name, name);
            t->pvalue = (char*)malloc(strlen(value) + 1);
            strcpy(t->pvalue, value);
            t->next = NULL;
        }else{
            if(0 == strcmp(option, name)){
                h = (rdata_s*)malloc(sizeof(rdata_s));
                strcpy(h->name, name);
                h->pvalue = (char*)malloc(strlen(value) + 1);
                strcpy(h->pvalue, value);
                h->next = NULL;
                goto func_exit;
            }else{
                continue;
            }
        }
    }

func_exit:
    if(fp != NULL) fclose(fp);

    return h;
}






