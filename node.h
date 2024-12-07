#ifndef _node_h_
#define _node_h_

struct node
{
    int   dataSize;
    void* data;
    struct node* flink;
    struct node* blink;
};

#endif

