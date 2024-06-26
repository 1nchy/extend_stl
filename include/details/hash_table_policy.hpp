#ifndef _DETAILS_ICY_HASH_TABLE_POLICY_HPP_
#define _DETAILS_ICY_HASH_TABLE_POLICY_HPP_

#include <cmath>
#include <cstring>
#include <memory>

#include "node.hpp"

namespace icy {

struct rehash_policy;
template <typename _Tp> struct hash_node;
template <typename _Value, typename _Alloc> struct hash_table_alloc;
struct _ExtractKey;

extern const unsigned long _prime_list[] = {
    5ul,          11ul,
    23ul,         47ul,         97ul,         193ul,       389ul,
    769ul,        1543ul,       3079ul,       6151ul,      12289ul,
    24593ul,      49157ul,      98317ul,      196613ul,    393241ul,
    786433ul,     1572869ul,    3145739ul,    6291469ul,   12582917ul,
    25165843ul,   50331653ul,   100663319ul,  201326611ul, 402653189ul,
    805306457ul,  1610612741ul, 3221225473ul, 4294967291ul
};

struct rehash_policy {
    typedef enum {
        __FAILED__,
        __WRONG__,
        __PAUSED__,
        __NORMAL__,
        __COMPLETED__,
    } status;
    typedef short bucket_id;
    // (0, x) indicates _buckets[x]
    // (1, y) indicates _rehash_buckets[y]
    typedef std::pair<bucket_id, size_t> bucket_index;

    rehash_policy(float _z = 1.0) : _max_load_factor(_z) {}
    rehash_policy(bool _enable) : _enable_rehash(_enable) {}

    float max_load_factor() const { return _max_load_factor; }
    size_t state() const { return _next_resize; }
    void reset(size_t _s = 0) { _next_resize = _s; }

    /**
     * @param %_n = current number of buckets
     * @return the next prime number that \ge %_n
    */
    size_t next_bkt(size_t _n) const;
    /**
     * @param %_n = the number of elements
     * @return the least number of buckets, which's able to contain %_n elements.
    */
    size_t bkt_for_elements(size_t _n) const;
    /**
     * @param %_n_bkt = the number of buckets in @hash_table;
     *        %_n_elt = the number of elements in @hash_table;
     *        %_n_ins = the number of elements that need to insert.
     * @return if rehash is needed，the return (true, new_bucket_count); else return (false, 0)
    */
    std::pair<bool, size_t> need_rehash(size_t _n_bkt, size_t _n_elt, size_t _n_ins) const;

    enum { _s_primes = sizeof(_prime_list) / sizeof(_prime_list[0]) };

