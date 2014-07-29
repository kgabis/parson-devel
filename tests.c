/*
 Parson ( http://kgabis.github.com/parson/ )
 Copyright (c) 2012 - 2014 Krzysztof Gabis
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

#include "parson.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define TEST(A) printf("%-72s-",#A);              \
                if(A){puts(" OK");tests_passed++;} \
                else{puts(" FAIL");tests_failed++;}
#define STREQ(A, B) ((A) && (B) ? strcmp((A), (B)) == 0 : 0)

#define EPSILON (0.000001)

void test_suite_1(void);
void test_suite_2(JSON_Value *value);
void test_suite_2_no_comments(void);
void test_suite_2_with_comments(void);
void test_suite_3(void);
void test_suite_4(void);
void test_suite_5(void);

char *read_file(const char *filename);
void print_commits_info(const char *username, const char *repo);
int  are_values_equal(const JSON_Value *a, const JSON_Value *b);

static int tests_passed;
static int tests_failed;

int main() {
    /* Example function from readme file:       */
    /* print_commits_info("torvalds", "linux"); */
    test_suite_1();
    test_suite_2_no_comments();
    test_suite_2_with_comments();
    test_suite_3();
    test_suite_4();
    test_suite_5();
    printf("Tests failed: %d\n", tests_failed);
    printf("Tests passed: %d\n", tests_passed);
    return 0;
}

/* 3 test files from json.org */
void test_suite_1(void) {
    JSON_Value *val;
    TEST((val = json_parse_file("tests/test_1_1.txt")) != NULL);
    TEST(are_values_equal(json_parse_string(json_serialize(val)), val));
    if (val) { json_value_free(val);
    }
    TEST((val = json_parse_file("tests/test_1_2.txt")) != NULL);
    TEST(are_values_equal(json_parse_string(json_serialize(val)), val));

    if (val) { json_value_free(val); }
    TEST((val = json_parse_file("tests/test_1_3.txt")) != NULL);
    TEST(are_values_equal(json_parse_string(json_serialize(val)), val));
    if (val) { json_value_free(val); }
    
    TEST((val = json_parse_file_with_comments("tests/test_1_1.txt")) != NULL);
    TEST(are_values_equal(json_parse_string(json_serialize(val)), val));
    if (val) { json_value_free(val); }
    TEST((val = json_parse_file_with_comments("tests/test_1_2.txt")) != NULL);
    TEST(are_values_equal(json_parse_string(json_serialize(val)), val));
    if (val) { json_value_free(val); }
    TEST((val = json_parse_file_with_comments("tests/test_1_3.txt")) != NULL);
    TEST(are_values_equal(json_parse_string(json_serialize(val)), val));
    if (val) { json_value_free(val); }

}

/* Testing correctness of parsed values */
void test_suite_2(JSON_Value *root_value) {
    JSON_Object *root_object;
    JSON_Array *array;
    size_t i;
    TEST(root_value);
    TEST(json_value_get_type(root_value) == JSONObject);
    root_object = json_value_get_object(root_value);
    TEST(STREQ(json_object_get_string(root_object, "string"), "lorem ipsum"));
    TEST(STREQ(json_object_get_string(root_object, "utf string"), "lorem ipsum"));
    TEST(STREQ(json_object_get_string(root_object, "utf-8 string"), "あいうえお"));
    TEST(STREQ(json_object_get_string(root_object, "surrogate string"), "lorem𝄞ipsum𝍧lorem"));
    TEST(json_object_get_number(root_object, "positive one") == 1.0);
    TEST(json_object_get_number(root_object, "negative one") == -1.0);
    TEST(json_object_get_number(root_object, "hard to parse number") == -0.000314);
    TEST(json_object_get_boolean(root_object, "boolean true") == 1);
    TEST(json_object_get_boolean(root_object, "boolean false") == 0);
    TEST(json_value_get_type(json_object_get_value(root_object, "null")) == JSONNull);
    
    array = json_object_get_array(root_object, "string array");
    if (array != NULL && json_array_get_count(array) > 1) {
        TEST(STREQ(json_array_get_string(array, 0), "lorem"));
        TEST(STREQ(json_array_get_string(array, 1), "ipsum"));
    } else {
        tests_failed++;
    }
    
    array = json_object_get_array(root_object, "x^2 array");
    if (array != NULL) {
        for (i = 0; i < json_array_get_count(array); i++) {
            TEST(json_array_get_number(array, i) == (i * i));
        }
    } else {
        tests_failed++;
    }
    
    TEST(json_object_get_array(root_object, "non existent array") == NULL);
    TEST(STREQ(json_object_dotget_string(root_object, "object.nested string"), "str"));
    TEST(json_object_dotget_boolean(root_object, "object.nested true") == 1);
    TEST(json_object_dotget_boolean(root_object, "object.nested false") == 0);
    TEST(json_object_dotget_value(root_object, "object.nested null") != NULL);
    TEST(json_object_dotget_number(root_object, "object.nested number") == 123);
    
    TEST(json_object_dotget_value(root_object, "should.be.null") == NULL);
    TEST(json_object_dotget_value(root_object, "should.be.null.") == NULL);
    TEST(json_object_dotget_value(root_object, ".") == NULL);
    TEST(json_object_dotget_value(root_object, "") == NULL);
    
    array = json_object_dotget_array(root_object, "object.nested array");
    if (array != NULL && json_array_get_count(array) > 1) {
        TEST(STREQ(json_array_get_string(array, 0), "lorem"));
        TEST(STREQ(json_array_get_string(array, 1), "ipsum"));
    } else {
        tests_failed++;
    }
    TEST(json_object_dotget_boolean(root_object, "nested true"));
    
    TEST(STREQ(json_object_get_string(root_object, "/**/"), "comment"));
    TEST(STREQ(json_object_get_string(root_object, "//"), "comment"));
    TEST(STREQ(json_object_get_string(root_object, "url"), "https://www.example.com/search?q=12345"));
    TEST(STREQ(json_object_get_string(root_object, "escaped chars"), "\" \\ /"));
}

