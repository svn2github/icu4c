/***********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-2004, International Business Machines Corporation
 * and others. All Rights Reserved.
 ***********************************************************************/

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING

#include "nmfmapts.h"

#include "unicode/numfmt.h"
#include "unicode/decimfmt.h"
#include "unicode/locid.h"
#include "unicode/unum.h"
#include "unicode/strenum.h"

// This is an API test, not a unit test.  It doesn't test very many cases, and doesn't
// try to test the full functionality.  It just calls each function in the class and
// verifies that it works on a basic level.

void IntlTestNumberFormatAPI::runIndexedTest( int32_t index, UBool exec, const char* &name, char* /*par*/ )
{
    if (exec) logln("TestSuite NumberFormatAPI");
    switch (index) {
        case 0: name = "NumberFormat API test"; 
                if (exec) {
                    logln("NumberFormat API test---"); logln("");
                    UErrorCode status = U_ZERO_ERROR;
                    Locale saveLocale;
                    Locale::setDefault(Locale::getEnglish(), status);
                    if(U_FAILURE(status)) {
                        errln("ERROR: Could not set default locale, test may not give correct results");
                    }
                    testAPI(/* par */);
                    Locale::setDefault(saveLocale, status);
                }
                break;
        case 1: name = "NumberFormatRegistration"; 
                if (exec) {
                    logln("NumberFormat Registration test---"); logln("");
                    UErrorCode status = U_ZERO_ERROR;
                    Locale saveLocale;
                    Locale::setDefault(Locale::getEnglish(), status);
                    if(U_FAILURE(status)) {
                        errln("ERROR: Could not set default locale, test may not give correct results");
                    }
                    testRegistration();
                    Locale::setDefault(saveLocale, status);
                }
                break;
        default: name = ""; break;
    }
}

/**
 * This test does round-trip testing (format -> parse -> format -> parse -> etc.) of
 * NumberFormat.
 */
