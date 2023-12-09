#ifndef __MAP_H__
#define __MAP_H__

#include <cstdint>
#include "util/linked_list/linked_list.h"

#include "util/hash/str_hash.h"
#include "util/mem_mana/mem_mana.h"

using std::uint32_t;

class map_t {
public:
    using key_t = const char*;
    using value_t = size_t;
    using foreach_cb_t = void (*)(key_t key, value_t value);

    class item_t {
    public:
        list_node_t node;

        key_t key;
        value_t value;
    };

    class item_list_t {
    public:
        item_t item;
        uint32_t length;
    };

private:
    uint32_t mod_value;
    uint32_t mem_pool;
    str_hash_t hash;
    mem_pool_t pool;
    item_list_t* items;

    static item_t* search_node(item_list_t* item_list, key_t key);

public:
    /*
    @brief create a map
    @param mod_value - should be a prime number to reduce hash conflict
    @param hash_cb - hash method
    */
    map_t(uint32_t mod_value, str_hash_t hash_cb, mem_pool_t pool = default_pool);

    /**
     * @brief delete a map
     *
     */
    ~map_t()
    {
        this->clear();
        delete[] this->items;
    }

    inline uint32_t length(void) const
    {
        return this->mod_value * (nullptr != this->items);
    }

    /**
     * @brief insert a key-value pair into a map
     *
     * @param key - a string
     * @param value - size_t value or a pointer
     * @return int == 0: succ, < 0: fail
     */
    int insert(key_t key, value_t value);

    /**
     * @brief search the node which match with the given key
     *
     * @param key - a string
     * @param res - searched node
     * @return int == 0: succ, < 0: fail
     */
    int search(key_t key, value_t* res);

    /**
     * @brief remove the node which has the given key
     *
     * @param key - a string
     * @return int == 0: succ, < 0: fail
     */
    int remove(key_t key);

    /**
     * @brief clear the map, which will delete all nodes
     *
     */
    void clear(void);

    /**
     * @brief iterate all map with a callback
     *
     * @param cb - callback function
     */
    void foreach (foreach_cb_t cb);
};

#endif // __MAP_H__
