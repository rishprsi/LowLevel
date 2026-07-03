#include "ctest.h"
#include "linkedlist.h"

int main(void) {
    /* build with push_front: 1,2,3 pushed -> head is 3 -> 2 -> 1 */
    {
        SECTION("list_push_front/reverse");
        Node *h = NULL;
        h = list_push_front(h, 1);
        h = list_push_front(h, 2);
        h = list_push_front(h, 3);
        CHECK_INT_EQ(list_length(h), 3);
        CHECK_INT_EQ(h->val, 3);
        CHECK_INT_EQ(h->next->val, 2);
        CHECK_INT_EQ(h->next->next->val, 1);

        /* reverse -> 1 -> 2 -> 3 */
        h = list_reverse(h);
        CHECK_INT_EQ(h->val, 1);
        CHECK_INT_EQ(h->next->val, 2);
        CHECK_INT_EQ(h->next->next->val, 3);
        CHECK_PTR_NULL(h->next->next->next);

        list_free(h);
    }

    /* empty + single + length */
    {
        SECTION("list_length");
        CHECK_INT_EQ(list_length(NULL), 0);
        Node *h = list_push_front(NULL, 7);
        CHECK_INT_EQ(list_length(h), 1);
        CHECK_INT_EQ(list_middle(h), 7);
        list_free(h);
    }

    /* middle: odd length 1..5 -> 3 ; even length 1..4 -> 3 (second middle) */
    {
        SECTION("list_middle");
        Node *odd = NULL;
        for (int i = 5; i >= 1; i--) {
            odd = list_push_front(odd, i); /* yields 1,2,3,4,5 */
        }
        CHECK_INT_EQ(list_middle(odd), 3);
        list_free(odd);

        Node *even = NULL;
        for (int i = 4; i >= 1; i--) {
            even = list_push_front(even, i); /* 1,2,3,4 */
        }
        CHECK_INT_EQ(list_middle(even), 3);
        list_free(even);

        Node *two = NULL;
        two = list_push_front(two, 2);
        two = list_push_front(two, 1); /* 1,2 */
        CHECK_INT_EQ(list_middle(two), 2);
        list_free(two);
    }

    /* cycle detection */
    {
        SECTION("list_has_cycle");
        Node *h = NULL;
        for (int i = 4; i >= 1; i--) {
            h = list_push_front(h, i); /* 1,2,3,4 */
        }
        CHECK_FALSE(list_has_cycle(h));

        /* make a cycle: tail->next = h, then verify, then break and free.
         * Bounded so a buggy push_front that already made a cycle can't hang. */
        Node *tail = h;
        for (int guard = 0; tail->next != NULL && guard < 100000; guard++) {
            tail = tail->next;
        }
        tail->next = h;
        CHECK_TRUE(list_has_cycle(h));
        tail->next = NULL; /* break the cycle before freeing */
        list_free(h);
    }

    CTEST_END();
}
