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

#ifndef CAM_UIMANAGER_H_
#define CAM_UIMANAGER_H_

#include <QObject>
#include <QListView>

#include <boost/bind.hpp>
#include <boost/signals.hpp>

#include <Gui/Tree.h>
#include <Gui/ViewProviderDocumentObject.h>

#include "../App/Features/CamFeature.h"
#include "../App/Features/TPGFeature.h"
#include "../App/TPG/TPGFactory.h"
#include "../App/TPG/TPG.h"
#include "TPGListModel.h"


typedef boost::signals::connection Connection;

namespace CamGui {


/**
 * A class to manage the interactions between the various UI components of the
 * CAM workbench.
 *
 * TODO: use Boost Signal/Slots since that is what the main FreeCAD uses.
 */
class CamGuiExport UIManagerInst : public QObject,
								   public Gui::SelectionSingleton::ObserverType
{

  Q_OBJECT

protected:
  static UIManagerInst* _pcSingleton;

  QString tpgLibrarySelectedID;

  Connection highlightObjectConnection;

  /**
   * Creates a new CamFeature and adds it to the document
   */
  Cam::CamFeature *makeCamFeature(App::Document* Doc);

  /**
   * Creates a new TPGFeature and adds it to the CamFeature
   */
  Cam::TPGFeature *makeTPGFeature(App::Document* Doc, Cam::CamFeature *CamFeature, Cam::TPGDescriptor *tpgDescriptor);

  /**
   * Performs the work to analyse the current selection and send setting update signal
   */
  void updateCamProjectSelection(const char* pDocName);

public:
  UIManagerInst();
  virtual ~UIManagerInst();

  // singleton manipators
  static UIManagerInst& instance(void);
  static void destruct (void);

  // Implementations for GUI Commands
  bool CamFeature();
  /**
   * Used by the CamTPGFeature GUI Command to do the work required to add a TPGFeature
   */
  bool TPGFeature();
  /**
   * Used by the CamToolFeature GUI Command to do the work required to add a ToolFeature
   */
  bool ToolFeature();
  /**
   * Used by the CamMachineFeature GUI Command to do the work required to add a MachineFeature
   */
  bool MachineFeature();
  /**
   * Executes the selected TPG(s) to (re)produce its Tool Path.
   */
  bool RunTPG();
  /**
   * Executes the selected TPG(s) to (re)produce its Machine Program from the Tool Path.
   */
  bool PostProcess();
  /**
   * Debugging Command for testing.
   * TODO: remove this Command when no long needed
   */
  bool WatchHighlight();

  /**
   * Change the TPG Library selection.  This controls which TPG will be created
   * when the TPGFeature Gui Command is activated.
   */
  void setTPGLibrarySelection(Cam::TPGDescriptor *tpgDescriptor);

  //SLOTS (boost)
//  void onHighlightObject(const Gui::ViewProviderDocumentObject&, const Gui::HighlightMode&, bool flag);

  /// Receive selection change events so we can update the Cam settings Dock window
  virtual void OnChange(Gui::SelectionSingleton::SubjectType &rCaller,
                        Gui::SelectionSingleton::MessageType Reason);


  /**
   * Sets the currently selected tool path line(s)
   */
  void setToolPathLineSelection(std::set<int> &tpSelection);

public Q_SLOTS:
  void addTPG(Cam::TPGDescriptor *tpg);
  void reloadTPGs();
  void updatedTPGState(QString tpgid, Cam::TPG::State state, int progress);

Q_SIGNALS:
  void updatedTPGList(TPGListModel *model);

  void updatedTPGSelection(Cam::TPGFeature* tpg);
  void updatedToolPathSelection(Cam::ToolPathFeature* toolpath);
  void updatedMachineProgramSelection(Cam::MachineProgramFeature* machineProgram);

  void updatedTPGStateSig(QString tpgid, Cam::TPG::State state, int progress);

  void updatedToolPathLineSelection(std::set<int> tpSelection);
};

/// Get the global instance
inline UIManagerInst& UIManager(void)
{
    return UIManagerInst::instance();
}

} /* namespace CamGui */
#endif /* CAMGUI_UIMANAGER_H_ */
