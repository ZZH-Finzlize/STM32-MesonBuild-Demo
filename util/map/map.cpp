#include <string.h>
#include "map.h"
#include "util/iterators.h"
#include "util/container_of.h"
#include "util/linked_list/list_iterators.h"
#include "util/gnu_attributes.h"

map_t::item_t* map_t::search_node(item_list_t* item_list, key_t key)
{
    CHECK_PTR(item_list, NULL);
    CHECK_PTR(key, NULL);

    // if there is no any node, then it can't be found
    // if have some nodes, then check every one
    if (0 != item_list->length) {
        ITER_LIST (iter, &item_list->item.node) {
            item_t* item = container_of(iter, item_t, node);
            if (NULL != item->key) {
                if (0 == strcmp(item->key, key)) {
                    return item;
                }
            }
        }
    }

    return NULL;
}

map_t::map_t(uint32_t mod_value, str_hash_t hash_cb, mem_pool_t pool)
    : mod_value(0), hash(nullptr), pool(pool)
{
    RETURN_IF(mod_value < 2, );
    CHECK_PTR(hash_cb, );

    this->items = new (this->pool) item_list_t[sizeof(item_list_t) * mod_value];
    CHECK_PTR(this->items, );

    this->mod_value = mod_value;
    this->hash = hash_cb;
    // this->mem_pool = pool;

    FOR_I (mod_value) {
        this->items[i].item.node.next = NULL;
        this->items[i].length = 0;
    }
}

int map_t::insert(key_t key, value_t value)
{
    CHECK_PTR(key, -EINVAL);
    RETURN_IF(this->mem_pool == UINT32_MAX, -ENOBUFS);

    key_t new_key = key;

    uint32_t hash_val = this->hash(new_key) % this->mod_value;
    item_list_t* item_list = &this->items[hash_val];

    item_t* item = search_node(item_list, key);

    if (NULL == item) {               // not a duplicate key
        if (0 == item_list->length) { // first node of this list
            item_list->item.key = new_key;
            item_list->item.value = value;
        } else { // from second node start, we need to alloc new node
            item_t* new_item = new (this->pool) item_t;
            CHECK_PTR(new_item, -ENOMEM);
            new_item->key = new_key;
            new_item->value = value;
            new_item->node.next = NULL;
            list_append(&item_list->item.node, &new_item->node);
        }

        item_list->length++;
    } else { // for duplicate key, just modify the value
        item->value = value;
    }

    return 0;
}

int map_t::search(key_t key, value_t* res)
{
    CHECK_PTR(res, -EINVAL);

    uint32_t hash_val = this->hash(key) % this->mod_value;
    item_list_t* item_list = &this->items[hash_val];

    item_t* item = this->search_node(item_list, key);

    CHECK_PTR(item, -EEXIST);

    *res = item->value;

    return 0;
}

int map_t::remove(key_t key)
{
    uint32_t hash_val = this->hash(key) % this->mod_value;
    item_list_t* item_list = &this->items[hash_val];

    item_t* item = this->search_node(item_list, key);

    CHECK_PTR(item, -ENODATA);

    // the item to be deleted is the head, which is not a allocted node
    if (item == &item_list->item) {
        item->key = NULL;
    } else { // item is a allocated node, then just delete it
        list_remove(&item_list->item.node, &item->node);
        delete item;
    }

    item_list->length--;

    return 0;
}

void map_t::clear()
{
    FOR_I (this->mod_value) {
        item_list_t* item_list = &this->items[i];
        if (item_list->length > 1) {
            list_node_t* node = LL_NEXT_NODE(&item_list->item.node);

            while (NULL != node) {
                list_node_t* next = node->next;
                delete container_of(node, item_t, node);
                node = next;
            }
        }

        item_list->item.key = NULL;
    }
}

void map_t::foreach (foreach_cb_t cb)
{
    FOR_I (this->mod_value) {
        item_list_t* item_list = &this->items[i];
        if (0 != item_list->length) {
            ITER_LIST (iter, &item_list->item.node) {
                item_t* item = container_of(iter, item_t, node);
                cb(item->key, item->value);
            }
        }
    }
}
