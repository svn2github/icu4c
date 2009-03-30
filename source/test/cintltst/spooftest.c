/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 2009, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/
/********************************************************************************
*
* File spooftest.c
*
*********************************************************************************/
/*C API TEST for the uspoof Unicode Indentifier Spoofing and Security API */
/**
*   This is an API test for ICU spoof detection in plain C.  It doesn't test very many cases, and doesn't
*   try to test the full functionality.  It just calls each function and verifies that it
*   works on a basic level.
*
*   More complete testing of spoof detection functionality is done with the C++ tests.
**/

#include "unicode/utypes.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "unicode/uspoof.h"
#include "unicode/ustring.h"
#include "unicode/uset.h"
#include "cintltst.h"

#define TEST_ASSERT_SUCCESS(status) {if (U_FAILURE(status)) { \
    log_err("Failure at file %s, line %d, error = %s\n", __FILE__, __LINE__, u_errorName(status));}}

#define TEST_ASSERT(expr) {if ((expr)==FALSE) { \
log_err("Test Failure at file %s, line %d: \"%s\" is false.\n", __FILE__, __LINE__, #expr);};}

#define TEST_ASSERT_EQ(a, b) { if ((a) != (b)) { \
    log_err("Test Failure at file %s, line %d: \"%s\" (%d) != \"%s\" (%d) \n", \
             __FILE__, __LINE__, #a, (a), #b, (b)); }}

#define TEST_ASSERT_NE(a, b) { if ((a) == (b)) { \
    log_err("Test Failure at file %s, line %d: \"%s\" (%d) == \"%s\" (%d) \n", \
             __FILE__, __LINE__, #a, (a), #b, (b)); }}


/*
 *   TEST_SETUP and TEST_TEARDOWN
 *         macros to handle the boilerplate around setting up test case.
 *         Put arbitrary test code between SETUP and TEARDOWN.
 *         "sc" is the ready-to-go  SpoofChecker for use in the tests.
 */
#define TEST_SETUP {  \
    UErrorCode status = U_ZERO_ERROR; \
    USpoofChecker *sc;     \
    sc = uspoof_open(&status);  \
    TEST_ASSERT_SUCCESS(status);   \
    if (U_SUCCESS(status)){

#define TEST_TEARDOWN  \
    }  \
    TEST_ASSERT_SUCCESS(status);  \
    uspoof_close(sc);  \
}


static void test_assert_string(const char *expected, const UChar *actual, UBool nulTerm, const char *file, int line) {
     char     buf_inside_macro[120];
     int32_t  len = (int32_t)strlen(expected);
     UBool    success;
     if (nulTerm) {
         u_austrncpy(buf_inside_macro, (actual), len+1);
         buf_inside_macro[len+2] = 0;
         success = (strcmp((expected), buf_inside_macro) == 0);
     } else {
         u_austrncpy(buf_inside_macro, (actual), len);
         buf_inside_macro[len+1] = 0;
         success = (strncmp((expected), buf_inside_macro, len) == 0);
     }
     if (success == FALSE) {
         log_err("Failure at file %s, line %d, expected \"%s\", got \"%s\"\n",
             file, line, (expected), buf_inside_macro);
     }
}

#define TEST_ASSERT_STRING(expected, actual, nulTerm) test_assert_string(expected, actual, nulTerm, __FILE__, __LINE__)



static void TestUSpoofCAPI(void);

void addUSpoofTest(TestNode** root);

void addUSpoofTest(TestNode** root)
{
    addTest(root, &TestUSpoofCAPI, "uspoof/TestUSpoofCAPI");
}

/*
 *  Identifiers, one good and one bad, for verifying that a spoof checker is minimally alive and working.
 */
const UChar goodLatin[] = {(UChar)0x75, (UChar)0x77, 0};     /* "uw", all ASCII             */
                                                          /*   (not confusable)          */
const UChar badId[]  = {(UChar)0x73, (UChar)0x0441, 0};   /* "sc", with Cyrillic 'c'     */
                                                          /*   (mixed script, confusable */

const UChar goodCyrl[] = {(UChar)0x438, (UChar)0x43B, 0};   /* Plain lower case Cyrillic letters,
                                                             no latin confusables */
        
const UChar goodGreek[] = {(UChar)0x3c0, (UChar)0x3c6, 0};  /* Plain lower case Greek letters */


/*
 *   Spoof Detction C API Tests
 */
static void TestUSpoofCAPI(void) {

    /*
     *  basic uspoof_open().
     */
    {
        USpoofChecker *sc;
        UErrorCode  status = U_ZERO_ERROR;
        sc = uspoof_open(&status);
        TEST_ASSERT_SUCCESS(status);
        if (U_FAILURE(status)) {
            /* If things are so broken that we can't even open a default spoof checker,  */
            /*   don't even try the rest of the tests.  They would all fail.             */
            return;
        }
        uspoof_close(sc);
    }

    
        
    /*
     *  Test Open from source rules.
    */
    TEST_SETUP
    const char *dataSrcDir;
    char       *fileName;
    char       *confusables;
    int         confusablesLength;
    char       *confusablesWholeScript;
    int         confusablesWholeScriptLength;
    FILE       *f;
    UParseError pe;
    int32_t     errType;
    USpoofChecker *rsc;
    
    dataSrcDir = ctest_dataSrcDir();
    fileName = malloc(strlen(dataSrcDir) + 100);
    strcpy(fileName, dataSrcDir);
    strcat(fileName, U_FILE_SEP_STRING "unidata" U_FILE_SEP_STRING "confusables.txt");
    f = fopen(fileName, "r");
    TEST_ASSERT_NE(f, NULL);
    confusables = malloc(3000000);
    confusablesLength = fread(confusables, 1, 3000000, f);
    fclose(f);

    
    strcpy(fileName, dataSrcDir);
    strcat(fileName, U_FILE_SEP_STRING "unidata" U_FILE_SEP_STRING "confusablesWholeScript.txt");
    f = fopen(fileName, "r");
    TEST_ASSERT_NE(f, NULL);
    confusablesWholeScript = malloc(1000000);
    confusablesWholeScriptLength = fread(confusablesWholeScript, 1, 1000000, f);
    fclose(f);

    rsc = uspoof_openFromSource(confusables, confusablesLength,
                                              confusablesWholeScript, confusablesWholeScriptLength,
                                              &errType, &pe, &status);
    TEST_ASSERT_SUCCESS(status);

    free(confusablesWholeScript);
    free(confusables);
    free(fileName);
    uspoof_close(rsc);
    /*  printf("ParseError Line is %d\n", pe.line);  */
    TEST_TEARDOWN;


    /*
     * openFromSerialized and serialize
    */
    TEST_SETUP
        int32_t        serializedSize = 0;
        int32_t        actualLength = 0;
        char           *buf;
        USpoofChecker  *sc2;
        int32_t         checkResults;

        
        serializedSize = uspoof_serialize(sc, NULL, 0, &status);
        TEST_ASSERT_EQ(status, U_BUFFER_OVERFLOW_ERROR);
        TEST_ASSERT(serializedSize > 0);

        /* Serialize the default spoof checker */
        status = U_ZERO_ERROR;
        buf = (char *)malloc(serializedSize + 10);
        TEST_ASSERT(buf != NULL);
        buf[serializedSize] = 42;
        uspoof_serialize(sc, buf, serializedSize, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT_EQ(42, buf[serializedSize]);

        /* Create a new spoof checker from the freshly serialized data */
        sc2 = uspoof_openFromSerialized(buf, serializedSize+10, &actualLength, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT_NE(NULL, sc2);
        TEST_ASSERT_EQ(serializedSize, actualLength);

        /* Verify that the new spoof checker at least wiggles */
        checkResults = uspoof_check(sc2, goodLatin, -1, NULL, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT_EQ(0, checkResults);

        checkResults = uspoof_check(sc2, badId, -1, NULL, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT_EQ(USPOOF_SINGLE_SCRIPT | USPOOF_MIXED_SCRIPT_CONFUSABLE, checkResults);

        uspoof_close(sc2);
        free(buf);
    TEST_TEARDOWN;
        
        
        
    /*
     * Set & Get Check Flags
    */
    TEST_SETUP
        int32_t t;
        uspoof_setChecks(sc, USPOOF_ALL_CHECKS, &status);
        TEST_ASSERT_SUCCESS(status);
        t = uspoof_getChecks(sc, &status);
        TEST_ASSERT_EQ(t, USPOOF_ALL_CHECKS);
    
        uspoof_setChecks(sc, 0, &status);
        TEST_ASSERT_SUCCESS(status);
        t = uspoof_getChecks(sc, &status);
        TEST_ASSERT_EQ(0, t);
        
        uspoof_setChecks(sc,
                        USPOOF_WHOLE_SCRIPT_CONFUSABLE | USPOOF_MIXED_SCRIPT_CONFUSABLE | USPOOF_ANY_CASE,
                        &status);
        TEST_ASSERT_SUCCESS(status);
        t = uspoof_getChecks(sc, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT_EQ(USPOOF_WHOLE_SCRIPT_CONFUSABLE | USPOOF_MIXED_SCRIPT_CONFUSABLE | USPOOF_ANY_CASE, t);
    TEST_TEARDOWN;

    /*
    * get & setAllowedChars
    */
    TEST_SETUP
        USet *us;
        const USet *uset;

        uset = uspoof_getAllowedChars(sc, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT(uset_isFrozen(uset));
        us = uset_open((UChar32)0x41, (UChar32)0x5A);   /*  [A-Z]  */
        uspoof_setAllowedChars(sc, us, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT_NE(us, uspoof_getAllowedChars(sc, &status));
        TEST_ASSERT(uset_equals(us, uspoof_getAllowedChars(sc, &status)));
        TEST_ASSERT_SUCCESS(status);
        uset_close(us);
    TEST_TEARDOWN;

    /*
    *  clone()
    */

    TEST_SETUP
        USpoofChecker *clone1 = NULL;
        USpoofChecker *clone2 = NULL;
        int32_t        checkResults = 0;
        
        clone1 = uspoof_clone(sc, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT_NE(clone1, sc);

        clone2 = uspoof_clone(clone1, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT_NE(clone2, clone1);

        uspoof_close(clone1);
        
        /* Verify that the cloned spoof checker is alive */
        checkResults = uspoof_check(clone2, goodLatin, -1, NULL, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT_EQ(0, checkResults);

        checkResults = uspoof_check(clone2, badId, -1, NULL, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT_EQ(USPOOF_SINGLE_SCRIPT | USPOOF_MIXED_SCRIPT_CONFUSABLE, checkResults);
        uspoof_close(clone2);
    TEST_TEARDOWN;

    /*
     *  get & set Checks
    */
    TEST_SETUP
        int32_t   checks;
        int32_t   checks2;
        int32_t   checkResults;

        checks = uspoof_getChecks(sc, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT_EQ(USPOOF_ALL_CHECKS, checks);

        checks &= ~(USPOOF_SINGLE_SCRIPT | USPOOF_MIXED_SCRIPT_CONFUSABLE);
        uspoof_setChecks(sc, checks, &status);
        TEST_ASSERT_SUCCESS(status);
        checks2 = uspoof_getChecks(sc, &status);
        TEST_ASSERT_EQ(checks, checks2);

        /* The checks that were disabled just above are the same ones that the "badId" test fails.
            So with those tests gone checking that Identifier should now succeed */
        checkResults = uspoof_check(sc, badId, -1, NULL, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT_EQ(0, checkResults);
    TEST_TEARDOWN;
        
    /*
     *  AllowedLoacles
     */

    TEST_SETUP
        const char  *allowedLocales;
        int32_t  checkResults;

        /* Default allowed locales list should be empty */
        allowedLocales = uspoof_getAllowedLocales(sc, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT(strcmp("", allowedLocales) == 0)

        /* Allow en and ru, which should enable Latin and Cyrillic only to pass */
        uspoof_setAllowedLocales(sc, "en, ru_RU", &status);
        TEST_ASSERT_SUCCESS(status);
        allowedLocales = uspoof_getAllowedLocales(sc, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT(strstr(allowedLocales, "en") != NULL);
        TEST_ASSERT(strstr(allowedLocales, "ru") != NULL);

        /* Limit checks to USPOOF_CHAR_LIMIT.  Some of the test data has whole script confusables also,
         * which we don't want to see in this test. */
        uspoof_setChecks(sc, USPOOF_CHAR_LIMIT, &status);
        TEST_ASSERT_SUCCESS(status);

        checkResults = uspoof_check(sc, goodLatin, -1, NULL, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT_EQ(0, checkResults);
        
        checkResults = uspoof_check(sc, goodGreek, -1, NULL, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT_EQ(USPOOF_CHAR_LIMIT, checkResults);

        checkResults = uspoof_check(sc, goodCyrl, -1, NULL, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT_EQ(0, checkResults);

        /* Reset with an empty locale list, which should allow all characters to pass */
        uspoof_setAllowedLocales(sc, " ", &status);
        TEST_ASSERT_SUCCESS(status);

        checkResults = uspoof_check(sc, goodGreek, -1, NULL, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT_EQ(0, checkResults);
    TEST_TEARDOWN;
}