    static const size_t _s_growth_factor = 2;
    // 负载因子，衡量桶的负载程度
    float _max_load_factor = 1.0;  // = _n_elt / _n_bkt
    // resize 后，能保存元素个数的最佳上限
    mutable size_t _next_resize = 0; // = _n_bkt * _max_load_factor
    bool _in_rehash = false;
    bucket_index _cur_process;
    const bool _enable_rehash = true;
};

template <typename _Tp> struct hash_node : public node<_Tp> {
    typedef node<_Tp> base;
    typedef hash_node<_Tp> self;
    typedef _Tp value_type;
    typedef _Tp* pointer;
    typedef self* bucket_type;
    typedef size_t hash_code;

    hash_node() : base() {}
    hash_node(const value_type& _v): base(_v) {}
    template <typename... _Args> hash_node(_Args&&... _args): base(std::forward<_Args>(_args)...) {}
    hash_node(const self& r) : base(r), _hash_code(r._hash_code) {}
    hash_node(self&& r) : base(std::move(r)), _hash_code(std::move(r._hash_code)) {}
    virtual ~hash_node() = default;
    
    self* _next = nullptr;
    self* _prev = nullptr;
    hash_code _hash_code;

    friend bool operator==(const self& _x, const self& _y) {
        return _x._hash_code == _y._hash_code && base::operator==(_x, _y);
    }
    friend bool operator!=(const self& _x, const self& _y) {
        return _x._hash_code != _y._hash_code || base::operator!=(_x, _y);
    }
};

template <typename _Value, typename _Alloc> struct hash_table_alloc : public _Alloc {
    typedef hash_node<_Value> node_type;
    typedef typename node_type::value_type value_type;
    typedef typename node_type::bucket_type bucket_type;
    typedef std::allocator_traits<_Alloc> alloc_traits;
    typedef typename alloc_traits::template rebind_alloc<node_type> node_allocator_type;
    typedef std::allocator_traits<node_allocator_type> node_alloc_traits;
    typedef typename alloc_traits::template rebind_alloc<bucket_type> bucket_allocator_type;
    typedef std::allocator_traits<bucket_allocator_type> bucket_alloc_traits;

    _Alloc& _M_get_allocator() { return *static_cast<_Alloc*>(this); }
    const _Alloc& _M_get_allocator() const { return *static_cast<const _Alloc*>(this); }
    node_allocator_type _M_get_node_allocator() const { return node_allocator_type(_M_get_allocator()); }
    bucket_allocator_type _M_get_bucket_allocator() const { return bucket_allocator_type(_M_get_allocator()); }

    node_type* _M_allocate_node(const node_type& _x) {
        node_allocator_type _node_alloc = _M_get_node_allocator();
        auto _ptr = node_alloc_traits::allocate(_node_alloc, 1);
        node_type* _p = std::addressof(*_ptr);
        node_alloc_traits::construct(_node_alloc, _p, _x.val());
        _p->_hash_code = _x._hash_code;
        return _p;
    }
    template <typename... _Args> node_type* _M_allocate_node(_Args&&... _args) {
        node_allocator_type _node_alloc = _M_get_node_allocator();
        auto _ptr = node_alloc_traits::allocate(_node_alloc, 1);
        node_type* _p = std::addressof(*_ptr);
        node_alloc_traits::construct(_node_alloc, _p, std::forward<_Args>(_args)...);
        return _p;
    }
    void _M_deallocate_node(node_type* _p) {
        node_allocator_type _node_alloc = _M_get_node_allocator();
        node_alloc_traits::destroy(_node_alloc, _p);
        node_alloc_traits::deallocate(_node_alloc, _p, 1);
    }
    bucket_type* _M_allocate_buckets(size_t _n) {
        bucket_allocator_type _bucket_alloc = _M_get_bucket_allocator();
        auto _ptr = bucket_alloc_traits::allocate(_bucket_alloc, _n);
        bucket_type* _p = std::addressof(*_ptr);
        bzero(_p, _n * sizeof(bucket_type));
        return _p;
    }
    void _M_deallocate_buckets(bucket_type* _p, size_t _n) {
        bucket_allocator_type _bucket_alloc = _M_get_bucket_allocator();
        bucket_alloc_traits::deallocate(_bucket_alloc, _p, _n);
    }
};

size_t rehash_policy::next_bkt(size_t _n) const {
    const unsigned long *_p = std::lower_bound(_prime_list, _prime_list + _s_primes - 1, _n);
    _next_resize = static_cast<size_t>(std::ceil(*_p * _max_load_factor));
    return *_p;
}

size_t rehash_policy::bkt_for_elements(size_t _n) const {
    return std::ceil(_n / (long double)_max_load_factor);
}

std::pair<bool, size_t> rehash_policy::need_rehash(size_t _n_bkt, size_t _n_elt, size_t _n_ins) const {
    if (_n_elt + _n_ins > _next_resize) {
        float _min_bkts = ((float(_n_ins) + float(_n_elt)) / _max_load_factor);
        if (_min_bkts > _n_bkt) {
            _min_bkts = std::max(_min_bkts, float(_s_growth_factor * _n_bkt));
            return std::make_pair(_enable_rehash, next_bkt(static_cast<size_t>(std::ceil(_min_bkts))));
        }
        else {
            _next_resize = static_cast<size_t>(std::ceil(_n_bkt * _max_load_factor));
            return std::make_pair(false, 0);
        }
    }
    else {
        return std::make_pair(false, 0);
    }
}

};

#endif  // _DETAILS_ICY_HASH_TABLE_POLICY_HPP_