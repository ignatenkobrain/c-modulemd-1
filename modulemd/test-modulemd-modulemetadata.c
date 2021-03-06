/* test-modulemd-metadata.c
 *
 * Copyright (C) 2017 Stephen Gallagher
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "modulemd-modulemetadata.h"

#include <glib.h>
#include <locale.h>

typedef struct _ModuleMetadataFixture {
    ModulemdModuleMetadata *md;
} ModuleMetadataFixture;

typedef struct _ModuleMetadataPropString {
    const gchar *property_name;
    const gchar *test_str;
} ModuleMetadataPropString;

static void
modulemd_modulemetadata_set_up(ModuleMetadataFixture *fixture,
                               gconstpointer user_data)
{
    fixture->md = modulemd_modulemetadata_new();
}

static void
modulemd_modulemetadata_tear_down(ModuleMetadataFixture *fixture,
                                  gconstpointer user_data)
{
    modulemd_modulemetadata_free(fixture->md);
}

static void
modulemd_modulemetadata_test_string_prop(ModuleMetadataFixture *fixture,
                                         gconstpointer user_data)
{
    GValue value = G_VALUE_INIT;
    GValue ref_value = G_VALUE_INIT;
    ModulemdModuleMetadata *md = fixture->md;
    ModuleMetadataPropString *prop_ctx =
        (ModuleMetadataPropString *)user_data;

    g_value_init(&value, G_TYPE_STRING);
    g_value_init(&ref_value, G_TYPE_STRING);
    g_value_set_string(&ref_value, prop_ctx->test_str);

    g_object_get_property(G_OBJECT(md), prop_ctx->property_name, &value);

    /* Initial state should be NULL */
    g_assert_cmpstr(g_value_get_string(&value), ==, NULL);
    g_value_reset(&value);

    /* Assign the test value */
    g_object_set_property(G_OBJECT(md), prop_ctx->property_name, &ref_value);
    g_value_reset(&value);

    /* Verify that the value is now set */
    g_object_get_property(G_OBJECT(md), prop_ctx->property_name, &value);
    g_assert_cmpstr(g_value_get_string(&value),
                    ==,
                    g_value_get_string(&ref_value));

    g_value_unset(&ref_value);
    g_value_unset(&value);
}

static void
modulemd_modulemetadata_test_get_set_buildrequires(ModuleMetadataFixture *fixture,
                                                   gconstpointer user_data)
{
    GValue value = G_VALUE_INIT;
    GValue *set_value;
    ModulemdModuleMetadata *md = fixture->md;
    GHashTable *htable;

    /* Should be initialized to an empty hash table */
    GHashTable *buildrequires = modulemd_modulemetadata_get_buildrequires(md);
    g_assert_cmpint(g_hash_table_size(buildrequires), ==, 0);

    /* Add a key and value using set_buildrequires() */
    g_hash_table_insert(buildrequires, g_strdup("MyKey"), g_strdup("MyValue"));
    modulemd_modulemetadata_set_buildrequires(md, buildrequires);

    /* Verify the key and value with get_buildrequires() */
    buildrequires = modulemd_modulemetadata_get_buildrequires(md);
    g_assert_cmpint(g_hash_table_size(buildrequires), ==, 1);
    g_assert_true(g_hash_table_contains(buildrequires, "MyKey"));
    g_assert_cmpstr(g_hash_table_lookup(buildrequires, "MyKey"), ==, "MyValue");

    /* Verify the key and value with properties */
    g_value_init(&value, G_TYPE_HASH_TABLE);
    g_object_get_property(G_OBJECT(md), "buildrequires", &value);
    htable = g_value_get_boxed(&value);
    g_value_reset(&value);

    g_assert_cmpint(g_hash_table_size(htable), ==, 1);
    g_assert_true(g_hash_table_contains(htable, "MyKey"));
    g_assert_cmpstr(g_hash_table_lookup(htable, "MyKey"), ==, "MyValue");

    /* Add a second key and value using set_buildrequires() */
    g_hash_table_insert(buildrequires, g_strdup("MyKey2"), g_strdup("MyValue2"));
    modulemd_modulemetadata_set_buildrequires(md, buildrequires);

    /* Verify the second key and value with properties */
    g_object_get_property(G_OBJECT(md), "buildrequires", &value);
    htable = g_value_get_boxed(&value);
    g_value_reset(&value);

    g_assert_cmpint(g_hash_table_size(htable), ==, 2);
    g_assert_true(g_hash_table_contains(htable, "MyKey2"));
    g_assert_cmpstr(g_hash_table_lookup(htable, "MyKey2"), ==, "MyValue2");


    /* Add a third key using the properties interface */
    g_hash_table_insert(htable,
                        g_strdup("MyKey3"),
                        g_strdup("MyValue3"));

    set_value = g_new0(GValue, 1);
    g_value_init(set_value, G_TYPE_HASH_TABLE);
    g_value_set_boxed(set_value, htable);
    modulemd_modulemetadata_set_buildrequires(md, htable);

    g_object_set_property(G_OBJECT(md), "buildrequires", set_value);

    /* Verify the third key and value with get_buildrequires() */
    buildrequires = modulemd_modulemetadata_get_buildrequires(md);
    g_assert_cmpint(g_hash_table_size(buildrequires), ==, 3);
    g_assert_true(g_hash_table_contains(buildrequires, "MyKey3"));
    g_assert_cmpstr(g_hash_table_lookup(buildrequires, "MyKey3"),
                    ==, "MyValue3");
}

