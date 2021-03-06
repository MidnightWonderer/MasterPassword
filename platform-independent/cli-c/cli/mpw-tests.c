#include <stdio.h>
#include <stdlib.h>

#define ftl(...) do { fprintf( stderr, __VA_ARGS__ ); exit(2); } while (0)

#include "mpw-algorithm.h"
#include "mpw-util.h"

#include "mpw-tests-util.h"

int main(int argc, char *const argv[]) {

    int failedTests = 0;

    xmlNodePtr tests = xmlDocGetRootElement( xmlParseFile( "mpw_tests.xml" ) );
    if (!tests) {
        ftl( "Couldn't find test case: mpw_tests.xml\n" );
        abort();
    }

    for (xmlNodePtr testCase = tests->children; testCase; testCase = testCase->next) {
        if (testCase->type != XML_ELEMENT_NODE || xmlStrcmp( testCase->name, BAD_CAST "case" ) != 0)
            continue;

        // Read in the test case.
        xmlChar *id = mpw_xmlTestCaseString( testCase, "id" );
        MPAlgorithmVersion algorithm = (MPAlgorithmVersion)mpw_xmlTestCaseInteger( testCase, "algorithm" );
        xmlChar *fullName = mpw_xmlTestCaseString( testCase, "fullName" );
        xmlChar *masterPassword = mpw_xmlTestCaseString( testCase, "masterPassword" );
        xmlChar *keyID = mpw_xmlTestCaseString( testCase, "keyID" );
        xmlChar *siteName = mpw_xmlTestCaseString( testCase, "siteName" );
        MPCounterValue siteCounter = (MPCounterValue)mpw_xmlTestCaseInteger( testCase, "siteCounter" );
        xmlChar *resultTypeString = mpw_xmlTestCaseString( testCase, "resultType" );
        xmlChar *keyPurposeString = mpw_xmlTestCaseString( testCase, "keyPurpose" );
        xmlChar *keyContext = mpw_xmlTestCaseString( testCase, "keyContext" );
        xmlChar *result = mpw_xmlTestCaseString( testCase, "result" );

        MPResultType resultType = mpw_typeWithName( (char *)resultTypeString );
        MPKeyPurpose keyPurpose = mpw_purposeWithName( (char *)keyPurposeString );

        // Run the test case.
        fprintf( stdout, "test case %s... ", id );
        if (!xmlStrlen( result )) {
            fprintf( stdout, "abstract.\n" );
            continue;
        }

        // 1. calculate the master key.
        MPMasterKey masterKey = mpw_masterKey(
                (char *)fullName, (char *)masterPassword, algorithm );
        if (!masterKey) {
            ftl( "Couldn't derive master key.\n" );
            continue;
        }

        // 2. calculate the site password.
        const char *sitePassword = mpw_siteResult(
                masterKey, (char *)siteName, siteCounter, keyPurpose, (char *)keyContext, resultType, NULL, algorithm );
        mpw_free( masterKey, MPMasterKeySize );
        if (!sitePassword) {
            ftl( "Couldn't derive site password.\n" );
            continue;
        }

        // Check the result.
        if (xmlStrcmp( result, BAD_CAST sitePassword ) == 0)
            fprintf( stdout, "pass.\n" );

        else {
            ++failedTests;
            fprintf( stdout, "FAILED!  (got %s != expected %s)\n", sitePassword, result );
        }

        // Free test case.
        mpw_free_string( sitePassword );
        xmlFree( id );
        xmlFree( fullName );
        xmlFree( masterPassword );
        xmlFree( keyID );
        xmlFree( siteName );
        xmlFree( resultTypeString );
        xmlFree( keyPurposeString );
        xmlFree( keyContext );
        xmlFree( result );
    }

    return failedTests;
}
