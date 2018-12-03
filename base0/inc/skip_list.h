#ifndef BASE_INC_SKIP_LIST_H_
#define BASE_INC_SKIP_LIST_H_

struct skip_list_node;

class SkipList
{
public:
    explicit SkipList(int max_level = 12);
    ~SkipList();

    int Insert(int data);
    bool Find(int data) const;
    int Remove(int data);
    int Size() const;

private:
    int RandLevel();

private:
    skip_list_node* header_;
    int size_;
    int max_level_;

private:
    SkipList(const SkipList&);
    SkipList& operator=(const SkipList&);
};

#endif // BASE_INC_SKIP_LIST_H_