static void
modulemd_modulemetadata_test_get_set_community(ModuleMetadataFixture *fixture,
                                               gconstpointer user_data)
{
    ModulemdModuleMetadata *md = fixture->md;

    /* Should be initialized to NULL */
    g_assert_cmpstr(modulemd_modulemetadata_get_community(md), ==, NULL);

    /* Assign a valid string */
    modulemd_modulemetadata_set_community(md, "MyCommunity");
    g_assert_cmpstr(modulemd_modulemetadata_get_community(md), ==, "MyCommunity");

    /* Reassign it to NULL */
    modulemd_modulemetadata_set_community(md, NULL);
    g_assert_cmpstr(modulemd_modulemetadata_get_community(md), ==, NULL);
}

static void
modulemd_modulemetadata_test_get_set_description(ModuleMetadataFixture *fixture,
                                                 gconstpointer user_data)
{
    ModulemdModuleMetadata *md = fixture->md;

    /* Should be initialized to NULL */
    g_assert_cmpstr(modulemd_modulemetadata_get_description(md), ==, NULL);

    /* Assign a valid string */
    modulemd_modulemetadata_set_description(md, "ModuleDesc");
    g_assert_cmpstr(modulemd_modulemetadata_get_description(md), ==, "ModuleDesc");

    /* Reassign it to NULL */
    modulemd_modulemetadata_set_description(md, NULL);
    g_assert_cmpstr(modulemd_modulemetadata_get_description(md), ==, NULL);
}

static void
modulemd_modulemetadata_test_get_set_documentation(ModuleMetadataFixture *fixture,
                                                   gconstpointer user_data)
{
    ModulemdModuleMetadata *md = fixture->md;

    /* Should be initialized to NULL */
    g_assert_cmpstr(modulemd_modulemetadata_get_documentation(md), ==, NULL);

    /* Assign a valid string */
    modulemd_modulemetadata_set_documentation(md, "ModuleDocs");
    g_assert_cmpstr(modulemd_modulemetadata_get_documentation(md), ==, "ModuleDocs");

    /* Reassign it to NULL */
    modulemd_modulemetadata_set_documentation(md, NULL);
    g_assert_cmpstr(modulemd_modulemetadata_get_documentation(md), ==, NULL);
}

static void
modulemd_modulemetadata_test_get_set_mdversion(ModuleMetadataFixture *fixture,
                                               gconstpointer user_data)
{
    ModulemdModuleMetadata *md = fixture->md;

    /* Should be initialized to 0 */
    g_assert_cmpuint(modulemd_modulemetadata_get_mdversion(md), ==, 0);

    /* Assign a valid version */
    modulemd_modulemetadata_set_mdversion(md, 1);
    g_assert_cmpuint(modulemd_modulemetadata_get_mdversion(md), ==, 1);

    /* Reassign it to 0 */
    modulemd_modulemetadata_set_mdversion(md, 0);
    g_assert_cmpuint(modulemd_modulemetadata_get_mdversion(md), ==, 0);
}

