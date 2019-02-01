#include <stdio.h>

#include "tinyxml.h"
#include "xml_msg.h"

typedef struct XmlMsgObject
{
    TiXmlDocument *doc;
} XmlMsgObject;

XML_MSG_HANDLE xml_msg_create( char *buffer )
{
    XmlMsgObject *handle = ( XmlMsgObject * )calloc( 1, sizeof( XmlMsgObject ) );
    if ( NULL == handle )
    {
        printf("xml create failed, not enough memory \n");
        return NULL;
    }

    handle->doc = new TiXmlDocument();
    handle->doc->Parse( buffer );

    return handle;
}

void  xml_msg_destroy( XML_MSG_HANDLE handle )
{
    if ( NULL == handle )
    {
        return;
    }

    delete handle->doc;
    free( handle );
}

int xml_msg_get_type_ns( XML_MSG_HANDLE handle, char *type, char *ns )
{
    char *tmp  = NULL;
    int count = 0;
    TiXmlNode* root_node = NULL;
    TiXmlElement* element = NULL;

    // find msg element.
    root_node = handle->doc->RootElement();
    if ( ( NULL == root_node ) || ( 0 != strcmp( MSG_NODE, root_node->Value() ) ) )
    {
        printf("can't find msg node \n");
        return -1;
    }
    element = root_node->ToElement();
    tmp = ( char * )element->Attribute( TYPE_ATTR );
    if ( NULL != tmp )
    {
        strcpy( type, tmp );
        count++;
    }

    // find tag element.
    element = root_node->FirstChildElement();
    if ( ( NULL == element ) || ( 0 != strcmp( TAG_NODE, element->Value() ) ) )
    {
        printf("can't find tag node \n");
        return -1;
    }

    tmp =  (char *)element->Attribute( NS_ATTR );
    if ( NULL != tmp )
    {
        strcpy( ns, tmp );
        count++;
    }

    return ( ( 2 == count ) ? 0 : -1 );
}

int xml_msg_get_frist_item( XML_MSG_HANDLE handle, char *item, char *value )
{
    char *tmp  = NULL;
    int ret   = -1;
    TiXmlNode* root_node = NULL;
    TiXmlElement* element = NULL;

    // find msg element.
    root_node = handle->doc->RootElement();
    if ( ( NULL == root_node ) || ( 0 != strcmp( MSG_NODE, root_node->Value() ) ) )
    {
        printf("can't find msg node \n");
        return -1;
    }

    // find tag element.
    element = root_node->FirstChildElement();
    if ( ( NULL == element ) || ( 0 != strcmp( TAG_NODE, element->Value() ) ) )
    {
        printf("can't find tag node \n");
        return -1;
    }

    // find the item element.
    element = element->FirstChildElement();
    if ( NULL != element )
    {
        tmp = ( char * )element->Value();
        if ( NULL != tmp )
        {
            strcpy( item, tmp );
        }

        tmp = (char *)element->GetText(); // text
        if ( NULL != tmp )
        {
            strcpy( value, tmp );
            ret = 0;
        }
    }

    return ret;
}

int xml_msg_get_item( XML_MSG_HANDLE handle, const char *item, char *value )
{
    char *tmp  = NULL;
    int ret   = -1;
    TiXmlNode* root_node = NULL;
    TiXmlElement* element = NULL;

    // find msg element.
    root_node = handle->doc->RootElement();
    if ( ( NULL == root_node ) || ( 0 != strcmp( MSG_NODE, root_node->Value() ) ) )
    {
        printf("can't find msg node \n");
        return -1;
    }

    // find tag element.
    element = root_node->FirstChildElement();
    if ( ( NULL == element ) || ( 0 != strcmp( TAG_NODE, element->Value() ) ) )
    {
        printf("can't find tag node \n");
        return -1;
    }

    // find the item element.
    element = element->FirstChildElement();
    while ( ( NULL != element ) && ( 0 != strcmp( item, element->Value() ) ) )
    {
        element = element->NextSiblingElement();
    }
    if ( NULL != element )
    {
        tmp = (char *)element->GetText(); // text
        if ( NULL != tmp )
        {
            strcpy( value, tmp );
            ret = 0;
        }
    }

    return ret;
}

