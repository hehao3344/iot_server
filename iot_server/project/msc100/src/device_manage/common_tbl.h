#ifndef __COMMON_TBL_H
#define __COMMON_TBL_H

#ifdef __cplusplus
extern "C"
{
#endif

#define TCP_TIMEOUT     (20)
#define TYPE            (0x9C)

extern long g_lCryptTable[];

unsigned int common_tbl_to_hash (void* pstring);
int          common_tbl_keys_equal_func (void *pkey1, void *pkey2);

#ifdef __cplusplus
extern "C"
}
#endif

#endif // __COMMON_TBL_H
