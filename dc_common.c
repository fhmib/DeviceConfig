#include "dc_common.h"

extern node_s *sdeveloper;

int gen_json(const char *path, node_s *root)
{
    int rval = 0;
    FILE *fp = NULL;
    int i;

    fp = fopen(path, "w");
    if(fp == NULL){
        perror("gen_json: open file failed");
        rval = 1;
        goto func_exit;
    }
    i = 0;

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

    //important
    memset(node, 0, sizeof(node_s));

    node->type = (char)type;
    node->child_h.next = node->child_t.next = NULL;

    switch(node->type){
        case JSON_BRACKET:
            node->pvalue = NULL;
            return node;

        case JSON_NORMAL:
        case JSON_ARRAY:
            if(strlen(pname) > NAME_LEN-1){
                fprintf(stderr, "%s:name is too long\n", __func__);
                free(node);
                return NULL;
            }
            strcpy(node->name, pname);
            node->pvalue = NULL;
            return node;

        case JSON_STRING:
            if((strlen(pname) > NAME_LEN-1) || (strlen(pname) < 1)){
                fprintf(stderr, "%s:name is invalid, len = %ld\n", __func__, strlen(pname));
                free(node);
                return NULL;
            }
            strcpy(node->name, pname);
            if(pvalue != NULL){
                node->pvalue = (char*)malloc(strlen(pvalue)+1);
                strcpy(node->pvalue, pvalue);
            }else{
                node->pvalue = NULL;
            }
            return node;

        case JSON_CUSTOM1:
            if(pvalue != NULL){
                node->pvalue = (char*)malloc(strlen(pvalue)+1);
                strcpy(node->pvalue, pvalue);
            }else{
                node->pvalue = NULL;
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
 *      delete a node and all childs belong to it, use it carefully, the use-method is refer to del_node() function.
 */
void _del_node(node_s *node)
{
    node_l *p;
    node_l *temp;

    p = &node->child_h;
    if(p->next != NULL){
        p = p->next;
        while(p != NULL){
            _del_node(p->pnode);
            temp = p;
            p = p->next;
            free(temp);
        }
    }

#if 0
    fprintf(stderr, "%s is freeing %s\n", __func__, node->name);
#endif
    if(node->pvalue != NULL){
        free(node->pvalue);
        node->pvalue = NULL;
    }
    free(node);

    return ;
}

/*
 * func:
 *      delete a node by name, the funcion will delete node and all its childs
 * ret:
 *      0:              success
 *      other:          failure
 */
int del_node(node_s *node, const char *name)
{
    int rval = 1;
    node_l *p, *temp;

    p = &node->child_h;
    while(p->next != NULL){
        if((0 == strcmp(p->next->pnode->name, name))){
            temp = p->next;
            p->next = temp->next;

            //if the deleted node is the last one, It's very important to change chilt_t to the previous one
            if(temp->next == NULL) node->child_t.next = p;

            _del_node(temp->pnode);
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
 *      remove all childs of a node
 */
int remove_childs(node_s *node){
    int rval = 0;
    node_l *p, *temp;

    p = node->child_h.next;

    while(p != NULL){
        temp = p;
        p = p->next;
        _del_node(temp->pnode);
        free(temp);
    }

    node->child_h.next = NULL;
    node->child_t.next = NULL;

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

    if(pvalue == NULL) return ;
    len = strlen(pvalue);
    if(node->pvalue == NULL){
        node->pvalue = (char*)malloc(len + 1);
    }else{
        if(len > (int)strlen(node->pvalue)){
            free(node->pvalue);
            node->pvalue = (char*)malloc(len + 1);
        }
    }
    strcpy(node->pvalue, pvalue);

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

    //printf("node->name=[%s], name=[%s]\n", node->name, name);
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
    if(fp == NULL){
        perror("read_json: open file failed");
        goto func_exit;
    }
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

void free_rdata(rdata_s *h)
{
    rdata_s *temp;

    while(h != NULL){
        temp = h;
        h = h->next;
        //printf("free %s success\n", temp->name);
        free(temp->pvalue);
        free(temp);
    }

    return ;
}

/*
 * func:
 *      modify the value of 'pvalue' according to 'value'
 */
void modify_value(char **pvalue, const char *value)
{
    unsigned int len;

    if(value == NULL){
        if(*pvalue != NULL){
            free(*pvalue);
            *pvalue = NULL;
        }
    }else{
        len = strlen(value);

        if(*pvalue == NULL){
            *pvalue = (char*)malloc(len+1);
        }else{
            if(strlen(*pvalue) < len){
                free(*pvalue);
                *pvalue = (char*)malloc(len+1);
            }
        }

        strcpy(*pvalue, value);
    }

    return ;
}

/*
 * func:
 *      read ip from string
 * params:
 *      buf:            ip string
 *      ip:             an 'int' type array to store ipaddress
 */
int read_ipaddr(const char *buf, int *ip)
{
    int rval = 0;

    sscanf(buf, "%u.%u.%u.%d", &ip[0], &ip[1], &ip[2], &ip[3]);

    return rval;
}

/*
 * func:
 *      judge whether ip is NULL
 */
int ipisnull(const char *buf)
{
    int ip[4];

    read_ipaddr(buf, ip);
    if((ip[0] == 0) && (ip[1] == 0) && (ip[2] == 0) && (ip[3] == 0)){
        return 1;
    }else return 0;

    return 0;
}

/*
 * func:
 *      judge whether ip is host
 */
int ipishost(const char *buf)
{
    int ip[4];

    read_ipaddr(buf, ip);
    if(ip[3] > 0){
        return 1;
    }else return 0;

    return 0;
}