void IntlTestNumberFormatAPI::testAPI(/* char* par */)
{
    UErrorCode status = U_ZERO_ERROR;

// ======= Test constructors

    logln("Testing NumberFormat constructors");

    NumberFormat *def = NumberFormat::createInstance(status);
    if(U_FAILURE(status)) {
        errln("ERROR: Could not create NumberFormat (default)");
    }

    status = U_ZERO_ERROR;
    NumberFormat *fr = NumberFormat::createInstance(Locale::getFrench(), status);
    if(U_FAILURE(status)) {
        errln("ERROR: Could not create NumberFormat (French)");
    }

    NumberFormat *cur = NumberFormat::createCurrencyInstance(status);
    if(U_FAILURE(status)) {
        errln("ERROR: Could not create NumberFormat (currency, default)");
    }

    status = U_ZERO_ERROR;
    NumberFormat *cur_fr = NumberFormat::createCurrencyInstance(Locale::getFrench(), status);
    if(U_FAILURE(status)) {
        errln("ERROR: Could not create NumberFormat (currency, French)");
    }

    NumberFormat *per = NumberFormat::createPercentInstance(status);
    if(U_FAILURE(status)) {
        errln("ERROR: Could not create NumberFormat (percent, default)");
    }

    status = U_ZERO_ERROR;
    NumberFormat *per_fr = NumberFormat::createPercentInstance(Locale::getFrench(), status);
    if(U_FAILURE(status)) {
        errln("ERROR: Could not create NumberFormat (percent, French)");
    }

// ======= Test equality

    logln("Testing equality operator");
    
    if( *per_fr == *cur_fr || ! ( *per_fr != *cur_fr) ) {
        errln("ERROR: == failed");
    }

// ======= Test various format() methods

    logln("Testing various format() methods");

    double d = -10456.0037;
    int32_t l = 100000000;
    Formattable fD(d);
    Formattable fL(l);

    UnicodeString res1, res2, res3, res4, res5, res6;
    FieldPosition pos1(0), pos2(0), pos3(0), pos4(0);
    
    res1 = cur_fr->format(d, res1);
    logln( (UnicodeString) "" + (int32_t) d + " formatted to " + res1);

    res2 = cur_fr->format(l, res2);
    logln((UnicodeString) "" + (int32_t) l + " formatted to " + res2);

    res3 = cur_fr->format(d, res3, pos1);
    logln( (UnicodeString) "" + (int32_t) d + " formatted to " + res3);

    res4 = cur_fr->format(l, res4, pos2);
    logln((UnicodeString) "" + (int32_t) l + " formatted to " + res4);

    status = U_ZERO_ERROR;
    res5 = cur_fr->format(fD, res5, pos3, status);
    if(U_FAILURE(status)) {
        errln("ERROR: format(Formattable [double]) failed");
    }
    logln((UnicodeString) "" + (int32_t) fD.getDouble() + " formatted to " + res5);

    status = U_ZERO_ERROR;
    res6 = cur_fr->format(fL, res6, pos4, status);
    if(U_FAILURE(status)) {
        errln("ERROR: format(Formattable [long]) failed");
    }
    logln((UnicodeString) "" + fL.getLong() + " formatted to " + res6);


// ======= Test parse()

    logln("Testing parse()");

    UnicodeString text("-10,456.0037");
    Formattable result1, result2, result3;
    ParsePosition pos(0), pos01(0);
    fr->parseObject(text, result1, pos);
    if(result1.getType() != Formattable::kDouble && result1.getDouble() != d) {
        errln("ERROR: Roundtrip failed (via parse()) for " + text);
    }
    logln(text + " parsed into " + (int32_t) result1.getDouble());

    fr->parse(text, result2, pos01);
    if(result2.getType() != Formattable::kDouble && result2.getDouble() != d) {
        errln("ERROR: Roundtrip failed (via parse()) for " + text);
    }
    logln(text + " parsed into " + (int32_t) result2.getDouble());

    status = U_ZERO_ERROR;
    fr->parse(text, result3, status);
    if(U_FAILURE(status)) {
        errln("ERROR: parse() failed");
    }
    if(result3.getType() != Formattable::kDouble && result3.getDouble() != d) {
        errln("ERROR: Roundtrip failed (via parse()) for " + text);
    }
    logln(text + " parsed into " + (int32_t) result3.getDouble());


// ======= Test getters and setters

    logln("Testing getters and setters");

    int32_t count = 0;
    const Locale *locales = NumberFormat::getAvailableLocales(count);
    logln((UnicodeString) "Got " + count + " locales" );
    for(int32_t i = 0; i < count; i++) {
        UnicodeString name(locales[i].getName(),"");
        logln(name);
    }

    fr->setParseIntegerOnly( def->isParseIntegerOnly() );
    if(fr->isParseIntegerOnly() != def->isParseIntegerOnly() ) {
        errln("ERROR: setParseIntegerOnly() failed");
    }

    fr->setGroupingUsed( def->isGroupingUsed() );
    if(fr->isGroupingUsed() != def->isGroupingUsed() ) {
        errln("ERROR: setGroupingUsed() failed");
    }

    fr->setMaximumIntegerDigits( def->getMaximumIntegerDigits() );
    if(fr->getMaximumIntegerDigits() != def->getMaximumIntegerDigits() ) {
        errln("ERROR: setMaximumIntegerDigits() failed");
    }

    fr->setMinimumIntegerDigits( def->getMinimumIntegerDigits() );
    if(fr->getMinimumIntegerDigits() != def->getMinimumIntegerDigits() ) {
        errln("ERROR: setMinimumIntegerDigits() failed");
    }

    fr->setMaximumFractionDigits( def->getMaximumFractionDigits() );
    if(fr->getMaximumFractionDigits() != def->getMaximumFractionDigits() ) {
        errln("ERROR: setMaximumFractionDigits() failed");
    }

    fr->setMinimumFractionDigits( def->getMinimumFractionDigits() );
    if(fr->getMinimumFractionDigits() != def->getMinimumFractionDigits() ) {
        errln("ERROR: setMinimumFractionDigits() failed");
    }


// ======= Test getStaticClassID()

    logln("Testing getStaticClassID()");

    status = U_ZERO_ERROR;
    NumberFormat *test = new DecimalFormat(status);
    if(U_FAILURE(status)) {
        errln("ERROR: Couldn't create a NumberFormat");
    }

    if(test->getDynamicClassID() != DecimalFormat::getStaticClassID()) {
        errln("ERROR: getDynamicClassID() didn't return the expected value");
    }

    delete test;
    delete def;
    delete fr;
    delete cur;
    delete cur_fr;
    delete per;
    delete per_fr;
}

#if !UCONFIG_NO_SERVICE

#define SRC_LOC Locale::getFrance()
#define SWAP_LOC Locale::getUS()

class NFTestFactory : public SimpleNumberFormatFactory {
    NumberFormat* currencyStyle;

public:
    NFTestFactory() 
        : SimpleNumberFormatFactory(SRC_LOC, TRUE)
    {
        UErrorCode status = U_ZERO_ERROR;
        currencyStyle = NumberFormat::createInstance(SWAP_LOC, status);
    }

