/******************************************************************************
*  LibGHT, software to manage point clouds.
*  LibGHT is free and open source software provided by the Government of Canada
*  Copyright (c) 2012 Natural Resources Canada
*
*  Nouri Sabo <nsabo@NRCan.gc.ca>, Natural Resources Canada
*  Paul Ramsey <pramsey@opengeo.org>, OpenGeo
*
******************************************************************************/

#include "CUnit/Basic.h"
#include "cu_tester.h"

/* GLOBALS ************************************************************/

/* Setup/teardown for this suite */
static int
init_suite(void)
{
    return 0;
}

static int
clean_suite(void)
{
    return 0;
}


/* TESTS **************************************************************/

static void
test_geohash_inout()
{
    /* ght_hash_from_coordinate(const GhtCoordinate *coord,
                                unsigned int resolution,
                                GhtHash **rhash) */

    /* ght_area_from_hash(const GhtHash *hash, GhtArea *area) */

    GhtHash *hash;
    GhtCoordinate coord;
    GhtErr err;
    GhtArea area;

    coord.x = 1.0;
    coord.y = 1.0;
    err = ght_hash_from_coordinate(&coord, 20, &hash);
    CU_ASSERT_EQUAL(err, GHT_OK);
    CU_ASSERT_STRING_EQUAL(hash, "s00twy01mtw037ms06g7");
    err = ght_area_from_hash(hash, &area);
    CU_ASSERT_EQUAL(err, GHT_OK);
    CU_ASSERT_DOUBLE_EQUAL(coord.x, (area.x.min + area.x.max)/2.0, 0.0000000001);
    CU_ASSERT_DOUBLE_EQUAL(coord.y, (area.y.min + area.y.max)/2.0, 0.0000000001);
    ght_free(hash);

    coord.x = 0.0;
    coord.y = 0.0;
    err = ght_hash_from_coordinate(&coord, 20, &hash);
    CU_ASSERT_EQUAL(err, GHT_OK);
    CU_ASSERT_STRING_EQUAL(hash, "s0000000000000000000");
    err = ght_area_from_hash(hash, &area);
    CU_ASSERT_EQUAL(err, GHT_OK);
    CU_ASSERT_DOUBLE_EQUAL(coord.x, (area.x.min + area.x.max)/2.0, 0.0000000001);
    CU_ASSERT_DOUBLE_EQUAL(coord.y, (area.y.min + area.y.max)/2.0, 0.0000000001);
    ght_free(hash);

    coord.x = 90.0;
    coord.y = 0.0;
    err = ght_hash_from_coordinate(&coord, 20, &hash);
    CU_ASSERT_EQUAL(err, GHT_OK);
    CU_ASSERT_STRING_EQUAL(hash, "w0000000000000000000");
    err = ght_area_from_hash(hash, &area);
    CU_ASSERT_EQUAL(err, GHT_OK);
    CU_ASSERT_DOUBLE_EQUAL(coord.x, (area.x.min + area.x.max)/2.0, 0.0000000001);
    CU_ASSERT_DOUBLE_EQUAL(coord.y, (area.y.min + area.y.max)/2.0, 0.0000000001);
    ght_free(hash);

    coord.x = 90.0;
    coord.y = 45.0;
    err = ght_hash_from_coordinate(&coord, 20, &hash);
    CU_ASSERT_EQUAL(err, GHT_OK);
    CU_ASSERT_STRING_EQUAL(hash, "y0000000000000000000");
    err = ght_area_from_hash(hash, &area);
    CU_ASSERT_EQUAL(err, GHT_OK);
    CU_ASSERT_DOUBLE_EQUAL(coord.x, (area.x.min + area.x.max)/2.0, 0.0000000001);
    CU_ASSERT_DOUBLE_EQUAL(coord.y, (area.y.min + area.y.max)/2.0, 0.0000000001);
    ght_free(hash);

    coord.x = 180.0;
    coord.y = 45.0;
    err = ght_hash_from_coordinate(&coord, 20, &hash);
    CU_ASSERT_EQUAL(err, GHT_OK);
    CU_ASSERT_STRING_EQUAL(hash, "zbpbpbpbpbpbpbpbpbpb");
    err = ght_area_from_hash(hash, &area);
    CU_ASSERT_EQUAL(err, GHT_OK);
    CU_ASSERT_DOUBLE_EQUAL(coord.x, (area.x.min + area.x.max)/2.0, 0.0000000001);
    CU_ASSERT_DOUBLE_EQUAL(coord.y, (area.y.min + area.y.max)/2.0, 0.0000000001);
    ght_free(hash);

    coord.x = -180.0;
    coord.y = 45.0;
    err = ght_hash_from_coordinate(&coord, 20, &hash);
    CU_ASSERT_EQUAL(err, GHT_OK);
    CU_ASSERT_STRING_EQUAL(hash, "b0000000000000000000");
    err = ght_area_from_hash(hash, &area);
    CU_ASSERT_EQUAL(err, GHT_OK);
    CU_ASSERT_DOUBLE_EQUAL(coord.x, (area.x.min + area.x.max)/2.0, 0.0000000001);
    CU_ASSERT_DOUBLE_EQUAL(coord.y, (area.y.min + area.y.max)/2.0, 0.0000000001);
    ght_free(hash);

    coord.x = 179.9999;
    coord.y = 45.0;
    err = ght_hash_from_coordinate(&coord, 9, &hash);
    CU_ASSERT_EQUAL(err, GHT_OK);
    CU_ASSERT_STRING_EQUAL(hash, "zbpbpbpbj");
    err = ght_area_from_hash(hash, &area);
    // printf("\n%s\n", hash);
    CU_ASSERT_EQUAL(err, GHT_OK);
    CU_ASSERT_DOUBLE_EQUAL(coord.x, (area.x.min + area.x.max)/2.0, 0.0001);
    CU_ASSERT_DOUBLE_EQUAL(coord.y, (area.y.min + area.y.max)/2.0, 0.0001);
    ght_free(hash);
}

