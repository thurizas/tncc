#include "codeEmitter.h"
#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>

static struct vec* asmInsts;
static char* outFile = NULL;
static const char* defOutName = "a.asm";
static FILE* fd = NULL;

static char* baseName(char* outName)
{
    char* loc = NULL;
    if (NULL == (loc = strchr(outName, '.')))             // period is not found
    {
        return outName;
    }
    else
    {
        int len = (int)strlen(outName);
        ptrdiff_t start = loc - outName;

        for (int ndx = (int)start; ndx < len; ndx++)
        {
            outName[ndx] = '\0';
        }

        return outName;
    }

}

bool ce_init(struct vec* _asmInsts, const char* outName, uint8_t flags)
{
    bool res = false;

    if (NULL != _asmInsts)
    {
        asmInsts = _asmInsts;

        if (NULL == outName)
        {
            fprintf(stderr, "[W] no output file name given, defaluting to %s\n", defOutName);
            outFile = calloc(strlen(defOutName) + 1, sizeof(char));
            if (NULL != outFile)
            {
                strncpy(outFile, defOutName, strlen(defOutName));
            }
            else
            {
                fprintf(stderr, "[-] failed to allocate memory for output file name\n");
                goto ERR_EXIT;
            }
        }
        else
        {
            char* base = baseName((char*)outName);

            outFile = calloc(strlen(base) + 5, sizeof(char));        // +4 for '.asm' and +1 for null-terminator
            if (NULL != outFile)
            {
                strncpy(outFile, base, strlen(outName));
                strcat(outFile, ".asm");
            }
            else
            {
                fprintf(stderr, "[-] failed to allocate memory for output file name\n");
                goto ERR_EXIT;
            }
        }

        fd = fopen(outFile, "w+");
        if (fd == NULL)
        {
            int err = errno;
            fprintf(stderr, "[-] Unable to open output file, error is: %d (%s)\n", err, strerror(err));
            goto ERR_EXIT;
        }
        res = true;
    }
    else
    {
        fprintf(stderr, "[-] No assembly instuctions to output");
    }

ERR_EXIT:
    return res;
}

void ce_deinit()
{
    if (NULL != outFile)
    {
        free(outFile);
        outFile = NULL;
    }

    if (NULL != fd)
    {
        fclose(fd);
        fd = NULL;
    }
}

bool ce_emit()
{
    bool res = false;
    int  ret = EOF;

    struct buffer* line = NULL;
    vec_setCurrentNdx(asmInsts, 0);

    line = vec_getCurrent(asmInsts);
    while (NULL != line)
    {
        if (line != NULL)
        {
            if (line->data != NULL)
            {
                for (uint32_t ndx = 0; ndx < line->curSize; ndx++)
                {
                    ret = fputc(line->data[ndx], fd);
                    if (ret == EOF)
                    {
                        int err = errno;
                        fprintf(stderr, "[-] error writting file, error: %d(%s)\n", err, strerror(err));
                        goto ERR_EXIT;
                    }
                }
            }
            else
            {
                fprintf(stdout, "[-] malformed line in assembly output\n");
                goto ERR_EXIT;
            }
        }

        line = vec_getCurrent(asmInsts);
    }

    res = true;

ERR_EXIT:
    return res;

}