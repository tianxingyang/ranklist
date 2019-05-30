#include <utility>

using std::pair;

template <class K, class E>
struct SkipNode
{
    typedef pair<const K, E> PairType;

    PairType element_;
    SkipNode<K, E>** next_; // 指针数组

    SkipNode(const PairType& the_pair, int size)
        : element_(the_pair), next_(new SkipNode<K, E>* [size]) {}
};

template <class K, class E>
class SkipList
{
public:
    // 关键字小于 large_key 且数对个数 size 最多为 max_pairs 。o < prob < 1
    SkipList(K large_key, int max_pairs, float prob);

    // 返回匹配到的数对的指针
    // 如果没有匹配的数对，返回NULL
    pair<const K, E>* find(const K& the_key);

    // 返回一个表示链表级的随机数，这个数不大于 max_level_
    int level() const;

    // 搜索关键字 the_key，把每一级链表中要查看的最后一个节点存储在数组 last 中
    // 返回包含关键字 the_key 的节点
    SkipNode<K, E>* search(const K& the_key) const;

    // 把数对 the pair 插入字典，覆盖其关键字相同的已存在的数对
    void insert(const pair<const K, E>& the_pair);

    // 删除关键字等于 the_key 的数对
    void erase(const K& the_key);

private:
    float cut_off_; // 确定层数
    int levels_; // 当前最大的非空链表
    int d_size_; // 字典的数对个数
    int max_level_; // 允许的最大链表层数
    K tail_key_; // 最大关键字
    SkipNode<K, E>* header_node_; // 头节点指针 
    SkipNode<K, E>* tail_node_; // 尾节点指针
    SkipNode<K, E>** last_; // last[i] 表示 i 层的最后节点
};

template<class K, class E>
inline SkipList<K, E>::SkipList(K large_key, int max_pairs, float prob)
{
    cut_off_ = prob * RAND_MAX;
    max_level_ = int(ceil(logf(int(max_pairs))) / logf(1 / prob) - 1);
    levels_ = 0;
    d_size_ = 0;
    tail_key_ = large_key;

    // 生成头节点、尾节点和数组 last
    pair<K, E> tail_pair;
    tail_pair.first = tail_key_;
    header_node_ = new SkipNode<K, E>(tail_pair, max_level_ + 1);
    tail_node_ = new SkipNode<K, E>(tail_pair, 0);
    last = new SkipNode<K, E>* [max_level_ + 1];

    // 链表为空时，任意级链表中头节点都指向尾节点
    for (int i = 0; i <= max_level_; ++i)
    {
        header_node_->next_[i] = tail_node_;
    }
}

template<class K, class E>
inline pair<const K, E>* SkipList<K, E>::find(const K& the_key)
{
    if (the_key > tail_key_)
    {
        return NULL;
    }

    // 位置 before_node 是关键字为 the_key 的节点之前最右边的位置
    SkipNode<K, E>* before_node = header_node_;
    for (int i = levels_; i >= 0; --i)
    {
        // 跟踪 i 级链表指针
        while (before_node->next_[i]->element_.first < the_key)
        {
            before_node = before_node->next_[i];
        }
    }

    if (before_node->next_[0]->element_.first == the_key)
    {
        return &before_node->next_[0]->element_;
    }

    return NULL;
}

template<class K, class E>
inline int SkipList<K, E>::level() const
{
    int lev = 0;
    while (rand() <= cut_off_)
    {
        ++lev;
    }

    return lev <= max_level_ ? lev : max_level_;
}

template<class K, class E>
inline SkipNode<K, E>* SkipList<K, E>::search(const K& the_key) const
{
    // 位置 before_node 是关键字为 the_key 的节点之前最右边的位置
    SkipNode<K, E>* before_node = header_node_;
    for (int i = levels_; i >= 0; --i)
    {
        while (before_node->next_[i]->element_.first < the_key)
        {
            before_node = before_node->next_[i];
        }
        last_[i] = before_node;
    }

    return before_node->next_[0];
}

template<class K, class E>
inline void SkipList<K, E>::insert(const pair<const K, E>& the_pair)
{
    if (the_pair.first > tail_key_)
    {
        return;
    }

    // 查找关键字为 the_key 的数对是否已经存在
    SkipNode<K, E>* the_node = search(the_pair.first);
    if (the_node->element_.first == the_pair.first)
    {
        the_node->element_.second = the_pair.second;
        return;
    }

    // 若不存在，则确定新节点所在的级链表
    int the_level = level();
    // 使级 the_level 为 <= levels_ + 1
    if (the_level > levels_)
    {
        the_level = ++levels_;
        last_[the_level] = header_node_;
    }
    
    // 在节点 the_node 之后插入新节点
    SkipNode<K, E>* new_node = new SkipNode<K, E>(the_pair, the_level + 1);
    for (int i = 0; i <= the_level; ++i)
    {
        // 插入 i 级链表
        new_node->next_[i] = last_[i]->next_[i];
        last_[i]->next_[i] = new_node;
    }

    d_size++;
}

template<class K, class E>
inline void SkipList<K, E>::erase(const K& the_key)
{
    if (the_key > tail_key_)
    {
        return;
    }

    SkipNode<K, E>* the_node = search(the_key);
    if (the_node->element_.first != the_key)
    {
        return;
    }

    // 从链表中删除节点
    for (int i = 0; i <= levels_ && last_[i]->next_[i] == the_node; ++i)
    {
        last_[i]->next_[i] = the_node->next_[i];
    }

    // 更新链表
    while (levels_ > 0 && header_node_->next_[levels_] == tail_node_)
    {
        levels_;
    }

    delete the_node;
    --d_size_;
}
