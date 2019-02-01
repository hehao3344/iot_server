#include <stdio.h>

#include "tinyxml.h"
#include "xml_api.h"

typedef struct XmlApiObject
{
    TiXmlDocument *doc;
    char fileName[64];
    xml_save_callback cb;
    void *arg;
} XmlApiObject;

//static void validate_xml_file( const char *fileName );
static TiXmlElement* xml_find_element( XML_HANDLE hXml, const char *rootString, const char *childString, const char *value );

XML_HANDLE xml_create( const char * fileName )
{
    XmlApiObject *xmlObj = ( XmlApiObject * )calloc( 1, sizeof( XmlApiObject ) );
    if ( NULL == xmlObj )
    {
        printf("xml create failed, not enough memory \n");
        return NULL;
    }

    //validate_xml_file( fileName );

    xmlObj->doc = new TiXmlDocument( fileName );
    bool loadOkay = xmlObj->doc->LoadFile();
    if ( !loadOkay )
    {
        printf( "Could not load file '%s'. Error='%s'. Exiting.\n", fileName, xmlObj->doc->ErrorDesc() );
        delete xmlObj->doc;
        free( xmlObj );
        return NULL;
    }
    strcpy( xmlObj->fileName, fileName );

    return xmlObj;
}

void xml_set_callback( XML_HANDLE hXml, xml_save_callback cb, void *arg )
{
    if ( NULL == hXml )
    {
        return;
    }
    hXml->cb = cb;
    hXml->arg = arg;
}

char *xml_get_string( XML_HANDLE hXml, const char *rootString,
                        const char *childString, const char *dftValue )
{
    TiXmlElement* element = xml_find_element( hXml, rootString, childString, NULL );

    return ( char * )( ( NULL == element ) ? dftValue : ( char * )element->Attribute( "value" ) );
}

int xml_get_int( XML_HANDLE hXml, const char *rootString, const char *childString, int dftValue )
{
    int value = dftValue;
    TiXmlElement* element = xml_find_element( hXml, rootString, childString, NULL );

    if ( NULL != element )
    {
        element->Attribute( "value", &value );
    }

    return value;
}

int xml_set_string( XML_HANDLE hXml, const char *rootString, const char *childString, const char *value )
{
    int ret = -1;
    TiXmlElement* element = xml_find_element( hXml, rootString, childString, value );

    if ( NULL != element )
    {
        element->SetAttribute( "value", value );
        hXml->doc->SaveFile();
        ret = 0;
    }
    if ( NULL != hXml->cb )
    {
        hXml->cb( hXml->arg, hXml->fileName );
    }

    return ret;
}

int xml_set_int( XML_HANDLE hXml, const char *rootString, const char *childString, int value )
{
    char buffer[16];
    int ret = -1;

    memset( buffer, 0, sizeof( buffer ) );
    sprintf( buffer, "%d", value );
    TiXmlElement* element = xml_find_element( hXml, rootString, childString, buffer );
    if ( NULL != element )
    {
        element->SetAttribute( "value", value );
        hXml->doc->SaveFile();
        if ( NULL != hXml->cb )
        {
            hXml->cb( hXml->arg, hXml->fileName );
        }
        ret = 0;
    }

    return ret;
}

void  xml_destroy( XML_HANDLE hXml )
{
    if ( NULL != hXml )
    {
        hXml->doc->SaveFile();
        delete hXml->doc;
        free( hXml );
    }
}
//////////////////////////////////////////////////////////////////////////////////////
// static function.
// Note: if value is NULL, means is get cmd.
//////////////////////////////////////////////////////////////////////////////////////
static TiXmlElement* xml_find_element( XML_HANDLE hXml, const char *rootString, const char *childString, const char *value )
{
    TiXmlNode* rootNode = NULL;
    TiXmlElement* element = NULL;
    rootNode = hXml->doc->RootElement();
    while( ( NULL != rootNode ) && ( 0 != strcmp( rootString, rootNode->Value() ) ) )
    {
        rootNode = rootNode->NextSibling();
    }

    if ( ( NULL == rootNode ) && ( NULL != value ) )
    {
        printf("[xml]: can't find root node %s, create it \n", rootString);
        TiXmlElement rootText( rootString );
        TiXmlNode* curRootNode = hXml->doc->InsertEndChild( rootText );
        TiXmlElement childText( childString );
        TiXmlNode* childNode = curRootNode->ToElement()->InsertEndChild( childText );
        childNode->ToElement()->SetAttribute( "value", value );
        hXml->doc->SaveFile();
        return NULL;
    }
    else if ( ( NULL == rootNode ) && ( NULL == value ) )
    {
        return NULL;
    }

    // find the rootString node.
    element = rootNode->FirstChildElement();
    while ( ( NULL != element ) && ( 0 != strcmp( childString, element->Value() ) ) )
    {
        element = element->NextSiblingElement();
    }

    if ( NULL == element )
    {
        if ( NULL != value )
        {
            printf("[xml]: can't find child node %s, create it \n", childString);
            TiXmlElement chileText( childString );
            TiXmlNode* curNode = rootNode->ToElement()->InsertEndChild( chileText );
            curNode->ToElement()->SetAttribute( "value", value );
            hXml->doc->SaveFile();
        }
        else
        {
            printf("[xml]: can't find child node %s \n", childString);
        }
        return NULL;
    }


    return element;
}

int xml_unit_test( void )
{
    //char *value = NULL;
    //int  valInt = 0;

    XML_HANDLE hXml = NULL;

    hXml = xml_create( "Config.xml" );
    if ( hXml )
    {
        xml_set_int( hXml, "Break", "test", 2343 );
        xml_destroy( hXml );
    }

    return 0;
}

#if 0
static void validate_xml_file( const char *fileName )
{
    int bytes = 0;

    FILE* fp = NULL;
    fp = fopen( fileName, "rb" );
    if ( NULL != fp )
    {
        fseek( fp, 0, SEEK_END );
        bytes = ftell( fp );
        fclose( fp );
    }

    if ( bytes <= 16 )
    {
        printf( "the length of %s invalid, try to recover \n", fileName );
        fp = fopen( fileName, "wb" );
        if ( NULL != fp )
        {
            char *str = (char*)"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\r\n";
            fwrite( str, 1, strlen(str), fp );
            fclose( fp );
        }
    }
}
#endif
