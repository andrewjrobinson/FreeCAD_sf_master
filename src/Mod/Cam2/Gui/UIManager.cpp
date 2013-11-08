/***************************************************************************
 *   Copyright (c) 2012 Andrew Robinson <andrewjrobinson@gmail.com>        *
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

#include <PreCompiled.h>
#ifndef _PreComp_
#include <Python.h>
#endif

#include <vector>
#include <set>

#include <QMessageBox>

#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>
#include <App/DocumentObjectGroup.h>

#include <Base/Console.h>

#include <Gui/Application.h>
#include <Gui/Command.h>
#include <Gui/Document.h>
#include <Gui/DockWindowManager.h>
#include <Gui/MainWindow.h>
#include <Gui/Selection.h>
#include <Gui/SelectionFilter.h>

#include "../App/CamManager.h"
#include "../App/Features/TPGFeature.h"
#include "../App/Features/ToolPathFeature.h"
#include "../App/TPG/PyTPGFactory.h"
#include "../App/TPG/TPG.h"

#include "UIManager.h"
#include "TPGListModel.h"
#include "ViewProviderCamFeature.h"

using namespace CamGui;

UIManagerInst* UIManagerInst::_pcSingleton = NULL;

UIManagerInst& UIManagerInst::instance()
{
    if (_pcSingleton == NULL)
        _pcSingleton = new UIManagerInst();

    return *_pcSingleton;
}
void UIManagerInst::destruct (void)
{
    if (_pcSingleton != NULL)
        delete _pcSingleton;
    _pcSingleton = NULL;
}

UIManagerInst::UIManagerInst() {

	// receive tpg running state changes from Cam layer.
	QObject::connect(&Cam::CamManager(), SIGNAL(updatedTPGState(QString, Cam::TPG::State, int)),
			this, SLOT(updatedTPGState(QString, Cam::TPG::State, int)));

    Gui::Selection().Attach(this);
}

UIManagerInst::~UIManagerInst() {

    Gui::Selection().Detach(this);
}


// ----- GUI Command Implementations -----

/**
 * Used by the CamFeature GUI Command to do the work required to add a CamFeature
 */
bool UIManagerInst::CamFeature() {


	// get the document
	App::Document* activeDoc = App::GetApplication().getActiveDocument();
	if (!activeDoc) {
		Base::Console().Error("No active document! Please create or open a FreeCad document\n");
		return false;
	}

	// make the CamFeature
	return makeCamFeature(activeDoc) != NULL;

//    std::string FeatName = activeDoc->getUniqueObjectName("Cam Feature");
////    App::Document *doc   = Gui::Document::getActiveGuiDocument()->getDocument();
//
//    // NOTE Need to use simple test case file
//    App::DocumentObject *camFeat =  activeDoc->addObject("Cam::CamFeature", FeatName.c_str());
//
//    // Initialise a few TPG Features and put this in tree for testing
//
////    App::DocumentObject *docObj = doc->getObject(FeatName.c_str());
//
//    if(camFeat && camFeat->isDerivedFrom(Cam::CamFeature::getClassTypeId())) {
//		Cam::CamFeature *camFeat = dynamic_cast<Cam::CamFeature *>(camFeat);
//
//		// We Must Initialise the Cam Feature before usage
//		camFeat->initialise();
//    }
//    else {
//    	Base::Console().Error("Unable to create Cam Feature\n");
//    	return false;
//	}
////    App::DocumentObject *docObj = activeDoc->getObject(FeatName.c_str());
////
////    if(docObj && docObj->isDerivedFrom(Cam::CamFeature::getClassTypeId())) {
////        Cam::CamFeature *camFeat = dynamic_cast<Cam::CamFeature *>(docObj);
////
////        // We Must Initialise the Cam Feature before usage
////        camFeat->initialise();
////    }
////    else {
////    	Base::Console().Error("Unable to get Cam Feature\n");
////    	return false;
////	}
//    return true;
}

/**
 * Used by the CamTPGFeature GUI Command to do the work required to add a TPGFeature
 */
bool UIManagerInst::TPGFeature() {


	// get the document
//	App::Document* activeDoc = App::GetApplication().getActiveDocument();
//	if (!activeDoc) {
//		Base::Console().Error("No active document! Please create or open a FreeCad document\n");
//		return false;
//	}

	Cam::TPGDescriptorCollection *plugins = Cam::TPGFactory().getDescriptors();
	if (plugins && plugins->size() > 0)
		addTPG(plugins->at(0));

	plugins->release();

	return true;
}