    virtual ~NFTestFactory()
    {
        delete currencyStyle;
    }
    
    virtual NumberFormat* createFormat(const Locale& /* loc */, UNumberFormatStyle formatType)
    {
        if (formatType == UNUM_CURRENCY) {
            return (NumberFormat*)currencyStyle->clone();
        }
        return NULL;
    }

   virtual inline UClassID getDynamicClassID() const
   {
       return (UClassID)&gID;
   }

   static inline UClassID getStaticClassID()
   {
        return (UClassID)&gID;
   }

private:
   static char gID;
};

char NFTestFactory::gID = 0;
#endif

void
IntlTestNumberFormatAPI::testRegistration() 
{
#if !UCONFIG_NO_SERVICE
    UErrorCode status = U_ZERO_ERROR;
    
    NumberFormat* f0 = NumberFormat::createInstance(SWAP_LOC, status);
    NumberFormat* f1 = NumberFormat::createInstance(SRC_LOC, status);
    NumberFormat* f2 = NumberFormat::createCurrencyInstance(SRC_LOC, status);
    URegistryKey key = NumberFormat::registerFactory(new NFTestFactory(), status);
    NumberFormat* f3 = NumberFormat::createCurrencyInstance(SRC_LOC, status);
    NumberFormat* f3a = NumberFormat::createCurrencyInstance(SRC_LOC, status);
    NumberFormat* f4 = NumberFormat::createInstance(SRC_LOC, status);

    StringEnumeration* locs = NumberFormat::getAvailableLocales();

    UNumberFormat* uf3 = unum_open(UNUM_CURRENCY, NULL, 0, SRC_LOC.getName(),NULL, &status);
    UNumberFormat* uf4 = unum_open(UNUM_DEFAULT, NULL, 0, SRC_LOC.getName(), NULL, &status);

    const UnicodeString* res;
    for (res = locs->snext(status); res; res = locs->snext(status)) {
        logln(*res); // service is still in synch
    }

    NumberFormat::unregister(key, status); // restore for other tests
    NumberFormat* f5 = NumberFormat::createCurrencyInstance(SRC_LOC, status);
    UNumberFormat* uf5 = unum_open(UNUM_CURRENCY, NULL, 0, SRC_LOC.getName(),NULL, &status);
    
    float n = 1234.567f;
    UnicodeString res0, res1, res2, res3, res4, res5;
    UChar ures3[50];
    UChar ures4[50];
    UChar ures5[50];
    
    f0->format(n, res0);
    f1->format(n, res1);
    f2->format(n, res2);
    f3->format(n, res3);
    f4->format(n, res4);
    f5->format(n, res5);

    unum_formatDouble(uf3, n, ures3, 50, NULL, &status);
    unum_formatDouble(uf4, n, ures4, 50, NULL, &status);
    unum_formatDouble(uf5, n, ures5, 50, NULL, &status);

    logln((UnicodeString)"f0 swap int: " + res0);
    logln((UnicodeString)"f1 src int: " + res1);
    logln((UnicodeString)"f2 src cur: " + res2);
    logln((UnicodeString)"f3 reg cur: " + res3);
    logln((UnicodeString)"f4 reg int: " + res4);
    logln((UnicodeString)"f5 unreg cur: " + res5);
    log("uf3 reg cur: ");
    logln(ures3);
    log("uf4 reg int: ");
    logln(ures4);
    log("uf5 ureg cur: ");
    logln(ures5);
    
    if (f3 == f3a) {
        errln("did not get new instance from service");
    } else {
        delete f3a;
    }
    if (res3 != res0) {
        errln("registered service did not match");
    }
    if (res4 != res1) {
        errln("registered service did not inherit");
    }
    if (res5 != res2) {
        errln("unregistered service did not match original");
    }

    if (res0 != ures3) {
        errln("registered service did not match / unum");
    }
    if (res1 != ures4) {
        errln("registered service did not inherit / unum");
    }
    if (res2 != ures5) {
        errln("unregistered service did not match original / unum");
    }

    unum_close(uf5);
    delete f5;
    unum_close(uf4);
    unum_close(uf3);
    delete f4;
    delete f3;
    delete f2;
    delete f1;
    delete f0;

    for (res = locs->snext(status); res; res = locs->snext(status)) {
        errln(*res); // service should be out of synch
    }

    locs->reset(status); // now in synch again, we hope
    for (res = locs->snext(status); res; res = locs->snext(status)) {
        logln(*res);
    }

    delete locs;
#endif
}


#endif /* #if !UCONFIG_NO_FORMATTING */