int xml_msg_set_type_ns( XML_MSG_HANDLE handle, char *type, char *ns )
{
    TiXmlNode* root_node = NULL;
    TiXmlElement* element = NULL;
    TiXmlElement* tag_element = NULL;

    // find msg element.
    root_node = handle->doc->RootElement();
    if ( ( NULL == root_node ) || ( 0 != strcmp( MSG_NODE, root_node->Value() ) ) )
    {
        printf("can't find msg node \n");
        return -1;
    }
    element = root_node->ToElement();
    element->SetAttribute( TYPE_ATTR, type );

    // find tag element.
    element = root_node->FirstChildElement();
    if ( ( NULL == element ) || ( 0 != strcmp( TAG_NODE, element->Value() ) ) )
    {
        printf("can't find tag node \n");
        return -1;
    }
    tag_element = element;
    element->SetAttribute( NS_ATTR, ns );

    return 0;
}

int xml_msg_set_item( XML_MSG_HANDLE handle, const char *item, char *value )
{
    TiXmlNode* root_node = NULL;
    TiXmlElement* element = NULL;
    TiXmlElement* tag_element = NULL;

    // find msg element.
    root_node = handle->doc->RootElement();
    if ( ( NULL == root_node ) || ( 0 != strcmp( MSG_NODE, root_node->Value() ) ) )
    {
        printf("can't find msg node \n");
        return -1;
    }

    // find tag element.
    element = root_node->FirstChildElement();
    if ( ( NULL == element ) || ( 0 != strcmp( TAG_NODE, element->Value() ) ) )
    {
        printf("can't find tag node \n");
        return -1;
    }
    tag_element = element;

    // find the item element.
    element = element->FirstChildElement();
    while ( ( NULL != element ) && ( 0 != strcmp( item, element->Value() ) ) )
    {
        element = element->NextSiblingElement();
    }

    if ( NULL == element )
    {
        // printf("[xml]: can't find item %s, create it \n", item );
        TiXmlElement child_elt( item );
        TiXmlNode* cur_node = tag_element->InsertEndChild( child_elt );
        element = cur_node->ToElement();
        TiXmlText new_node( value );
        element->InsertEndChild( new_node );
    }
    else
    {
        TiXmlText new_text( value );
        TiXmlNode *old_node = element->FirstChild();
        element->ReplaceChild( old_node, new_text );
    }

    return 0;
}

int xml_msg_get_content( XML_MSG_HANDLE handle, char *buf, int buf_len )
{
    int ret = 0;
    TiXmlPrinter printer;
    printer.SetIndent( "    " );
    handle->doc->Accept( &printer );

    if ( buf_len > (int)printer.Size() )
    {
        strcpy( buf, printer.CStr() );
        ret = printer.Size();
    }
    else
    {
        printf("xml get content buf size too small [%d] < [%d] \n", buf_len, printer.Size() );
    }

    return ret;
}

int xml_msg_get_content_without_head( XML_MSG_HANDLE handle, char *buf, int buf_len )
{
    int ret = 0;
    TiXmlPrinter printer;
    printer.SetIndent( "    " );
    handle->doc->Accept( &printer );

    if ( buf_len > (int)printer.Size() )
    {
        char *tmp = NULL;

        // <?xml version="1.0" encoding="UTF-8" ?>
        // find the root node.
        tmp = strstr( (char *)printer.CStr(), "?>" );
        if ( NULL != tmp )
        {
            // point to first node.
            tmp = strchr( tmp, '<' );
            if ( NULL != tmp )
            {
                strcpy( buf, tmp );
                ret = printer.Size() - ( tmp - printer.CStr() );
            }
        }

    }
    else
    {
        printf("xml get content buf size too small [%d] < [%d] \n", buf_len, printer.Size() );
    }

    return ret;
}

int xml_msg_unit_test( void )
{
    char buf[256] = { 0 };

    FILE *fp = fopen( "test.xml", "rb" );
    //fread( buf, 1, 256, fp );
    fclose( fp );

    XML_MSG_HANDLE handle = xml_msg_create( buf );

    xml_msg_set_item( handle, (char*)"result",    (char*)"setted-value" );
    xml_msg_set_item( handle, (char*)"userright", (char*)"setted-value" );
    xml_msg_set_item( handle, (char*)"userrighta",(char*)"setted-value" );

    int length = xml_msg_get_content_without_head( handle, buf, 256 );
    fp = fopen( "test1.xml", "wb" );
    fwrite( buf, 1, length, fp );
    fclose( fp );

    xml_msg_destroy( handle );

    return 0;
}
