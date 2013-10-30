
#include "Com.h"

LOCAL const Uds_ServiceType UdsServiceTable[] =
{
	{
		SID_DIAGNOSTIC_SESSION_CONTROL,
		UdsAllSession,	// all Session available
		UdsUnSecurityLevel
	},
	{
		SID_SECURITY_ACCESS,
		UdsAllSession,
		UdsUnSecurityLevel
	},
	{
		SID_COMMUNICATION_CONTROL,
		UdsProgramSession,
		UdsSecurityLevel1
	},
	{
		SID_TESTER_PRESENT,
		UdsAllSession,
		UdsUnSecurityLevel
	},
	{
		SID_READ_DATA_BY_IDENTIFIER,
		UdsAllSession,
		UdsUnSecurityLevel
	},
	{
		SID_WRITE_DATA_BY_IDENTIFIER,
		UdsAllSession,
		UdsUnSecurityLevel
	},
	{
		SID_ROUTINE_CONTROL,
		UdsAllSession,
		UdsUnSecurityLevel
	}
};
// An Example Config
IMPORT uint16 UdsRDID_FF01(uint8* Data,uint16 length);
IMPORT uint16 UdsRDID_FF09(uint8* Data,uint16 length);
LOCAL const Uds_DIDType UdsRDIDTable[] =
{
	{
		0xFF01,
		UdsProgramSession,
		UdsSecurityLevel1,
		UdsRDID_FF01
	},
	{
		0xFF09,
		UdsProgramSession,
		UdsSecurityLevel1,
		UdsRDID_FF09
	}
};

// An Example Config
IMPORT uint16 UdsWDID_FE02(uint8* Data,uint16 length);
IMPORT uint16 UdsWDID_FE09(uint8* Data,uint16 length);
LOCAL const Uds_DIDType UdsWDIDTable[] =
{
	{
		0xFE02,
		UdsProgramSession,
		UdsSecurityLevel1,
		UdsWDID_FE02
	},
	{
		0xFE09,
		UdsProgramSession,
		UdsSecurityLevel1,
		UdsWDID_FE09
	}
};
IMPORT uint16 UdsStartRC_AB11(uint8* param,uint16 length,uint8* status);
IMPORT uint16 UdsStopRC_AB11(uint8* param,uint16 length,uint8* status);
IMPORT uint16 UdsRequestResultRC_AB11(uint8* param,uint16 length,uint8* result);
LOCAL const Uds_RCType UdsRCTable[] =
{
	{
		0xAB11,
		UdsProgramSession,
		UdsSecurityLevel1,
		UdsStartRC_AB11,
		UdsStopRC_AB11,
		UdsRequestResultRC_AB11
	}
};



EXPORT const Uds_ConfigType UdsConfig =
{
	UdsServiceTable,
	sizeof(UdsServiceTable)/sizeof(Uds_ServiceType),
	UdsRDIDTable,
	sizeof(UdsRDIDTable)/sizeof(Uds_DIDType),
	UdsWDIDTable,
	sizeof(UdsWDIDTable)/sizeof(Uds_DIDType),
	UdsRCTable,
	sizeof(UdsRCTable)/sizeof(Uds_RCType),
};

EXPORT uint16 UdsRDID_FF01(uint8* Data,uint16 length)
{
	uint8 i;
	if(length < 128)
	{
		return 0;
	}
	for(i=0;i<128;i++)
	{
		Data[i] = (i%'A') + 'A';
	}
	return 128;
}

EXPORT uint16 UdsRDID_FF09(uint8* Data,uint16 length)
{
	uint8 i;
	if(length < 64)
	{
		return 0;
	}
	for(i=0;i<64;i++)
	{
		Data[i] = i;
	}
	return 64;
}

EXPORT uint16 UdsWDID_FE02(uint8* Data,uint16 length)
{
	int i;
	if(128u != length)
	{
		return E_NOT_OK;
	}
	printf("WDID_FE02=[");
	for(i=0;i<length;i++)
	{
		printf("0x%X,",Data[i]);
	}
	printf("]\n");
	return E_OK;
}

EXPORT uint16 UdsWDID_FE09(uint8* Data,uint16 length)
{
	int i;
	if(64u != length)
	{
		return E_NOT_OK;
	}
	printf("WDID_FE09=[");
	for(i=0;i<length;i++)
	{
		printf("0x%X,",Data[i]);
	}
	printf("]\n");
	return E_OK;
}

EXPORT uint16 UdsStartRC_AB11(uint8* param,uint16 length,uint8* status)
{
	printf("StartRC_AB11().\n");
	memcpy(status,"StartRC_AB11().\n",sizeof("StartRC_AB11().\n"));
	return RCReturn(sizeof("StartRC_AB11().\n"));
}
EXPORT uint16 UdsStopRC_AB11(uint8* param,uint16 length,uint8* status)
{
	printf("StopRC_AB11().\n");
	memcpy(status,"StopRC_AB11().\n",sizeof("StopRC_AB11().\n"));
	return RCReturn(sizeof("StopRC_AB11().\n"));
}
EXPORT uint16 UdsRequestResultRC_AB11(uint8* param,uint16 length,uint8* result)
{
	printf("RequestResultRC_AB11().\n");
	memcpy(result,"RequestResultRC_AB11().\n",sizeof("RequestResultRC_AB11().\n"));
	return RCReturn(sizeof("RequestResultRC_AB11().\n"));
}



