// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "MainConfig.h"
#include "reaper_plugin_functions.h"


int MainConfig::GetIntConfigVar(const char *setting, int error)
{
	int *value = (int *) GetConfigVar(setting);
	return value ? *value : error;
}


bool MainConfig::SetIntConfigVar(const char *setting, int newValue)
{
	int *variable = (int *) GetConfigVar(setting);
	if (variable)
	{
		*variable = newValue;
		return true;
	}
	return false;
}


double MainConfig::GetDoubleConfigVar(const char *setting, double error)
{
	double *value = (double *) GetConfigVar(setting);
	return value ? *value : error;
}


bool MainConfig::SetDoubleConfigVar(const char *setting, double newValue)
{
	double *variable = (double*) GetConfigVar(setting);
	if (variable)
	{
		*variable = newValue;
		return true;
	}
	return false;
}


void *MainConfig::GetConfigVar(const char *setting)
{
	int sztmp;
	int offset = projectconfig_var_getoffs(setting, &sztmp);
	if (offset)
		return projectconfig_var_addr(EnumProjects(-1, NULL, 0), offset);
	return get_config_var(setting, &sztmp);
}