/**
 * Adds a new ToolFeature to the active document.  If a group is selected then it will be
 * the parent of the ToolFeature otherwise it will be created at the top level.
 * Used by the CamToolFeature GUI Command to do the work required to add a ToolFeature
 */
bool UIManagerInst::ToolFeature() {

    App::DocumentObjectGroup *docObjGroup = NULL;

    // check if the selection is a group (so object is created in the group)
    std::vector<Gui::SelectionSingleton::SelObj> objs = Gui::Selection().getSelection(App::GetApplication().getActiveDocument()->getName());
    if (objs.size() == 1 && objs[0].pObject->isDerivedFrom(App::DocumentObjectGroup::getClassTypeId()))
        docObjGroup = dynamic_cast<App::DocumentObjectGroup*>(objs[0].pObject);

    // make the ToolFeature
    return Cam::CamManager().ToolFeature(docObjGroup);
}

/**
 * Used by the CamMachineFeature GUI Command to do the work required to add a MachineFeature
 */
bool UIManagerInst::MachineFeature() {

    App::DocumentObjectGroup *docObjGroup = NULL;

    // check if the selection is a group (so object is created in the group)
    std::vector<Gui::SelectionSingleton::SelObj> objs = Gui::Selection().getSelection(App::GetApplication().getActiveDocument()->getName());
    if (objs.size() == 1 && objs[0].pObject->isDerivedFrom(App::DocumentObjectGroup::getClassTypeId()))
        docObjGroup = dynamic_cast<App::DocumentObjectGroup*>(objs[0].pObject);

    // make the ToolFeature
    return Cam::CamManager().MachineFeature(docObjGroup);
}


/**
 * Executes the selected TPG(s) to (re)produce its Tool Path.
 * TODO: make non-TPG selection a warning only
 * TODO: allow CamFeature selection to run all TPG's within
 * TODO: allow (and ask for) parallel TPG execution.
 */
bool UIManagerInst::RunTPG() {

    // make unique list of selected objects
    std::vector<Gui::SelectionSingleton::SelObj> objs = Gui::Selection().getSelection(App::GetApplication().getActiveDocument()->getName());
    std::set<App::DocumentObject*> selDocObjs;
    for (std::vector<Gui::SelectionSingleton::SelObj>::iterator it = objs.begin(); it != objs.end(); ++it)
    	selDocObjs.insert(it->pObject);

    // check all objects are TPG's
    for (std::set<App::DocumentObject*>::const_iterator it = selDocObjs.begin(); it != selDocObjs.end(); ++it) {
    	if (!(*it)->isDerivedFrom(Cam::TPGFeature::getClassTypeId())) {
    		QMessageBox msgBox;
			msgBox.setText(QObject::tr("Your selection contains more than just TPG's"));
			msgBox.setInformativeText(QObject::tr("Please only select TPG's"));
			msgBox.setStandardButtons(QMessageBox::Ok);
			int ret = msgBox.exec();
			return false;
    	}
    }

    // run TPG's
    for (std::set<App::DocumentObject*>::const_iterator it = selDocObjs.begin(); it != selDocObjs.end(); ++it) {
		Cam::CamManager().runTPGByName((*it)->getNameInDocument());
    }
	return true;
}


/**
 * Executes the selected TPG(s) Tool Path to (re)produce its Machine Program.
 */
bool UIManagerInst::PostProcess() {
    // make unique list of selected objects
    std::vector<Gui::SelectionSingleton::SelObj> objs = Gui::Selection().getSelection(App::GetApplication().getActiveDocument()->getName());
    std::set<App::DocumentObject*> selDocObjs;
    for (std::vector<Gui::SelectionSingleton::SelObj>::iterator it = objs.begin(); it != objs.end(); ++it)
    	selDocObjs.insert(it->pObject);

    // check all objects are TPG's
    for (std::set<App::DocumentObject*>::const_iterator it = selDocObjs.begin(); it != selDocObjs.end(); ++it) {
    	if (!(*it)->isDerivedFrom(Cam::TPGFeature::getClassTypeId())) {
    		QMessageBox msgBox;
			msgBox.setText(QObject::tr("Your selection contains more than just TPG's"));
			msgBox.setInformativeText(QObject::tr("Please only select TPG's"));
			msgBox.setStandardButtons(QMessageBox::Ok);
			int ret = msgBox.exec();
			return false;
    	}
    }

    // run TPG's
    for (std::set<App::DocumentObject*>::const_iterator it = selDocObjs.begin(); it != selDocObjs.end(); ++it) {
		Cam::CamManager().runPostProcessByName((*it)->getNameInDocument(), (*it)->getDocument());
    }
	return true;
}



