#include "dc_common.h"

/*
 * func:
 *      generate json-file according to the tree
 */
int gen_json(const char* path, node_s* root)
{
    int rval = 0;
    FILE* fp = NULL;
    int i;

    fp = fopen(path, "w");
    if (fp == NULL) {
        perror("gen_json: open file failed");
        rval = 1;
        goto func_exit;
    }
    i = 0;

    first_tree(fp, 0, root);

func_exit:

    if (fp != NULL) {
        fclose(fp);
        fp = NULL;
    }

    return rval;
}

void first_tree(FILE* fp, int level, node_s* pnode)
{
    node_l* p = NULL;
    int i;
    char buf[512];

    buf[0] = 0;
    for (i = level; i > 0; i--) {
        strcat(buf, "\t");
    }

    switch (pnode->type) {
    case JSON_BRACKET:
        strcat(buf, "{\n");
        fwrite(buf, strlen(buf), 1, fp);

        p = &pnode->child_h;
        while (p->next != NULL) {
            p = p->next;
            first_tree(fp, level + 1, p->pnode);
            if (p->next != NULL) {
                strcpy(buf, ",\n");
                fwrite(buf, strlen(buf), 1, fp);
            } else {
                strcpy(buf, "\n");
                fwrite(buf, strlen(buf), 1, fp);
            }
        }

        buf[0] = 0;
        for (i = level; i > 0; i--) {
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
        while (p->next != NULL) {
            p = p->next;
            first_tree(fp, level + 1, p->pnode);
            if (p->next != NULL) {
                strcpy(buf, ",\n");
                fwrite(buf, strlen(buf), 1, fp);
            } else {
                strcpy(buf, "\n");
                fwrite(buf, strlen(buf), 1, fp);
            }
        }

        buf[0] = 0;
        for (i = level; i > 0; i--) {
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
        while (p->next != NULL) {
            p = p->next;
            first_tree(fp, level + 1, p->pnode);
            if (p->next != NULL) {
                strcpy(buf, ",\n");
                fwrite(buf, strlen(buf), 1, fp);
            } else {
                strcpy(buf, "\n");
                fwrite(buf, strlen(buf), 1, fp);
            }
        }

        buf[0] = 0;
        for (i = level; i > 0; i--) {
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

        if (pnode->isstr == 1) {
            strcat(buf, "\"");
        } else {
            strcat(buf, "");
        }

        if (pnode->pvalue != NULL) {
            strcat(buf, pnode->pvalue);
        } else {
            if (pnode->isstr != 1)
                strcat(buf, "\"\"");
        }

        if (pnode->isstr == 1) {
            strcat(buf, "\"");
        } else {
            strcat(buf, "");
        }
        fwrite(buf, strlen(buf), 1, fp);

        //printf("level = %d, type = %d\n", level, pnode->type);
        break;

    case JSON_CUSTOM1:
        strcat(buf, "[");
        if (pnode->pvalue != NULL)
            strcat(buf, pnode->pvalue);
        strcat(buf, "]");
        fwrite(buf, strlen(buf), 1, fp);

        //printf("level = %d, type = %d\n", level, pnode->type);
        break;

    default:
        printf("level = %d\n", level);
        break;
    }

    return;
}

/*
 * func:
 *      generate tree according to array of node_s type.
 */
int gen_tree(node_s* pn, sdata_s* pd, int cnt)
{
    int rval = 0;
    int i;
    node_s *node, *pfn;

    for (i = 0; i < cnt; i++) {
        if (strcmp(pd[i].fname, "null") == 0) {
            node = create_node(pd[i].type, pd[i].name, pd[i].pvalue, pd[i].isstr);
            if (node == NULL) {
                fprintf(stderr, "%s,%d:%s is set wrong\n", __func__, __LINE__, pd[i].name);
                rval = 1;
                goto func_exit;
            } else {
                insert_node(pn, node);
            }
        } else {
            pfn = search_node(pn, pd[i].fname);
            if (pfn == NULL) {
                fprintf(stderr, "%s's fname is set wrong, fname is '%s'\n", pd[i].name, pd[i].fname);
                rval = 2;
                goto func_exit;
            }
            node = create_node(pd[i].type, pd[i].name, pd[i].pvalue, pd[i].isstr);
            if (node == NULL) {
                fprintf(stderr, "%s,%d:%s is set wrong\n", __func__, __LINE__, pd[i].name);
                rval = 3;
                goto func_exit;
            } else {
                insert_node(pfn, node);
            }
        }
    }

func_exit:
    return rval;
}

/*
 * func:
 *      create a node.
 */
node_s* create_node(int type, const char* pname, const char* pvalue, char isstr)
{
    node_s* node = (node_s*)malloc(sizeof(node_s));

    //important
    memset(node, 0, sizeof(node_s));

    node->type = (char)type;
    node->isstr = isstr;
    node->child_h.next = node->child_t.next = NULL;

    switch (node->type) {
    case JSON_BRACKET:
        node->pvalue = NULL;
        return node;

    case JSON_NORMAL:
    case JSON_ARRAY:
        if (strlen(pname) > NAME_LEN - 1) {
            fprintf(stderr, "%s:name is too long\n", __func__);
            free(node);
            return NULL;
        }
        strcpy(node->name, pname);
        node->pvalue = NULL;
        return node;

    case JSON_STRING:
        if ((strlen(pname) > NAME_LEN - 1) || (strlen(pname) < 1)) {
            fprintf(stderr, "%s:name is invalid, len = %ld\n", __func__, strlen(pname));
            free(node);
            return NULL;
        }
        strcpy(node->name, pname);
        if (pvalue != NULL) {
            node->pvalue = (char*)malloc(strlen(pvalue) + 1);
            strcpy(node->pvalue, pvalue);
        } else {
            node->pvalue = NULL;
        }
        return node;

    case JSON_CUSTOM1:
        if (pvalue != NULL) {
            node->pvalue = (char*)malloc(strlen(pvalue) + 1);
            strcpy(node->pvalue, pvalue);
        } else {
            node->pvalue = NULL;
        }
        return node;

    default:
        printf("no such type\n");
        break;
    }

    if (node != NULL) {
        free(node);
        node = NULL;
    }
    return NULL;
}

/*
 * func:
 *      insert a node to the tree, the node 'src' is child of 'dst'.
 */
void insert_node(node_s* dst, node_s* src)
{
    node_l* p = &dst->child_t;

    if (dst->child_t.next == NULL) {
        p = &dst->child_t;
    } else {
        p = dst->child_t.next;
    }
    assert(p->next == NULL);

    p->next = (node_l*)malloc(sizeof(node_l));
    p = p->next;
    p->pnode = src;
    p->next = NULL;
    dst->child_t.next = p;
    if (dst->child_h.next == NULL) {
        dst->child_h.next = p;
    }

    return;
}

/*
 * func:
 *      delete a node and all childs belong to it, use it carefully because it doesn't care about its parent. The use-method is refer to del_node() function.
 */
void _del_node(node_s* node)
{
    node_l* p;
    node_l* temp;

    p = &node->child_h;
    if (p->next != NULL) {
        p = p->next;
        while (p != NULL) {
            _del_node(p->pnode);
            temp = p;
            p = p->next;
            free(temp);
        }
    }

#if 0
    fprintf(stderr, "%s is freeing %s\n", __func__, node->name);
#endif
    if (node->pvalue != NULL) {
        free(node->pvalue);
        node->pvalue = NULL;
    }
    free(node);

    return;
}

/*
 * func:
 *      delete a node by name, the funcion will delete node and all its childs
 * ret:
 *      0:              success
 *      other:          failure
 */
int del_node(node_s* node, const char* name)
{
    int rval = 1;
    node_l *p, *temp;

    p = &node->child_h;
    while (p->next != NULL) {
        if ((0 == strcmp(p->next->pnode->name, name))) {
            // fprintf(stderr, "%s,%d:deleting node name:%s\n", __func__, __LINE__, name);
            temp = p->next;
            p->next = temp->next;

            //if the deleted node is the last one, It's very important to change chilt_t to the previous one or NULL
            if (temp->next == NULL) {
                if (node->child_h.next != NULL) {
                    node->child_t.next = p;
                } else {
                    node->child_t.next = NULL;
                }
            }

            _del_node(temp->pnode);
            free(temp);
            temp = NULL;
            rval = 0;
            goto func_exit;
        } else {
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
int remove_childs(node_s* node)
{
    int rval = 0;
    node_l *p, *temp;

    p = node->child_h.next;

    while (p != NULL) {
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
void mod_node(node_s* node, const char* pvalue)
{
    int len;

    if (pvalue == NULL)
        return;
    len = strlen(pvalue);
    if (node->pvalue == NULL) {
        node->pvalue = (char*)malloc(len + 1);
    } else {
        if (len > (int)strlen(node->pvalue)) {
            free(node->pvalue);
            node->pvalue = (char*)malloc(len + 1);
        }
    }
    strcpy(node->pvalue, pvalue);

    return;
}

/*
 * func:
 *      search a node by name use Pre-order
 * ret:
 *      NULL:           failure
 *      other:          pointer to node
 */
node_s* search_node(node_s* node, const char* name)
{
    node_l* p = NULL;
    node_s* res;

    if (node == NULL) {
        return NULL;
    }

    //printf("node->name=[%s], name=[%s]\n", node->name, name);
    if (0 == (strcmp(node->name, name))) {
        return node;
    } else {
        p = &node->child_h;
        while (p->next != NULL) {
            res = search_node(p->next->pnode, name);
            if (res == NULL) {
                p = p->next;
            } else {
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
rdata_s* read_json(const char* path, const char* option)
{
    FILE* fp;
    char buf[256];
    char *name, *value;
    int ret, i, mode;
    rdata_s* h = NULL;
    rdata_s* t = NULL;

    fp = fopen(path, "r");
    if (fp == NULL) {
        perror("read_json: open file failed");
        goto func_exit;
    }
    while (NULL != fgets(buf, 256, fp)) {
        // printf("%s:buf[%s]\n", __func__, buf);
        ret = strlen(buf);
        mode = 0;
        name = buf + ret;
        value = buf + ret;
        for (i = 0; i < ret; i++) {
            switch (mode) {
            case 0:
                if (buf[i] == '\"') {
                    name = buf + i + 1;
                    mode = 1;
                }
                break;
            case 1:
                if (buf[i] == '\"') {
                    buf[i] = 0;
                    mode = 2;
                }
                break;
            case 2:
                if (buf[i] == '\"') {
                    value = buf + i + 1;
                    mode = 3;
                } else if (buf[i] == '[') {
                    value = buf + i + 1;
                    mode = 4;
                }
                break;
            case 3:
                if (buf[i] == '\"') {
                    buf[i] = 0;
                    mode = 5;
                }
                break;
            case 4:
                if (buf[i] == ']') {
                    buf[i] = 0;
                    mode = 5;
                }
                break;
            default:
                break;
            }
            if (mode > 4) {
                break;
            }
        }

        if (strlen(name) == 0) {
            continue;
        }

        if (mode < 5) {
            if (option != NULL) {
                continue;
            } else {
                if (h == NULL) {
                    h = t = (rdata_s*)malloc(sizeof(rdata_s));
                } else {
                    t->next = (rdata_s*)malloc(sizeof(rdata_s));
                    t = t->next;
                }
                strcpy(t->name, name);
                t->pvalue = NULL;
                t->next = NULL;
                continue;
            }
        }

        if (strlen(value) == 0) {
            continue;
        }

        // printf("%s:name[%s] value[%s]\n", __func__, name, value);

        if (option == NULL) {
            if (h == NULL) {
                h = t = (rdata_s*)malloc(sizeof(rdata_s));
            } else {
                t->next = (rdata_s*)malloc(sizeof(rdata_s));
                t = t->next;
            }
            strcpy(t->name, name);
            t->pvalue = (char*)malloc(strlen(value) + 1);
            strcpy(t->pvalue, value);
            t->next = NULL;
        } else {
            if (0 == strcmp(option, name)) {
                h = (rdata_s*)malloc(sizeof(rdata_s));
                strcpy(h->name, name);
                h->pvalue = (char*)malloc(strlen(value) + 1);
                strcpy(h->pvalue, value);
                h->next = NULL;
                goto func_exit;
            } else {
                continue;
            }
        }
    }
#if 0
    while (NULL != fgets(buf, 256, fp)) {
        name = strtok(buf, "{[\t\n \"");
        if (name == NULL)
            continue;
        if (strlen(name) >= 64) {
            printf("name is too long\n");
            continue;
        }

        if ((value = strtok(NULL, "{[\t\n \"")) == NULL)
            continue;

        if (0 != strcmp(value, ":"))
            continue;

        value = strtok(NULL, "{[\t\n \"");

        if (value == NULL)
            continue;

        //printf("%s: %s\n", name, value);

        if (option == NULL) {
            if (h == NULL) {
                h = t = (rdata_s*)malloc(sizeof(rdata_s));
            } else {
                t->next = (rdata_s*)malloc(sizeof(rdata_s));
                t = t->next;
            }
            strcpy(t->name, name);
            t->pvalue = (char*)malloc(strlen(value) + 1);
            strcpy(t->pvalue, value);
            t->next = NULL;
        } else {
            if (0 == strcmp(option, name)) {
                h = (rdata_s*)malloc(sizeof(rdata_s));
                strcpy(h->name, name);
                h->pvalue = (char*)malloc(strlen(value) + 1);
                strcpy(h->pvalue, value);
                h->next = NULL;
                goto func_exit;
            } else {
                continue;
            }
        }
    }
#endif

func_exit:
    if (fp != NULL)
        fclose(fp);

    return h;
}

void free_rdata(rdata_s* h)
{
    rdata_s* temp;

    while (h != NULL) {
        temp = h;
        h = h->next;
        //printf("free %s success\n", temp->name);
        free(temp->pvalue);
        free(temp);
    }

    return;
}

/*
 * func:
 *      modify the value of 'pvalue' according to 'value'
 */
void modify_value(char** pvalue, const char* value)
{
    unsigned int len;

    if (value == NULL) {
        if (*pvalue != NULL) {
            free(*pvalue);
            *pvalue = NULL;
        }
    } else {
        len = strlen(value);

        if (*pvalue == NULL) {
            *pvalue = (char*)malloc(len + 1);
        } else {
            if (strlen(*pvalue) < len) {
                free(*pvalue);
                *pvalue = (char*)malloc(len + 1);
            }
        }

        strcpy(*pvalue, value);
    }

    return;
}

/*
 * func:
 *      read ip from string
 * params:
 *      buf:            ip string
 *      ip:             an 'int' type array to store ipaddress
 */
int read_ipaddr(const char* buf, int* ip)
{
    int rval = 0;

    sscanf(buf, "%u.%u.%u.%d", &ip[0], &ip[1], &ip[2], &ip[3]);

    return rval;
}

/*
 * func:
 *      judge whether ip is NULL
 */
int ipisnull(const char* buf)
{
    int ip[4];

    read_ipaddr(buf, ip);
    if ((ip[0] == 0) && (ip[1] == 0) && (ip[2] == 0) && (ip[3] == 0)) {
        return 1;
    } else
        return 0;

    return 0;
}

/*
 * func:
 *      judge whether ip is host
 */
int ipishost(const char* buf)
{
    int ip[4];

    read_ipaddr(buf, ip);
    if (ip[3] > 0) {
        return 1;
    } else
        return 0;

    return 0;
}

/*
 * func:
 *      get the number from a string, for example, '32' from the string 'node_32' or 12 from the string 'dataPort12Speed'.
 * ret:
 *      failure:                -1;
 */
int getnumfromstr(char* arg)
{
    int value;
    char* p;

    p = arg;
    while (*p != 0) {
        if ((*p >= '0') && (*p <= '9')) {
            sscanf(p, "%d", &value);
            break;
        }
        p++;
    }
    if (*p == 0) {
        value = -1;
    }

    return value;
}