static void
test_ght_hash_common_length(void)
{
    /*
    int
    ght_hash_common_length(const GhtHash *a, const GhtHash *b, int max_len)
    */
    GhtHash *a, *b;
    int common;

    a = "b000000";
    common = ght_hash_common_length(a, a, 9);
    CU_ASSERT_EQUAL(common, 7);
    b = "b00d000";
    common = ght_hash_common_length(a, b, 3);
    CU_ASSERT_EQUAL(common, 3);
    common = ght_hash_common_length(a, b, 5);
    CU_ASSERT_EQUAL(common, 3);

    a = "b000000";
    b = "a00d000";
    common = ght_hash_common_length(a, b, 3);
    CU_ASSERT_EQUAL(common, -1);

    a = "b000000";
    b = "";
    common = ght_hash_common_length(a, b, 3);
    CU_ASSERT_EQUAL(common, 0);

    a = "b000000";
    b = "b00000";
    common = ght_hash_common_length(a, b, 9);
    CU_ASSERT_EQUAL(common, 6);
    common = ght_hash_common_length(b, a, 9);
    CU_ASSERT_EQUAL(common, 6);
}

static void
test_ght_hash_leaf_parts(void)
{
    /*
    GhtErr
    ght_hash_leaf_parts(const GhtHash *a, const GhtHash *b, int maxlen,
                        GhtHashMatch *matchtype, GhtHash **a_leaf, GhtHash **b_leaf)
    */
    GhtHash *a, *b;
    GhtHash *a_leaf, *b_leaf;
    GhtHashMatch matchtype;
    GhtErr e;
    int maxlen = 5;

    a = "abcdefgh";
    b = "abcdefgh";
    maxlen = 8;
    e = ght_hash_leaf_parts(a, b, maxlen, &matchtype, &a_leaf, &b_leaf);
    CU_ASSERT_EQUAL(e, GHT_OK);
    CU_ASSERT_EQUAL(matchtype, GHT_SAME);

    maxlen = 5;
    e = ght_hash_leaf_parts(a, b, maxlen, &matchtype, &a_leaf, &b_leaf);
    CU_ASSERT_EQUAL(e, GHT_OK);
    CU_ASSERT_EQUAL(matchtype, GHT_SPLIT);
    CU_ASSERT_STRING_EQUAL(a_leaf, "fgh");
    CU_ASSERT_STRING_EQUAL(b_leaf, "fgh");

    a = "abcde";
    b = "abcdefgh";
    maxlen = 8;
    e = ght_hash_leaf_parts(a, b, maxlen, &matchtype, &a_leaf, &b_leaf);
    CU_ASSERT_EQUAL(e, GHT_OK);
    CU_ASSERT_EQUAL(matchtype, GHT_CHILD);
    CU_ASSERT_STRING_EQUAL(b_leaf, "fgh");

    a = "abcde";
    b = "1abcdefgh";
    maxlen = 8;
    e = ght_hash_leaf_parts(a, b, maxlen, &matchtype, &a_leaf, &b_leaf);
    CU_ASSERT_EQUAL(e, GHT_ERROR);
    CU_ASSERT_EQUAL(matchtype, GHT_NONE);

    a = "";
    b = "1abcdefgh";
    maxlen = 8;
    e = ght_hash_leaf_parts(a, b, maxlen, &matchtype, &a_leaf, &b_leaf);
    CU_ASSERT_EQUAL(e, GHT_OK);
    CU_ASSERT_EQUAL(matchtype, GHT_GLOBAL);

    a = "abcdafda";
    b = "1abcdh";
    maxlen = 8;
    e = ght_hash_leaf_parts(a, b, maxlen, &matchtype, &a_leaf, &b_leaf);
    CU_ASSERT_EQUAL(e, GHT_ERROR);
    CU_ASSERT_EQUAL(matchtype, GHT_NONE);

}

