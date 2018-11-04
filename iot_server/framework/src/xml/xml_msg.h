#ifndef __XML_MSG_H
#define __XML_MSG_H
// only support parent and child node.

#ifdef __cplusplus
extern "C" {
#endif

#define MSG_NODE    "msg"
#define TAG_NODE    "tag"
#define TYPE_ATTR   "type"
#define NS_ATTR     "xmlns"

#define XML_BASED_MSG "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"\
    "<msg type=\'\'>"\
    "<tag xmlns=\'\'>"\
    "</tag>"\
"</msg>"

//<?xml version="1.0" encoding="UTF-8" ?>
//<msg type='response'>
//<tag xmlns='hh:wireless:register'>
//    <result>result-value</result>
//    <userright>user-right-value</userright>
//</tag>
//</msg>

typedef struct XmlMsgObject* XML_MSG_HANDLE;

// Functionality:
//   Initialize xml library.
// Parameters:
//   @[in]fileName: the xml file name.
// Return:
//   handle on success, NULL failed.
//
XML_MSG_HANDLE xml_msg_create( char *buffer );

// Functionality:
//   exit from xml library.
// Parameters:
//   @[in]handle: return from xml_msg_create().
// Return:
//   None.
//
void  xml_msg_destroy( XML_MSG_HANDLE handle );

// Functionality:
//   get type and namespace.
// Parameters:
//   @[in]handle: return from xml_msg_create().
//   @[in]type:   always "request", "response".
//   @[in]ns:     name space of xml.
// Return:
//   0: success, -1 failed.
int xml_msg_get_type_ns( XML_MSG_HANDLE handle, char *type, char *ns );

// Functionality:
//   get item and value.
// Parameters:
//   @[in]handle: return from xml_msg_create().
//   @[in]item:   output param.
//   @[in]value:  text of <item>"value"</item>.
// Return:
//   0: success, -1 failed.
int xml_msg_get_frist_item( XML_MSG_HANDLE handle, char *item, char *value );

// Functionality:
//   get value.
// Parameters:
//   @[in]handle: return from xml_msg_create().
//   @[in]item:   input  node.
//   @[in]value:  text of <item>"value"</item>.
// Return:
//   0: success, -1 failed.
int xml_msg_get_item( XML_MSG_HANDLE handle, const char *item, char *value );

// Functionality:
//   set type and ns.
// Parameters:
//   @[in]handle: return from xml_msg_create().
//   @[in]type:   request/response.
//   @[in]ns:     xmlns: xml name space.
// Return:
//   0: success, -1 failed.
int xml_msg_set_type_ns( XML_MSG_HANDLE handle, char *type, char *ns );

// Functionality:
//   set string value.
// Parameters:
//   @[in]handle: return from xml_msg_create().
//   @[in]item:   input  node.
//   @[in]value:  the value of <item>"value"</item>.
// Return:
//   0: success, -1 failed.
int xml_msg_set_item( XML_MSG_HANDLE handle, const char *item, char *value );

// Functionality:
//   get content of xml file, include head.
// Parameters:
//   @[in]handle: return from xml_msg_create().
//   @[in]buf:     buffer for store data.
//   @[in]buf_len: the length of buf.
// Return:
//   0: failed, > 0: the length of xml file.
int xml_msg_get_content( XML_MSG_HANDLE handle, char *buf, int buf_len );


// Functionality:
//   get content of xml file, exclude head.
// Parameters:
//   @[in]handle: return from xml_msg_create().
//   @[in]buf:     buffer for store data.
//   @[in]buf_len: the length of buf.
// Return:
//   0: failed, > 0: the length of xml file.
int xml_msg_get_content_without_head( XML_MSG_HANDLE handle, char *buf, int buf_len );

int xml_msg_unit_test( void );

#ifdef __cplusplus
}
#endif

#endif // __XML_API_H
