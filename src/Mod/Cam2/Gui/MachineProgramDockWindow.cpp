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
# include <QGridLayout>
#endif

#include "MachineProgramDockWindow.h"
#include "UIManager.h"

#include "sstream"

namespace CamGui {

MachineProgramDockWindow::MachineProgramDockWindow(Gui::Document*  pcDocument, QWidget *parent)
: DockWindow(pcDocument, parent), ui(new Ui_MachineProgramDockWindow)
{
	setWindowTitle(QString::fromUtf8("Machine program"));

    ui->setupUi(this);

    clearSelection();

	// listen for Machine Program Selection events
	QObject::connect(&UIManager(), SIGNAL(updatedMachineProgramSelection(Cam::MachineProgramFeature*)), this,
          SLOT(updatedMachineProgramSelection(Cam::MachineProgramFeature*)));

	updatingLineSelection = false;
    currentMachineProgram = NULL;

	//TODO: check if initial selection is such that a MP should be shown
}

MachineProgramDockWindow::~MachineProgramDockWindow() {
}

/**
 * Clear the output (i.e. no toolpath or machine program selected)
 */
void MachineProgramDockWindow::clearSelection() {
    ui->MachineProgram->clear();
    ui->MachineProgram->setSelectionMode(QAbstractItemView::NoSelection);
    ui->MachineProgram->addItem(QString::fromUtf8("No machine program/tool-path selected"));
}

/**
 * Receive messages to update the machineProgram display
 */
void MachineProgramDockWindow::updatedMachineProgramSelection(Cam::MachineProgramFeature* machineProgram) {
    if (machineProgram != NULL) {
        if (machineProgram != currentMachineProgram) {
            const std::vector<std::string> mpcmds = machineProgram->MPCommands.getValues();
            if (mpcmds.size() > 0) {

                // format the machine program
                //TODO: do a colour coded rendering
                //INFO: http://www.qtcentre.org/threads/27777-Customize-QListWidgetItem-how-to?p=135369#post135369

                updatingLineSelection = true;
                ui->MachineProgram->clear();
                std::vector<std::string>::const_iterator it;
                int i = 0;
                for (it = mpcmds.begin(); it != mpcmds.end(); ++it) {
                    QListWidgetItem *item = new QListWidgetItem();
                    item->setText(QString::fromStdString(*it));
                    item->setData(Qt::UserRole, QVariant::fromValue(i));
                    ui->MachineProgram->addItem(item);
                }
                updatingLineSelection = false;
                ui->MachineProgram->setSelectionMode(QAbstractItemView::ExtendedSelection);
            }
            else
                clearSelection();
        }
    }
    else
        clearSelection();
    currentMachineProgram = machineProgram;
}

#include "moc_MachineProgramDockWindow.cpp"

} /* namespace Cam */