static void
test_ght_node_build_tree(void)
{
    GhtCoordinate coord;
    int x, y;
    GhtNode *node1, *node2, *node3, *node4, *node5, *root;
    GhtErr err;

    /* ght_node_from_coordinate(const GhtCoordinate *coord, unsigned int resolution, GhtNode **node); */
    coord.x = -127.4123;
    coord.y = 49.23141;
    err = ght_node_from_coordinate(&coord, GHT_MAX_HASH_LENGTH, &root);
    CU_ASSERT_STRING_EQUAL(root->hash, "c0v2hdm1wpzpy4vtv4");
    CU_ASSERT_EQUAL(err, GHT_OK);

    /* ght_node_insert_node(GhtNode *node, GhtNode *node_to_insert, int duplicates) */

    /* insert duplicate */
    coord.x = -127.4123;
    coord.y = 49.23141;
    err = ght_node_from_coordinate(&coord, GHT_MAX_HASH_LENGTH, &node1);
    err = ght_node_insert_node(root, node1, 1);
    CU_ASSERT_EQUAL(err, GHT_OK);
    CU_ASSERT_EQUAL(node1->hash, NULL);

    /* insert split */
    coord.x = -127.4124;
    coord.y = 49.23142;
    err = ght_node_from_coordinate(&coord, GHT_MAX_HASH_LENGTH, &node2);
    /* before insert, it's full length */
    CU_ASSERT_STRING_EQUAL(node2->hash, "c0v2hdm1gcuekpf9y1");
    err = ght_node_insert_node(root, node2, 1);
    CU_ASSERT_EQUAL(err, GHT_OK);
    /* after insert, it's been truncated to the distinct part */
    CU_ASSERT_STRING_EQUAL(node2->hash, "gcuekpf9y1");
    /* and the root has been truncated to the common part */
    CU_ASSERT_STRING_EQUAL(root->hash, "c0v2hdm1");
    /* and distinct part of the root is now a new child node */
    CU_ASSERT_STRING_EQUAL(root->children->nodes[0]->hash, "wpzpy4vtv4");
    /* which in turn has the old identical node as a child */
    CU_ASSERT_EQUAL(root->children->nodes[0]->children->nodes[0], node1);

    /* insert child */
    err = ght_node_new("c0v2hdm1wpzpy4vkv4", &node3);
    /* before insert, it's full length */
    CU_ASSERT_STRING_EQUAL(node3->hash, "c0v2hdm1wpzpy4vkv4");
    err = ght_node_insert_node(root, node3, 1);
    CU_ASSERT_EQUAL(err, GHT_OK);
    /* after insert it's only got the last piece */
    CU_ASSERT_STRING_EQUAL(node3->hash, "kv4");

    /* insert duplicate of previous */
    err = ght_node_new("c0v2hdm1wpzpy4vkv4", &node4);
    /* before insert, it's full length */
    CU_ASSERT_STRING_EQUAL(node4->hash, "c0v2hdm1wpzpy4vkv4");
    err = ght_node_insert_node(root, node4, 1);
    CU_ASSERT_EQUAL(err, GHT_OK);
    /* after insert it's nulled, because it's a duplicate */
    CU_ASSERT_EQUAL(node4->hash, NULL);
    /* also, it's hanging off the previous node */
    CU_ASSERT_EQUAL(node3->children->nodes[0], node4);

    /* insert another duplicate of previous */
    err = ght_node_new("c0v2hdm1wpzpy4vkv4", &node5);
    /* before insert, it's full length */
    CU_ASSERT_STRING_EQUAL(node5->hash, "c0v2hdm1wpzpy4vkv4");
    err = ght_node_insert_node(root, node5, 1);
    CU_ASSERT_EQUAL(err, GHT_OK);
    /* after insert it's nulled, because it's a duplicate */
    CU_ASSERT_EQUAL(node5->hash, NULL);
    /* also, it's hanging off the parent node */
    CU_ASSERT_EQUAL(node3->children->nodes[1], node5);

}