bool UIManagerInst::WatchHighlight() {

//	Base::Console().Message("WatchHighlight: start\n");
//	Gui::Document *guiDoc = Gui::Command::getGuiApplication()->activeDocument();
//	Base::Console().Message("WatchHighlight: mid\n");
//	highlightObjectConnection = guiDoc->signalHighlightObject.connect(boost::bind(&CamGui::UIManagerInst::onHighlightObject, this, _1, _2, _3));
//	Base::Console().Message("WatchHighlight: end\n");
	return true;
}

//test
//delObjConnection = getDocument()->signalDeletedObject.connect(boost::bind(&Cam::CamFeature::onDelete, this, _1));


//void UIManagerInst::onHighlightObject(const Gui::ViewProviderDocumentObject& docObjVP,
//		const Gui::HighlightMode& highlightMode, bool flag) {
//
//	if (flag)
//		Base::Console().Message("Highlight Flag on");
//	else
//		Base::Console().Message("Highlight Flag off");
//}

/**
 * Receive selection events so we can update the settings shown on the Cam Settings Dock Window
 */
void UIManagerInst::OnChange(Gui::SelectionSingleton::SubjectType &rCaller,
                             Gui::SelectionSingleton::MessageType Reason) {

    if (Reason.Type == Gui::SelectionChanges::AddSelection) {
    	Q_EMIT updatedTPGSelection(NULL);
    }
    else if (Reason.Type == Gui::SelectionChanges::SetSelection) {
    	updateCamProjectSelection(Reason.pDocName);
    }
    else if (Reason.Type == Gui::SelectionChanges::ClrSelection) {
    	Q_EMIT updatedTPGSelection(NULL);
    }
    else if (Reason.Type == Gui::SelectionChanges::RmvSelection) {
    	updateCamProjectSelection(Reason.pDocName);
    }
}

/**
 * Sets the currently selected tool path line(s)
 */
void UIManagerInst::setToolPathLineSelection(std::set<int> &tpSelection) {
    Q_EMIT updatedToolPathLineSelection(tpSelection);
}

/**
 * Performs the work to analyse the current selection and send setting update signal
 */
