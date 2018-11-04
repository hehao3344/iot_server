#ifndef __XML_API_H
#define __XML_API_H
// only support parent and child node.

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XmlApiObject* XML_HANDLE;

typedef void  ( *xml_save_callback )( void *arg, const char *fileName );

// Functionality:
//   Initialize xml library.
// Parameters:
//   @[in]fileName: the xml file name.
// Return:
//   handle on success, NULL failed.
//
XML_HANDLE xml_create( const char * fileName );

// Functionality:
//   exit from xml library.
// Parameters:
//   @[in]hXml: return from xml_create().
// Return:
//   None.
//
void  xml_destroy( XML_HANDLE hXml );

// Functionality:
//   get string value.
// Parameters:
//   @[in]hXml: return from xml_create().
//   @[in]rootString:  parent node of xml file.
//   @[in]childString: child  node of xml file.
//   @[in]dftValue:    if can't find rootString/childString node, return dftValue.
// Return:
//   the rootString/childString "value" attribute(char *).
//
char *xml_get_string( XML_HANDLE hXml, const char *rootString, const char *childString, const char *dftValue );

// Functionality:
//   get integer value.
// Parameters:
//   @[in]hXml: return from xml_create().
//   @[in]rootString:  parent node of xml file.
//   @[in]childString: child  node of xml file.
//   @[in]dftValue:    if can't find rootString/childString node, return dftValue.
// Return:
//   the rootString/childString "value" attribute(integer).
//
int   xml_get_int( XML_HANDLE hXml, const char *rootString, const char *childString, int dftValue );

// Functionality:
//   Set callback of xml save file.
// Parameters:
//   @[in]cb:  callback of xml save.
//   @[in]arg: the param of callback.
// Return:
//   None.
//
void xml_set_callback( XML_HANDLE hXml, xml_save_callback cb, void *arg );

// Functionality:
//   set string value.
// Parameters:
//   @[in]hXml: return from xml_create().
//   @[in]rootString:  parent node of xml file.
//   @[in]childString: child  node of xml file.
//   @[in]value:       the setted value(char *) of rootString/childString.
// Return:
//   0: success, -1 failed.
//
int   xml_set_string( XML_HANDLE hXml, const char *rootString, const char *childString, const char *value );

// Functionality:
//   set integer value.
// Parameters:
//   @[in]hXml: return from xml_create().
//   @[in]rootString:  parent node of xml file.
//   @[in]childString: child  node of xml file.
//   @[in]value:       the setted value(int) of rootString/childString.
// Return:
//   0: success, -1 failed.
//
int   xml_set_int( XML_HANDLE hXml, const char *rootString, const char *childString, int value );

int xml_unit_test( void );

#ifdef __cplusplus
}
#endif

#endif // __XML_API_H
