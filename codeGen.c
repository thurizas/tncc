#include "codeGen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "vector.h"
#include "buffer.h"
#include "node.h"
#include "astNode.h"


static struct vec* asmInsts = NULL;              // vector to hold list of assembly instructions
static struct astNode* root = NULL;

bool cg_init(struct astNode* _root, uint8_t flags)
//bool cg_init(struct astNode* _root, uint8_t flags)
{
    bool res = false;

    if (NULL != _root)
    {
        root = _root;                                    // get a local copy of AST
        
        vec_init(&asmInsts);
    
        if (asmInsts != NULL)
        {
            res = true;
        }
        else
        {
            fprintf(stderr, "[-] failed to initialize assembly instruction list\n");
        }
    }
    else
    {
        fprintf(stderr, "[-] AST structure in NULL.");
    }

    return res;
}

void cg_deinit()
{
    struct buffer* line = NULL;   
    vec_setCurrentNdx(asmInsts, 0);

    line = vec_getCurrent(asmInsts);

    while (NULL != line)
    {
        buf_free(&line);

        line = vec_getCurrent(asmInsts);
    }
    vec_free(asmInsts);
}

static bool cg_evalExpression(struct astNode* pNode, struct val* pVal)
{
    bool res = false;

    if (pNode->type == AST_TYPE_INTVAL)
    {
        pVal->type = INTVAL;
        pVal->iVal = pNode->iVal;
    }

    return res;
}

static bool cg_processStmt(struct stmt* pStmt)
{
    bool res = false;

    switch (pStmt->type)
    {
        case AST_STMT_TYPE_RETURN:
        {
            struct val val;
            // evaluate the return value
            struct astNode* pNode = pStmt->returnStmt.exp;
            cg_evalExpression(pNode, &val);

            char* buf = tncc_calloc(64, sizeof(char));

            struct buffer* line = NULL;
            buf_init(&line);

            sprintf(buf, "%*smov    rax, %d\n", 4, "", val.iVal);
            buf_insert(line, buf);

            memset(buf, '\0', 15);
            sprintf(buf, "%*sret\n", 4, "");
            buf_insert(line, buf);

            vec_push(asmInsts, (int)strlen(buf)+1, line);

            free(buf);

            res = true;
        }
        break;

        default:
            fprintf(stderr, "[-] unknown statement type\n");
    }

    return res;
}

static bool cg_processFnct(struct fnct* fnct)
{
    bool res = false;

    // generate label for function
    struct buffer* label = NULL;
    buf_init(&label);
    buf_insert(label, fnct->name);
    buf_append(label, ':');
    buf_append(label, '\n');
    vec_push(asmInsts, buf_len(label), label);

    // generate function body
    vec_setCurrentNdx(fnct->stmts, 0);
    struct astNode* node = vec_getCurrent(fnct->stmts);
    while (NULL != node)
    {
        switch (node->type)
        {
            case AST_TYPE_STMT:
                res = cg_processStmt(&(node->stmt));
                break;
        }

        
        node = vec_getCurrent(fnct->stmts);
    }

    return res;
}

bool cg_genAsm()
{
    bool res = false;

    if ((NULL != root))
    {
        uint32_t type = root->type;
        switch (type)
        {
            case AST_TYPE_PROGRAM:
                {
                    struct buffer* fnctList = NULL;
                    buf_init(&fnctList);
                    
                    if (NULL != fnctList)
                    {
                        struct prog p = root->prog;

                        if ((NULL != p.fncts) && (vec_len(p.fncts) > 0))
                        {
                            vec_setCurrentNdx(p.fncts, 0);
                            struct astNode* n = NULL;
                            while (NULL != (n = vec_getCurrent(p.fncts)))
                            {
                                struct fnct* f = &(n->fnct);
                                char* name = f->name;
                                buf_insert(fnctList, "\n.globl ");
                                buf_insert(fnctList, name);
                                buf_insert(fnctList, "\n\n");

                                if (!cg_processFnct(f))
                                {
                                    fprintf(stderr, "[-] assembly generation failed at line: %d, col: %d\n", root->pos.line, root->pos.col);
                                }
                            }

                            // push fnctList to beginning of asmInsts
                            vec_front(asmInsts, buf_len(fnctList), fnctList);


                        }
                        else
                        {
                            fprintf(stdout, "Warning: program has no functions");
                        }
                    }
                }

                break;

            default:
                {
                    fprintf(stderr, "[-] unknow AST node type: %d\n", root->type);
                }

                break;
        }

        struct buffer* bufTail = NULL;
        buf_init(&bufTail);
        buf_insert(bufTail, "\n.section .note.GNU-stack, \"\",@progbits");
        vec_push(asmInsts, buf_len(bufTail), bufTail);
        res = true;
    }

    return res;
}

struct vec* cg_getAsm()
{
    return asmInsts;
}


void cg_printAsm(struct vec* asmListing)
{
    struct buffer* line = NULL;
    vec_setCurrentNdx(asmListing, 0);

    line = vec_getCurrent(asmListing);
    while (NULL != line)
    {
        buf_print(line);
        line = vec_getCurrent(asmListing);
    }
}