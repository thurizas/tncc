#ifndef _node_h_
#define _node_h_

struct node
{
    size_t       dataSize;
    void*        data;
    struct node* flink;
    struct node* blink;
};

#endif

