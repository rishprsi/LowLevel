#include "ctest.h"
#include "intrusive.h"

/* A payload struct with the list_head embedded in the MIDDLE, so that
 * container_of must actually subtract a nonzero offset. */
struct task {
    int id;
    struct list_head link;
    int payload;
};

/* Collect task ids by walking forward; returns count (caps at maxn). */
static size_t collect_ids(const struct list_head *head, int *out, size_t maxn) {
    size_t n = 0;
    for (const struct list_head *p = head->next; p != head && n < maxn;
         p = p->next) {
        const struct task *t = container_of(p, const struct task, link);
        out[n++] = t->id;
    }
    return n;
}

/* Same, walking backward via prev. */
static size_t collect_ids_rev(const struct list_head *head, int *out,
                              size_t maxn) {
    size_t n = 0;
    for (const struct list_head *p = head->prev; p != head && n < maxn;
         p = p->prev) {
        const struct task *t = container_of(p, const struct task, link);
        out[n++] = t->id;
    }
    return n;
}

int main(void) {
    int ids[16];

    /* ---- LIST_INIT static initializer ---- */
    SECTION("LIST_INIT static initializer");
    struct list_head statik = LIST_INIT(statik);
    CHECK_TRUE(list_empty(&statik));
    CHECK_TRUE(statik.next == &statik);
    CHECK_TRUE(statik.prev == &statik);

    /* ---- runtime init ---- */
    SECTION("list_head_init");
    struct list_head todo;
    list_head_init(&todo);
    CHECK_TRUE(list_empty(&todo));
    CHECK_UINT_EQ(collect_ids(&todo, ids, 16), 0);

    /* ---- container_of recovers the embedding struct ---- */
    SECTION("container_of");
    struct task a = {.id = 1, .payload = 100};
    struct task b = {.id = 2, .payload = 200};
    struct task c = {.id = 3, .payload = 300};
    CHECK_TRUE(container_of(&a.link, struct task, link) == &a);

    /* ---- list_add inserts after head (stack order) ---- */
    SECTION("list_add");
    list_add(&a.link, &todo); /* [1] */
    CHECK_FALSE(list_empty(&todo));
    list_add(&b.link, &todo); /* [2 1] */
    list_add(&c.link, &todo); /* [3 2 1] */
    CHECK_UINT_EQ(collect_ids(&todo, ids, 16), 3);
    CHECK_INT_EQ(ids[0], 3);
    CHECK_INT_EQ(ids[1], 2);
    CHECK_INT_EQ(ids[2], 1);
    /* backward walk sees the reverse */
    CHECK_UINT_EQ(collect_ids_rev(&todo, ids, 16), 3);
    CHECK_INT_EQ(ids[0], 1);
    CHECK_INT_EQ(ids[1], 2);
    CHECK_INT_EQ(ids[2], 3);
    /* payloads recovered intact through container_of */
    CHECK_INT_EQ(container_of(todo.next, struct task, link)->payload, 300);
    CHECK_INT_EQ(container_of(todo.prev, struct task, link)->payload, 100);

    /* ---- list_add_tail inserts before head (queue order) ---- */
    SECTION("list_add_tail");
    struct task d = {.id = 4, .payload = 400};
    list_add_tail(&d.link, &todo); /* [3 2 1 4] */
    CHECK_UINT_EQ(collect_ids(&todo, ids, 16), 4);
    CHECK_INT_EQ(ids[3], 4);

    /* ---- list_del: middle, front, back ---- */
    SECTION("list_del");
    list_del(&b.link); /* [3 1 4] */
    CHECK_UINT_EQ(collect_ids(&todo, ids, 16), 3);
    CHECK_INT_EQ(ids[0], 3);
    CHECK_INT_EQ(ids[1], 1);
    CHECK_INT_EQ(ids[2], 4);
    list_del(&c.link); /* front -> [1 4] */
    list_del(&d.link); /* back  -> [1] */
    CHECK_UINT_EQ(collect_ids(&todo, ids, 16), 1);
    CHECK_INT_EQ(ids[0], 1);
    CHECK_FALSE(list_empty(&todo));
    list_del(&a.link); /* -> [] */
    CHECK_TRUE(list_empty(&todo));
    CHECK_UINT_EQ(collect_ids(&todo, ids, 16), 0);

    /* ---- a deleted node can be re-added ---- */
    SECTION("re-add deleted node");
    list_add_tail(&a.link, &todo);
    list_add_tail(&b.link, &todo); /* [1 2] */
    CHECK_UINT_EQ(collect_ids(&todo, ids, 16), 2);
    CHECK_INT_EQ(ids[0], 1);
    CHECK_INT_EQ(ids[1], 2);

    /* ---- moving an entry between two lists ---- */
    SECTION("move entry between lists");
    struct list_head done;
    list_head_init(&done);
    list_del(&a.link);
    list_add_tail(&a.link, &done);
    CHECK_UINT_EQ(collect_ids(&todo, ids, 16), 1);
    CHECK_INT_EQ(ids[0], 2);
    CHECK_UINT_EQ(collect_ids(&done, ids, 16), 1);
    CHECK_INT_EQ(ids[0], 1);

    CTEST_END();
}