static void
modulemd_modulemetadata_test_get_set_name(ModuleMetadataFixture *fixture,
                                          gconstpointer user_data)
{
    ModulemdModuleMetadata *md = fixture->md;

    /* Should be initialized to NULL */
    g_assert_cmpstr(modulemd_modulemetadata_get_name(md), ==, NULL);

    /* Assign a valid string */
    modulemd_modulemetadata_set_name(md, "ModuleName");
    g_assert_cmpstr(modulemd_modulemetadata_get_name(md), ==, "ModuleName");

    /* Reassign it to NULL */
    modulemd_modulemetadata_set_name(md, NULL);
    g_assert_cmpstr(modulemd_modulemetadata_get_name(md), ==, NULL);
}

static void
modulemd_modulemetadata_test_get_set_stream(ModuleMetadataFixture *fixture,
                                            gconstpointer user_data)
{
    ModulemdModuleMetadata *md = fixture->md;

    /* Should be initialized to NULL */
    g_assert_cmpstr(modulemd_modulemetadata_get_stream(md), ==, NULL);

    /* Assign a valid string */
    modulemd_modulemetadata_set_stream(md, "ModuleStream");
    g_assert_cmpstr(modulemd_modulemetadata_get_stream(md), ==, "ModuleStream");

    /* Reassign it to NULL */
    modulemd_modulemetadata_set_stream(md, NULL);
    g_assert_cmpstr(modulemd_modulemetadata_get_stream(md), ==, NULL);
}

static void
modulemd_modulemetadata_test_get_set_summary(ModuleMetadataFixture *fixture,
                                             gconstpointer user_data)
{
    ModulemdModuleMetadata *md = fixture->md;

    /* Should be initialized to NULL */
    g_assert_cmpstr(modulemd_modulemetadata_get_summary(md), ==, NULL);

    /* Assign a valid string */
    modulemd_modulemetadata_set_summary(md, "ModuleSummary");
    g_assert_cmpstr(modulemd_modulemetadata_get_summary(md), ==, "ModuleSummary");

    /* Reassign it to NULL */
    modulemd_modulemetadata_set_summary(md, NULL);
    g_assert_cmpstr(modulemd_modulemetadata_get_summary(md), ==, NULL);
}

static void
modulemd_modulemetadata_test_get_set_tracker(ModuleMetadataFixture *fixture,
                                             gconstpointer user_data)
{
    ModulemdModuleMetadata *md = fixture->md;

    /* Should be initialized to NULL */
    g_assert_cmpstr(modulemd_modulemetadata_get_tracker(md), ==, NULL);

    /* Assign a valid string */
    modulemd_modulemetadata_set_tracker(md, "ModuleTracker");
    g_assert_cmpstr(modulemd_modulemetadata_get_tracker(md), ==, "ModuleTracker");

    /* Reassign it to NULL */
    modulemd_modulemetadata_set_tracker(md, NULL);
    g_assert_cmpstr(modulemd_modulemetadata_get_tracker(md), ==, NULL);
}

static void
modulemd_modulemetadata_test_get_set_version(ModuleMetadataFixture *fixture,
                                             gconstpointer user_data)
{
    ModulemdModuleMetadata *md = fixture->md;

    /* Should be initialized to 0 */
    g_assert_cmpuint(modulemd_modulemetadata_get_version(md), ==, 0);

    /* Assign a valid version */
    modulemd_modulemetadata_set_version(md, 1);
    g_assert_cmpuint(modulemd_modulemetadata_get_version(md), ==, 1);

    /* Reassign it to 0 */
    modulemd_modulemetadata_set_version(md, 0);
    g_assert_cmpuint(modulemd_modulemetadata_get_version(md), ==, 0);
}