void UIManagerInst::updateCamProjectSelection(const char* pDocName) {

    // make unique list of selected objects
    std::vector<Gui::SelectionSingleton::SelObj> objs = Gui::Selection().getSelection(pDocName);
    std::set<App::DocumentObject*> selDocObjs;
    for (std::vector<Gui::SelectionSingleton::SelObj>::iterator it = objs.begin(); it != objs.end(); ++it)
    	selDocObjs.insert(it->pObject);

    // display settings only if single TPG selection
    if (selDocObjs.size() == 1)
    {
    	App::DocumentObject *docObj = *selDocObjs.begin();
    	if (docObj->isDerivedFrom(Cam::TPGFeature::getClassTypeId()))
    	{
			Cam::TPGFeature *tpgFeature = dynamic_cast<Cam::TPGFeature *>(docObj);
			if (tpgFeature) {
				Q_EMIT updatedTPGSelection(tpgFeature);
			}
            else
            	Q_EMIT updatedTPGSelection(NULL);
            Q_EMIT updatedToolPathSelection(NULL);
            Q_EMIT updatedMachineProgramSelection(NULL);
    	}
        else if (docObj->isDerivedFrom(Cam::ToolPathFeature::getClassTypeId())) {
            Cam::ToolPathFeature *tpFeature = dynamic_cast<Cam::ToolPathFeature *>(docObj);

            if (tpFeature) {
                // find matching machine program (and signal its selection before the toolpath)
                std::vector<App::DocumentObject*> refMe = tpFeature->getInList();
                Cam::TPGFeature *parentTPG = NULL;
                for (std::vector<App::DocumentObject*>::iterator it = refMe.begin(); it != refMe.end(); ++it) {
                    if ((*it)->getTypeId() == Cam::TPGFeature::getClassTypeId()) {
                        parentTPG = dynamic_cast<Cam::TPGFeature *>(*it);
                        break;
                    }
                }
                if (parentTPG) {
                    Q_EMIT updatedMachineProgramSelection(dynamic_cast<Cam::MachineProgramFeature *>(parentTPG->MachineProgram.getValue(Cam::MachineProgramFeature::getClassTypeId())));
                } else
                    Q_EMIT updatedMachineProgramSelection(NULL);

                // signal selection change
                Q_EMIT updatedToolPathSelection(tpFeature);
            }
            else {
                Q_EMIT updatedToolPathSelection(NULL);
                Q_EMIT updatedMachineProgramSelection(NULL);
            }
            Q_EMIT updatedTPGSelection(NULL);
        }
        else if (docObj->isDerivedFrom(Cam::MachineProgramFeature::getClassTypeId())) {
            Cam::MachineProgramFeature *mpFeature = dynamic_cast<Cam::MachineProgramFeature *>(docObj);
            Q_EMIT updatedToolPathSelection(NULL);
            if (mpFeature) {
                // find matching toolpath (and signal its selection before the machine program)
                std::vector<App::DocumentObject*> refMe = mpFeature->getInList();
                Cam::TPGFeature *parentTPG = NULL;
                for (std::vector<App::DocumentObject*>::iterator it = refMe.begin(); it != refMe.end(); ++it) {
                    if ((*it)->getTypeId() == Cam::TPGFeature::getClassTypeId()) {
                        parentTPG = dynamic_cast<Cam::TPGFeature *>(*it);
                        break;
                    }
                }
                if (parentTPG) {
                    Q_EMIT updatedToolPathSelection(dynamic_cast<Cam::ToolPathFeature *>(parentTPG->ToolPath.getValue(Cam::ToolPathFeature::getClassTypeId())));
                } else
                    Q_EMIT updatedToolPathSelection(NULL);

                // signal selection change
                Q_EMIT updatedMachineProgramSelection(mpFeature);
            }
            else {
                Q_EMIT updatedToolPathSelection(NULL);
                Q_EMIT updatedMachineProgramSelection(NULL);
            }
            Q_EMIT updatedTPGSelection(NULL);
        }
        else {
        	Q_EMIT updatedTPGSelection(NULL);
            Q_EMIT updatedToolPathSelection(NULL);
            Q_EMIT updatedMachineProgramSelection(NULL);
        }
    }
    else {
    	Q_EMIT updatedTPGSelection(NULL);
        Q_EMIT updatedToolPathSelection(NULL);
        Q_EMIT updatedMachineProgramSelection(NULL);
    }
}


// ----- Selection control -----

/**
 * Change the TPG Library selection.  This controls which TPG will be created
 * when the TPGFeature Gui Command is activated.
 */
void UIManagerInst::setTPGLibrarySelection(Cam::TPGDescriptor *tpgDescriptor) {
	if (tpgDescriptor != NULL)
		this->tpgLibrarySelectedID = tpgDescriptor->id;
}


/**
 * A Slot to receive requests to add TPG's to the document tree.
 * TODO: move the App specific code to the CamManager class so it can be used in Console only mode.
 *
 * @deprecated: This method will be removed in the future.  Use setTPGLibrarySelection() and TPGFeature() instead
 */