void test_suite_2_no_comments(void) {
    const char *filename = "tests/test_2.txt";
    JSON_Value *root_value = NULL;
    printf("Testing %s:\n", filename);
    root_value = json_parse_file(filename);
    test_suite_2(root_value);
    TEST(are_values_equal(root_value, json_parse_string(json_serialize(root_value))));
    json_value_free(root_value);
}

void test_suite_2_with_comments(void) {
    const char *filename = "tests/test_2_comments.txt";
    JSON_Value *root_value = NULL;
    printf("Testing %s:\n", filename);
    root_value = json_parse_file_with_comments(filename);
    test_suite_2(root_value);
    TEST(are_values_equal(root_value, json_parse_string(json_serialize(root_value))));
    json_value_free(root_value);
}

/* Testing values, on which parsing should fail */
void test_suite_3(void) {
    char nested_20x[] = "[[[[[[[[[[[[[[[[[[[[\"hi\"]]]]]]]]]]]]]]]]]]]]";
    puts("Testing invalid strings:");
    TEST(json_parse_string(NULL) == NULL);
    TEST(json_parse_string("") == NULL); /* empty string */
    TEST(json_parse_string("[\"lorem\",]") == NULL);
    TEST(json_parse_string("{\"lorem\":\"ipsum\",}") == NULL);
    TEST(json_parse_string("{lorem:ipsum}") == NULL);
    TEST(json_parse_string("[,]") == NULL);
    TEST(json_parse_string("[,") == NULL);
    TEST(json_parse_string("[") == NULL);
    TEST(json_parse_string("]") == NULL);
    TEST(json_parse_string("{\"a\":0,\"a\":0}") == NULL); /* duplicate keys */
    TEST(json_parse_string("{:,}") == NULL);
    TEST(json_parse_string("{,}") == NULL);
    TEST(json_parse_string("{,") == NULL);
    TEST(json_parse_string("{:") == NULL);
    TEST(json_parse_string("{") == NULL);
    TEST(json_parse_string("}") == NULL);
    TEST(json_parse_string("x") == NULL);
    TEST(json_parse_string("\"string\"") == NULL);
    TEST(json_parse_string("{:\"no name\"}") == NULL);
    TEST(json_parse_string("[,\"no first value\"]") == NULL);
    TEST(json_parse_string("[\"\\u00zz\"]") == NULL); /* invalid utf value */
    TEST(json_parse_string("[\"\\u00\"]") == NULL); /* invalid utf value */
    TEST(json_parse_string("[\"\\u\"]") == NULL); /* invalid utf value */
    TEST(json_parse_string("[\"\\\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\"\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\0\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\a\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\b\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\t\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\n\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\f\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\r\"]") == NULL); /* control character */
    TEST(json_parse_string(nested_20x) == NULL); /* too deep */
    TEST(json_parse_string("[0x2]") == NULL);    /* hex */
    TEST(json_parse_string("[0X2]") == NULL);    /* HEX */
    TEST(json_parse_string("[07]") == NULL);     /* octals */
    TEST(json_parse_string("[0070]") == NULL);
    TEST(json_parse_string("[07.0]") == NULL);
    TEST(json_parse_string("[-07]") == NULL);
    TEST(json_parse_string("[-007]") == NULL);
    TEST(json_parse_string("[-07.0]") == NULL);
    TEST(json_parse_string("[\"\\uDF67\\uD834\"]") == NULL); /* wrong order surrogate pair */
}

