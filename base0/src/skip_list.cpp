#include "skip_list.h"
#include <stdlib.h>

struct skip_list_node
{
    int data_;
    skip_list_node** forward_;
};

SkipList::SkipList(int max_level /*=12*/)
    : header_(NULL), size_(0), max_level_(max_level)
{
    this->header_ = new skip_list_node;
    this->header_->forward_ = new skip_list_node* [this->max_level_];
    for (int level = 0; level < this->max_level_; ++level)
    {
        this->header_->forward_[level] = NULL;
    }
}

SkipList::~SkipList()
{
    while (this->header_)
    {
        skip_list_node* tmp_node = this->header_;
        this->header_ = this->header_->forward_[0];
        delete[]tmp_node->forward_;
        delete tmp_node;
    }
}

int SkipList::Insert(int data)
{
    skip_list_node** update = new skip_list_node* [this->max_level_];
    skip_list_node* cur = this->header_;
    for (int k = this->max_level_ - 1; k >= 0; --k)
    {
        skip_list_node* next = NULL;
        while ((next = cur->forward_[k]) && (next->data_ < data))
        {
            cur = next;
        }
        update[k] = cur;
    }

    bool b_find = (update[0]->forward_[0]
                   && update[0]->forward_[0]->data_ == data);
    if (!b_find)
    {
        skip_list_node* node = new skip_list_node;
        node->data_ = data;
        int new_node_level = this->RandLevel();
        if (new_node_level > this->max_level_)
        {
            new_node_level = this->max_level_;
        }
        node->forward_ = new skip_list_node* [new_node_level];
        for (int k = 0; k < new_node_level; ++k)
        {
            node->forward_[k] = update[k]->forward_[k];
            update[k]->forward_[k] = node;
        }
        ++this->size_;
    }

    delete[]update;
    return b_find ? -1 : 0;
}

bool SkipList::Find(int data) const
{
    bool b_find = false;
    skip_list_node* cur = this->header_;
    for (int k = this->max_level_ - 1; k >= 0; --k)
    {
        skip_list_node* next = NULL;
        while ((next = cur->forward_[k]) && next->data_ < data)
        {
            cur = next;
        }
        if (next && next->data_ == data)
        {
            b_find = true;
            break;
        }
    }

    return b_find;
}

int SkipList::Remove(int data)
{
    skip_list_node** update = new skip_list_node* [this->max_level_];
    for (int k = this->max_level_ - 1; k >= 0; --k)
    {
        skip_list_node* cur = this->header_;
        skip_list_node* next = NULL;
        while ((next = cur->forward_[k]) && next->data_ < data)
        {
            cur = next;
        }
        update[k] = cur;
    }
    bool b_find = (update[0]->forward_[0]
                   && update[0]->forward_[0]->data_ == data);
    if (b_find)
    {
        skip_list_node* find_node = update[0]->forward_[0];
        for (int k = 0; k < this->max_level_; ++k)
        {
            if (update[k]->forward_[k] == find_node)
            {
                update[k]->forward_[k] = find_node->forward_[k];
            }
            else
            {
                break;
            }
        }
        delete[]find_node->forward_;
        delete find_node;
        --this->size_;
    }

    return b_find ? 0 : -1;
}

int SkipList::Size() const
{
    return this->size_;
}

int SkipList::RandLevel()
{
    int level = 1;
    while ((::rand() & 0xffff) < (0xffff >> 2))
    {
        ++level;
    }

    return level;
}