void UIManagerInst::addTPG(Cam::TPGDescriptor *tpgDescriptor) 
{
    if (tpgDescriptor == NULL) {
        Base::Console().Error("This is an invalid plugin description");
        return;
    }

    /////// PSEUDO CODE ///////
    // Check if CamFeature is selected?
    	// add TPG to this feature (after other TPG's)
    // if a parent of selection is a CamFeature?
    	// on selection of?
    		// TPG:
    			// add TPG before current selection
    		// Toolpath:
    			// add TPG before parent TPG
    		// Machine Program:
    			// add TPG at end of TPG's
    // else if count(CamFeature) == 1
    	// add TPG to this feature (after other TPG's)
    // else
    	// Ask if we should create a new CamFeature?
    		// Add CamFeature
    		// Add TPG to new CamFeature
    	// else do nothing

    /////// END PSEUDO ///////
    // get the Active document
	App::Document* activeDoc = App::GetApplication().getActiveDocument();
	if (!activeDoc) {
		Base::Console().Error("No active document! Please create or open a FreeCad document\n");
		return;
	}

	// check for CamFeature in selection
    Gui::SelectionFilter CamFeatureFilter("SELECT Cam::CamFeature COUNT 1");
    if (CamFeatureFilter.match()) {
    	Cam::CamFeature *CamFeature = static_cast<Cam::CamFeature*>(CamFeatureFilter.Result[0][0].getObject());

		// create the feature (for Document Tree)
	    std::string tpgFeatName = activeDoc->getUniqueObjectName(tpgDescriptor->name.toAscii().constData());
	    App::DocumentObject *tpgFeat =  activeDoc->addObject("Cam::TPGFeature", tpgFeatName.c_str());
	    if(tpgFeat && tpgFeat->isDerivedFrom(Cam::TPGFeature::getClassTypeId())) {
			Cam::TPGFeature *tpgFeature = dynamic_cast<Cam::TPGFeature *>(tpgFeat);

			// We Must Initialise the TPG Feature before usage
			tpgFeature->initialise();

			// Add descriptor details
			tpgFeature->PluginId.setValue(tpgDescriptor->id.toStdString().c_str());

			CamFeature->addTPG(tpgFeature);

			activeDoc->recompute();
	    }
	    else {

	    	if (tpgFeat)
		    	Base::Console().Error("Object not TPG Feature\n");
	    	else
	    		Base::Console().Error("Unable to create TPG Feature\n");
	    	Gui::Command::abortCommand();
	    	return;
		}
    }
    //TODO: check the selection is a child of a CamFeature
    else {
    	// get all CamFeatures in document
    	std::vector<App::DocumentObject*> camFeatures = activeDoc->getObjectsOfType(Cam::CamFeature::getClassTypeId());
    	if (camFeatures.size() == 1) { // only one found (just add to it)
    		Cam::CamFeature *CamFeature = static_cast<Cam::CamFeature*>(camFeatures[0]);
    		if (CamFeature) {
    			makeTPGFeature(activeDoc, CamFeature, tpgDescriptor);
    		}
    	}
    	else if (camFeatures.size() == 0) { // none found (ask to add one first)
            QMessageBox msgBox;
            msgBox.setText(QObject::tr("You do not have any Cam Features in your document."));
            msgBox.setInformativeText(QObject::tr("Would you please me to create one?"));
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            int ret = msgBox.exec();
            if (ret == QMessageBox::Yes) {
            	Cam::CamFeature *CamFeature = makeCamFeature(activeDoc);
            	activeDoc->recompute();
//            	activeDoc->recomputeFeature(CamFeature);
            	if (CamFeature)
            		makeTPGFeature(activeDoc, CamFeature, tpgDescriptor);
            }
    	}
    	else { // more than one CamFeature
    		Base::Console().Error("You have more than one Cam Feature.  Please select the one you wish to add the TPG to.\n");
    	}
    }


    // Test settings editor
//    Cam::TPG* tpg = tpgDescriptor->make();
//    Q_EMIT updatedTPGSelection(tpg);
//
//    Base::Console().Log("This is where I would add a '%s' TPG to the document", tpgDescriptor->name.toStdString().c_str());

//    // TPG was successfully created
//    // Find currently active CamFeature and add create a new TPGFeature and assign the TPG
//    Cam::CamFeature *camFeat = NULL;
//
//    Gui::Document * doc = Gui::Application::Instance->activeDocument();
//    if (doc->getInEdit() && doc->getInEdit()->isDerivedFrom(ViewProviderCamFeature::getClassTypeId())) {
//        ViewProviderCamFeature *vp = dynamic_cast<ViewProviderCamFeature*>(doc->getInEdit());
//        if(!vp) {
//            Base::Console().Log("An invalid view provider is currently being used");
//            return;
//        }
//        camFeat = vp->getObject();
//    }
//
//    // Create a new TPG Feature
//    if (camFeat != NULL) {
//        std::string tpgFeatName = doc->getDocument()->getUniqueObjectName("TPGFeature");
//        App::DocumentObject *tpgDocObj =  doc->getDocument()->addObject("Cam::TPGFeature", tpgFeatName.c_str());
//
//        if(!tpgDocObj || !tpgDocObj->isDerivedFrom(Cam::TPGFeature::getClassTypeId()))
//            return;
//
//        Cam::TPGFeature *tpgFeat = dynamic_cast<Cam::TPGFeature *>(tpgDocObj);
//
//        // Set a friendly label
//        if (tpgFeat != NULL) {
//            tpgFeat->Label.setValue(tpgDescriptor->name.toAscii());
//            Cam::TPG *temp = tpgDescriptor->make();
//            QMessageBox(QMessageBox::Warning, QString::fromAscii("Info"), temp->getName());
//            // Attempt to create and load the TPG Plugin
//            bool loadPlugin = tpgFeat->loadTPG(tpgDescriptor);
//
//            if(loadPlugin) {
//                // Add the Plugin to the TPG Feature's container
//                camFeat->getTPGContainer()->addTPG(tpgFeat);
//            } else {
//                QMessageBox(QMessageBox::Warning, QString::fromAscii("Info"), QString::fromAscii("Plugin couldn't be loaded"));
//                // remove TPGFeature
//                doc->getDocument()->remObject(tpgFeatName.c_str());
//            }
//        }
//        else
//            Base::Console().Log("Unable to find TPG Feature");
//    }
//    else
//        Base::Console().Log("Unable to find Cam Feature");

}

