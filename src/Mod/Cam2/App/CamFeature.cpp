/***************************************************************************
 *   Copyright (c) 2012 Luke Parry    (l.parry@warwick.ac.uk)              *
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
# include <TopoDS_Shape.hxx>
# include <TopoDS_Face.hxx>
# include <TopoDS.hxx>
# include <TopExp_Explorer.hxx>
# include <BRepAdaptor_Surface.hxx>
# include <BRepAdaptor_Curve.hxx>
# include <BRep_Tool.hxx>
# include <Geom_Plane.hxx>
# include <GeomAPI_ProjectPointOnSurf.hxx>
# include <BRepOffsetAPI_NormalProjection.hxx>
# include <BRepBuilderAPI_MakeFace.hxx>
#endif

#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>

#include <Base/Writer.h>
#include <Base/Reader.h>
#include <Base/Tools.h>
#include <Base/Console.h>
#include <Base/Exception.h>

#include <vector>

#include <boost/bind.hpp>

#include "CamPartsList.h"
#include "TPGList.h"
#include "StockGeometry.h"
#include "GCodeFeature.h"

#include "CamFeature.h"

// #include "CamFeaturePy.h"

using namespace Cam;
using namespace Base;

PROPERTY_SOURCE(Cam::CamFeature, App::DocumentObject)

const char *camFeatGroup = "Cam Feature";

CamFeature::CamFeature()
{
    ADD_PROPERTY_TYPE(Result,(0), camFeatGroup, (App::Prop_None),"Result");
}

CamFeature::~CamFeature()
{
    delObjConnection.disconnect();
}

void CamFeature::onSettingDocument()
{
    //Create a signal to observe slot if this item is deleted
    delObjConnection = getDocument()->signalDeletedObject.connect(boost::bind(&Cam::CamFeature::onDelete, this, _1));
}

/**
 * Initialise creates the basic Cam Feature tree with all the required sub features
 */
void CamFeature::initialise()
{    
    App::Document *doc = getDocument();
    //Generate unique names for the objects
    std::string CamPartsListName  = doc->getUniqueObjectName("CamPartsList");
    std::string StockGeometryName = doc->getUniqueObjectName("StockGeometry");
    std::string TPGListName       = doc->getUniqueObjectName("ToolPaths");
    std::string GCodeFeatName     = doc->getUniqueObjectName("GCodeFeature");

    // Create the SubNodes that will be referenced by pointers for quick access;

    App::DocumentObject *camPartsObj  = doc->addObject("Cam::CamPartsList" , CamPartsListName.c_str());
    App::DocumentObject *stockGeomObj = doc->addObject("Cam::StockGeometry", StockGeometryName.c_str());
    App::DocumentObject *TPGListObj   = doc->addObject("Cam::TPGList"      , TPGListName.c_str());
    App::DocumentObject *gcodeFeatObj = doc->addObject("Cam::GCodeFeature" , GCodeFeatName.c_str());

    if(!camPartsObj || !camPartsObj->isDerivedFrom(CamPartsList::getClassTypeId()))
        throw new Base::Exception("A CamPartsList Feature couldn't successfuly be created");

    if(!stockGeomObj || !stockGeomObj->isDerivedFrom(StockGeometry::getClassTypeId()))
        throw new Base::Exception("A Stock Geometry Feature couldn't successfuly be created");

    if(!TPGListObj || !TPGListObj->isDerivedFrom(TPGList::getClassTypeId()))
        throw new Base::Exception("A TPGList Feature couldn't successfuly be created");

    TPGList *tpgList = dynamic_cast<TPGList *>(TPGListObj);
    tpgList->CamPartsListObject.setValue(camPartsObj); // Create Object Link
    tpgList->StockGeometryObject.setValue(stockGeomObj); // Create Object Link

    if(!gcodeFeatObj || !gcodeFeatObj->isDerivedFrom(GCodeFeature::getClassTypeId()))
        throw new Base::Exception("A GCode Feature couldn't successfuly be created");

    GCodeFeature *gCodeFeat = dynamic_cast<GCodeFeature *>(gcodeFeatObj);
    gCodeFeat->TPGListObj.setValue(TPGListObj); // Create Object Link
    this->Result.setValue(gcodeFeatObj);

    doc->recompute();
}

void CamFeature::onDelete(const App::DocumentObject &docObj)
{
    // If deleted object matches this cam feature, proceed to delete children
    if(std::strcmp(docObj.getNameInDocument(), getNameInDocument()) == 0)
    {
        App::Document *pcDoc = getDocument();
        // Automatically remove children linked to this CamFeature
        pcDoc->remObject(getPartsContainer()->getNameInDocument());
        pcDoc->remObject(getStockGeometry()->getNameInDocument());
        pcDoc->remObject(getTPGContainer()->getNameInDocument());
    }
}

short int CamFeature::mustExecute() const
{
    return App::DocumentObject::mustExecute();
}


App::DocumentObjectExecReturn *CamFeature::execute(void)
{
    return App::DocumentObject::StdReturn;
}

// PyObject *CamFeature::getPyObject(void)
// {
//     if (PythonObject.is(Py::_None())) {
//         // ref counter is set to 1
//         PythonObject = Py::Object(new CamFeaturePy(this),true);
//     }
//     return Py::new_reference_to(PythonObject);
// }

TPGList * CamFeature::getTPGContainer() const
{
    return getGCodeFeature()->getTPGList();
}

StockGeometry * CamFeature::getStockGeometry() const
{
    return getTPGContainer()->getStockGeometry();
}

CamPartsList * CamFeature::getPartsContainer() const
{
    return getTPGContainer()->getCamPartsList();
}

GCodeFeature * CamFeature::getGCodeResult() const
{
    App::DocumentObject *docObj = Result.getValue();
    if(!docObj || !docObj->isDerivedFrom(GCodeFeature::getClassTypeId()))
        throw new Base::Exception("Cannot restore this Cam Feature::Invalid Link to Result Property");

     return dynamic_cast<GCodeFeature *>(docObj);
}

unsigned int CamFeature::getMemSize(void) const
{
    return 0;
}

void CamFeature::Save(Writer &writer) const
{
    //save the father classes
    App::DocumentObject::Save(writer);
}

void CamFeature::Restore(XMLReader &reader)
{
    //read the father classes
    App::DocumentObject::Restore(reader);
}

void CamFeature::onChanged(const App::Property* prop)
{
    App::DocumentObject::onChanged(prop);
    if(prop = &Result) {
        return;
    }    
}

// void CamFeature::onDocumentRestored()
// {
//     //We need to update the references on restoration. This is achieved by going through the dependency tree
//     App::DocumentObject::onDocumentRestored();
//     try {
//     }
//     catch (...) {
//     }
// }
// 
// void CamFeature::onFinishDuplicating()
// {
// }
