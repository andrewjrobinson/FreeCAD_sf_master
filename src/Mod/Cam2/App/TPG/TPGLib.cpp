/***************************************************************************
 *   Copyright (c) 2012 Luke Parry      (l.parry@warwick.ac.uk)            *
 *                 2012 Andrew Robinson (andrewjrobinson@gmail.com)        *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#include "PreCompiled.h"
#ifndef _PreComp_
#endif

#include <QString>

#include "../Features/TPGFeature.h"
#include "TPGLib.h"

using namespace Cam;

TPG* LibTPGDescriptor::make()
{
    return TPGFactory().getPlugin(id);
}

LibTPG::LibTPG(TPGDescriptor *descriptor)
       :TPG(descriptor->id, descriptor->name, descriptor->description)
{
    
}

LibTPG::~LibTPG()
{
}

