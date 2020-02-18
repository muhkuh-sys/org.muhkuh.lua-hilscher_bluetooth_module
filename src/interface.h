#ifndef __INTERFACE_H__
#define __INTERFACE_H__


typedef enum BT_COMMAND_ENUM
{
	BT_COMMAND_ReadDeviceInfo      = 0,
	BT_COMMAND_UpdateMAC           = 1
} BT_COMMAND_T;



typedef struct BT_COMMAND_READDEVICEINFO_STRUCT
{
	unsigned char aucData[66];
} BT_READDEVICEINFO_WRITEAREA_T;



typedef struct BT_COMMAND_UPDATEMAC_STRUCT
{
	unsigned char aucMAC[6];
} BT_COMMAND_UPDATEMAC_T;



typedef struct BT_PARAMETER_STRUCT
{
	BT_COMMAND_T tCommand;
	union
	{
		BT_READDEVICEINFO_WRITEAREA_T tReadDeviceInfo;
		BT_COMMAND_UPDATEMAC_T tUpdateMac;
	} uData;
} BT_PARAMETER_T;



typedef enum TEST_RESULT_ENUM
{
	TEST_RESULT_OK = 0,
	TEST_RESULT_ERROR = 1
} TEST_RESULT_T;


TEST_RESULT_T test(BT_PARAMETER_T *ptParameter);


#endif  /* __INTERFACE_H__ */
