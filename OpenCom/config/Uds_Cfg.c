
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
	}
};
// An Example Config
IMPORT uint16 UdsRDID_FF01(uint8* Data,uint16 length);
IMPORT uint16 UdsRDID_FF09(uint8* Data,uint16 length);
LOCAL const Uds_RDIDType UdsRDIDTable[] =
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

EXPORT const Uds_ConfigType UdsConfig =
{
	UdsServiceTable,
	sizeof(UdsServiceTable)/sizeof(Uds_ServiceType),
	UdsRDIDTable,
	sizeof(UdsRDIDTable)/sizeof(Uds_RDIDType),
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



