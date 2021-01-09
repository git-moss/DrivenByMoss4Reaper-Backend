// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2021
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <locale>
#include <codecvt>


template<class I, class E, class S>
struct codecvt : std::codecvt<I, E, S>
{
    ~codecvt()
    { }
};