/**
 * Creates a new CamFeature and adds it to the document
 */
Cam::CamFeature *UIManagerInst::makeCamFeature(App::Document* Doc)
{
	Gui::Command::openCommand("Add Cam Feature");
	std::string FeatName = Doc->getUniqueObjectName("Cam Feature");

	// create the object
	App::DocumentObject *camFeat =  Doc->addObject("Cam::CamFeature", FeatName.c_str());
	if(camFeat && camFeat->isDerivedFrom(Cam::CamFeature::getClassTypeId())) {
		Cam::CamFeature *camFeature = dynamic_cast<Cam::CamFeature *>(camFeat);
		if (camFeature != NULL)
		{
			camFeature->initialise();

			Gui::Command::commitCommand();
	//		Doc->recompute();
			return camFeature;
		}
		else
		{
			Base::Console().Error("Unable to create Cam Feature\n");

			Gui::Command::abortCommand();
			return NULL;
		}
	}
	else {
		Base::Console().Error("Unable to create Cam Feature\n");

		Gui::Command::abortCommand();
		return NULL;
	}
}

/**
 * Creates a new TPGFeature and adds it to the CamFeature
 */
Cam::TPGFeature *UIManagerInst::makeTPGFeature(App::Document* Doc, Cam::CamFeature *CamFeature, Cam::TPGDescriptor *tpgDescriptor)
{
	Gui::Command::openCommand("Add TPG");
    std::string tpgFeatName = Doc->getUniqueObjectName(tpgDescriptor->name.toStdString().c_str());
    App::DocumentObject *tpgFeat =  Doc->addObject("Cam::TPGFeature", tpgFeatName.c_str());
    if(tpgFeat && tpgFeat->isDerivedFrom(Cam::TPGFeature::getClassTypeId())) {
		Cam::TPGFeature *tpgFeature = dynamic_cast<Cam::TPGFeature *>(tpgFeat);

		// We Must Initialise the TPG Feature before usage
		tpgFeature->initialise();

		// Add descriptor details
		tpgFeature->PluginId.setValue(tpgDescriptor->id.toStdString().c_str());
		CamFeature->addTPG(tpgFeature);
//		Doc->recompute();

	    Gui::Command::commitCommand();
		return tpgFeature;
    }
	if (tpgFeat)
		Base::Console().Error("Object not TPG Feature\n");
	else
		Base::Console().Error("Unable to create TPG Feature\n");
	Gui::Command::abortCommand();
	return NULL;
}

/**
 * A Slot to request a Library reload.
 */
void UIManagerInst::reloadTPGs()
{
    // scan for TPGs
    Cam::TPGFactory().scanPlugins();

    // get the TPGs
    Cam::TPGDescriptorCollection *plugins = Cam::TPGFactory().getDescriptors();

    CamGui::TPGListModel *model = new CamGui::TPGListModel(plugins);
    plugins->release();
    Q_EMIT updatedTPGList(model);
}

/**
 * A Slot to relay TPG state updates to UI Components
 */
void UIManagerInst::updatedTPGState(QString tpgid, Cam::TPG::State state, int progress) {

	Q_EMIT updatedTPGStateSig(tpgid, state, progress);
}

#include "moc_UIManager.cpp"