/* Test deep copy funtion */
void test_suite_4() {
    const char *filename = "tests/test_2.txt";
    JSON_Value *a = NULL, *a_copy = NULL;
    printf("Testing %s:\n", filename);
    a = json_parse_file(filename);
    TEST(are_values_equal(a, a)); /* test equality test */
    a_copy = json_value_deep_copy(a);
    TEST(a_copy != NULL);
    TEST(are_values_equal(a, a_copy));
}

/* Test building json values from scratch */
void test_suite_5(void) {
    JSON_Value *val_from_file = json_parse_file("tests/test_5.txt");
    
    JSON_Value *val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(val);
    json_object_set(obj, "first", json_value_init_string("John"));
    json_object_set(obj, "last", json_value_init_string("Doe"));
    json_object_set(obj, "age", json_value_init_number(25));
    json_object_set(obj, "registered", json_value_init_boolean(1));
    json_object_set(obj, "interests", json_value_init_array());
    json_array_append(json_object_get_array(obj, "interests"), json_value_init_string("Writing"));
    json_array_append(json_object_get_array(obj, "interests"), json_value_init_string("Mountain Biking"));
    json_array_set(json_object_get_array(obj, "interests"), 0, json_value_init_string("Reading"));
    json_object_dotset(obj, "favorites.color", json_value_init_string("blue"));
    json_object_dotset(obj, "favorites.sport", json_value_init_string("running"));
    json_object_dotset(obj, "favorites.fruit", json_value_init_string("apple"));
    json_object_dotremove(obj, "favorites.fruit");

    TEST(are_values_equal(val_from_file, val));
}

void print_commits_info(const char *username, const char *repo) {
    JSON_Value *root_value;
    JSON_Array *commits;
    JSON_Object *commit;
    size_t i;
    
    char curl_command[512];
    char cleanup_command[256];
    char output_filename[] = "commits.json";
    
    /* it ain't pretty, but it's not a libcurl tutorial */
    sprintf(curl_command, 
        "curl -s \"https://api.github.com/repos/%s/%s/commits\" > %s",
        username, repo, output_filename);
    sprintf(cleanup_command, "rm -f %s", output_filename);
    system(curl_command);
    
    /* parsing json and validating output */
    root_value = json_parse_file(output_filename);
    if (json_value_get_type(root_value) != JSONArray) {
        system(cleanup_command);
        return;
    }
    
    /* getting array from root value and printing commit info */
    commits = json_value_get_array(root_value);
    printf("%-10.10s %-10.10s %s\n", "Date", "SHA", "Author");
    for (i = 0; i < json_array_get_count(commits); i++) {
        commit = json_array_get_object(commits, i);
        printf("%.10s %.10s %s\n",
               json_object_dotget_string(commit, "commit.author.date"),
               json_object_get_string(commit, "sha"),
               json_object_dotget_string(commit, "commit.author.name"));
    }
    
    /* cleanup code */
    json_value_free(root_value);
    system(cleanup_command);
}

int are_values_equal(const JSON_Value *a, const JSON_Value *b) {
    JSON_Object *a_object = NULL, *b_object = NULL;
    JSON_Array *a_array = NULL, *b_array = NULL;
    const char *a_string = NULL, *b_string = NULL;
    const char *a_key = NULL, *b_key = NULL;
    size_t a_count = 0, b_count = 0, i = 0;
    JSON_Value_Type a_type, b_type;
    a_type = json_value_get_type(a);
    b_type = json_value_get_type(b);
    if (a_type != b_type)
        return 0;
    switch (a_type) {
        case JSONArray:
            a_array = json_value_get_array(a);
            b_array = json_value_get_array(b);
            a_count = json_array_get_count(a_array);
            b_count = json_array_get_count(b_array);
            if (a_count != b_count)
                return 0;
            for (i = 0; i < a_count; i++) {
                if (!are_values_equal(json_array_get_value(a_array, i), json_array_get_value(b_array, i)))
                    return 0;
            }
            return 1;
        case JSONObject:
            a_object = json_value_get_object(a);
            b_object = json_value_get_object(b);
            a_count = json_object_get_count(a_object);
            b_count = json_object_get_count(b_object);
            if (a_count != b_count)
                return 0;
            for (i = 0; i < a_count; i++) {
                a_key = json_object_get_name(a_object, i);
                b_key = json_object_get_name(b_object, i);
                if (!STREQ(a_key, b_key))
                    return 0;
                if (!are_values_equal(json_object_get_value(a_object, a_key), json_object_get_value(b_object, b_key)))
                    return 0;
            }
            return 1;
        case JSONString:
            a_string = json_value_get_string(a);
            b_string = json_value_get_string(b);
            return STREQ(a_string, b_string);
        case JSONBoolean:
            return json_value_get_boolean(a) == json_value_get_boolean(b);
        case JSONNumber:
            return fabs(json_value_get_number(a) - json_value_get_number(b)) < EPSILON;
        case JSONError:
            return 1;
        case JSONNull:
            return 1;
        default:
            return 1;
    }
}
