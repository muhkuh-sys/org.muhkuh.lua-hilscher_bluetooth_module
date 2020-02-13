#ifndef __INTERFACE_H__
#define __INTERFACE_H__


typedef enum TEST_RESULT_ENUM
{
	TEST_RESULT_OK = 0,
	TEST_RESULT_ERROR = 1
} TEST_RESULT_T;


TEST_RESULT_T test(void);


#endif  /* __INTERFACE_H__ */