static void
test_ght_node_build_tree_big(void)
{
    GhtCoordinate coord;
    int i, j;
    int npts = 200;
    const double x_off = -127.0;
    const double y_off = 49.0;
    const double scale = 0.0001;
    GhtNode *node, *root;
    GhtErr err;
    int count = 0;
    stringbuffer_t *sb;

    for ( i = 0; i < npts; i++ )
    {
        for ( j = 0; j < npts; j++ )
        {
            coord.x = x_off + i*scale;
            coord.y = y_off + j*scale;
            err = ght_node_from_coordinate(&coord, GHT_MAX_HASH_LENGTH, &node);
            if ( i || j )
            {
                err = ght_node_insert_node(root, node, 1);
            }
            else
            {
                root = node;
            }
        }
    }
    // sb = stringbuffer_create();
    // err = ght_node_to_string(root, sb, 0);
    // printf("\n%s\n", stringbuffer_getstring(sb));
    // stringbuffer_destroy(sb);
    err = ght_node_count_leaves(root, &count);
    CU_ASSERT_EQUAL(err, GHT_OK);
    // printf("count %d\n", count);
    CU_ASSERT_EQUAL(count, npts*npts);
}

/* REGISTER ***********************************************************/

CU_TestInfo core_tests[] =
{
    GHT_TEST(test_geohash_inout),
    GHT_TEST(test_ght_hash_common_length),
    GHT_TEST(test_ght_hash_leaf_parts),
    GHT_TEST(test_ght_node_build_tree),
    GHT_TEST(test_ght_node_build_tree_big),
    CU_TEST_INFO_NULL
};

CU_SuiteInfo core_suite = {"core", init_suite, clean_suite, core_tests};

