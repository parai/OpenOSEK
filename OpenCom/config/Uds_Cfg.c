
#include "Com.h"

LOCAL const Uds_ServiceType UdsServiceTable[] =
{
	{
		SID_DIAGNOSTIC_SESSION_CONTROL,
		0xFFFFU,	// all Session available
		UdsUnSecurityLevel,
	}
};


EXPORT const Uds_ConfigType UdsConfig =
{
	UdsServiceTable,
	sizeof(UdsServiceTable)/sizeof(Uds_ServiceType)
};



