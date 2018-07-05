// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

class MainConfig
{
public:
	int GetIntConfigVar(const char* _varName, int _errVal);
	bool SetIntConfigVar(const char* _varName, int _newVal);
	double GetDoubleConfigVar(const char* _varName, double _errVal);
	bool SetDoubleConfigVar(const char* _varName, double _newVal);

private:
	void *GetConfigVar(const char* cVar);
};
