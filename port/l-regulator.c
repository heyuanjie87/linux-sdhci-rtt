#include "osdep/port.h"

int __sdhci_regulator_get_current_limit(struct regulator *regulator)
{
	return 0;
}

int __sdhci_regulator_is_supported_voltage(struct regulator *regulator,
				   int min_uV, int max_uV)
{
	return 0;
}