int
main (int argc, char *argv[])
{

    setlocale (LC_ALL, "");

    g_test_init (&argc, &argv, NULL);
    g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

    // Define the tests.

    g_test_add ("/modulemd/modulemetadata/test_get_set_buildrequires",
                ModuleMetadataFixture, NULL,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_get_set_buildrequires,
                modulemd_modulemetadata_tear_down);

    g_test_add ("/modulemd/modulemetadata/test_get_set_community",
                ModuleMetadataFixture, NULL,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_get_set_community,
                modulemd_modulemetadata_tear_down);

    ModuleMetadataPropString community;
    community.property_name = "community";
    community.test_str = "MyCommunity";
    g_test_add ("/modulemd/modulemetadata/test_prop_community",
                ModuleMetadataFixture, &community,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_string_prop,
                modulemd_modulemetadata_tear_down);


    g_test_add ("/modulemd/modulemetadata/test_get_set_description",
                ModuleMetadataFixture, NULL,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_get_set_description,
                modulemd_modulemetadata_tear_down);

    ModuleMetadataPropString desc;
    desc.property_name = "description";
    desc.test_str = "MyDescription";
    g_test_add ("/modulemd/modulemetadata/test_prop_description",
                ModuleMetadataFixture, &desc,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_string_prop,
                modulemd_modulemetadata_tear_down);

    g_test_add ("/modulemd/modulemetadata/test_get_set_documentation",
                ModuleMetadataFixture, NULL,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_get_set_documentation,
                modulemd_modulemetadata_tear_down);

    ModuleMetadataPropString doc;
    doc.property_name = "documentation";
    doc.test_str = "MyDocumentation";
    g_test_add ("/modulemd/modulemetadata/test_prop_documentation",
                ModuleMetadataFixture, &doc,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_string_prop,
                modulemd_modulemetadata_tear_down);

    g_test_add ("/modulemd/modulemetadata/test_get_set_mdversion",
                ModuleMetadataFixture, NULL,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_get_set_mdversion,
                modulemd_modulemetadata_tear_down);

    g_test_add ("/modulemd/modulemetadata/test_get_set_name",
                ModuleMetadataFixture, NULL,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_get_set_name,
                modulemd_modulemetadata_tear_down);

    ModuleMetadataPropString name;
    name.property_name = "name";
    name.test_str = "MyName";
    g_test_add ("/modulemd/modulemetadata/test_prop_name",
                ModuleMetadataFixture, &name,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_string_prop,
                modulemd_modulemetadata_tear_down);

    g_test_add ("/modulemd/modulemetadata/test_get_set_stream",
                ModuleMetadataFixture, NULL,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_get_set_stream,
                modulemd_modulemetadata_tear_down);

    ModuleMetadataPropString stream;
    stream.property_name = "stream";
    stream.test_str = "MyStream";
    g_test_add ("/modulemd/modulemetadata/test_prop_stream",
                ModuleMetadataFixture, &stream,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_string_prop,
                modulemd_modulemetadata_tear_down);

    g_test_add ("/modulemd/modulemetadata/test_get_set_summary",
                ModuleMetadataFixture, NULL,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_get_set_summary,
                modulemd_modulemetadata_tear_down);

    ModuleMetadataPropString summary;
    summary.property_name = "summary";
    summary.test_str = "MySummary";
    g_test_add ("/modulemd/modulemetadata/test_prop_summary",
                ModuleMetadataFixture, &summary,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_string_prop,
                modulemd_modulemetadata_tear_down);

    g_test_add ("/modulemd/modulemetadata/test_get_set_tracker",
                ModuleMetadataFixture, NULL,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_get_set_tracker,
                modulemd_modulemetadata_tear_down);

    ModuleMetadataPropString tracker;
    tracker.property_name = "tracker";
    tracker.test_str = "MyTracker";
    g_test_add ("/modulemd/modulemetadata/test_prop_tracker",
                ModuleMetadataFixture, &tracker,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_string_prop,
                modulemd_modulemetadata_tear_down);

    g_test_add ("/modulemd/modulemetadata/test_get_set_version",
                ModuleMetadataFixture, NULL,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_get_set_version,
                modulemd_modulemetadata_tear_down);

    return g_test_run ();
}
