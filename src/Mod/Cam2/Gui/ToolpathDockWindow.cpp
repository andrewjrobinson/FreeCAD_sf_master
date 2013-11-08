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
# include <QListWidget>
#endif
#include <QListWidgetItem>

#include "ToolpathDockWindow.h"
#include "UIManager.h"

#include "sstream"

namespace CamGui {

ToolpathDockWindow::ToolpathDockWindow(Gui::Document*  pcDocument, QWidget *parent)
  : DockWindow(pcDocument, parent), ui(new Ui_ToolpathDockWindow)
{
	setWindowTitle(QString::fromUtf8("Toolpath"));

    ui->setupUi(this);

    clearSelection();

    // listen for ToolPath object changes
    QObject::connect(&UIManager(), SIGNAL(updatedToolPathSelection(Cam::ToolPathFeature*)),
            this,SLOT(updatedToolPathSelection(Cam::ToolPathFeature*)));

    // listen for individual Toolpath selection events (from self)
    QObject::connect(ui->Toolpath, SIGNAL(itemSelectionChanged()),
            this, SLOT(itemSelectionChanged()));

    // listen for selection changes (from updatedToolPathLineSelection(std::set<int>
    QObject::connect(&UIManager(), SIGNAL(updatedToolPathLineSelection(std::set<int>)),
            this, SLOT(updatedToolPathLineSelection(std::set<int>)));


    updatingLineSelection = false;
    currentToolpath = NULL;

    //TODO: check if initial selection is such that a TP should be shown
}

ToolpathDockWindow::~ToolpathDockWindow() {
}

/**
 * Clear the output (i.e. no toolpath or machine program selected)
 */
void ToolpathDockWindow::clearSelection() {
    //TODO: make this simply hide the list and show a no selection message so the list can be restored (maintaining selection) when reselecting or changing from TP to MP and vice-versa
    ui->Toolpath->clear();
    ui->Toolpath->setSelectionMode(QAbstractItemView::NoSelection);
    ui->Toolpath->addItem(QString::fromUtf8("No tool-path/machine program selected"));
}

/**
 * Receive messages to update the toolpath display
 */
void ToolpathDockWindow::updatedToolPathSelection(Cam::ToolPathFeature* toolpath) {
    if (toolpath != NULL) {
        if (toolpath != currentToolpath) {
            const std::vector<std::string> tpcmds = toolpath->TPCommands.getValues();
            if (tpcmds.size() > 0) {

                // format the toolpath
                //TODO: do a colour coded rendering
                //INFO: http://www.qtcentre.org/threads/27777-Customize-QListWidgetItem-how-to?p=135369#post135369

                updatingLineSelection = true;
                ui->Toolpath->clear();
                std::vector<std::string>::const_iterator it;
                int i = 0;
                for (it = tpcmds.begin(); it != tpcmds.end(); ++it) {
                    QListWidgetItem *item = new QListWidgetItem();
                    item->setText(QString::fromStdString(*it));
                    item->setData(Qt::UserRole, QVariant::fromValue(i));
                    ui->Toolpath->addItem(item);
                }
                updatingLineSelection = false;
                ui->Toolpath->setSelectionMode(QAbstractItemView::ExtendedSelection);
            }
            else
                clearSelection();
        }
    }
    else
        clearSelection();
    currentToolpath = toolpath;
}

/**
 * Receive messages when selection changes on the list of toolpaths
 */
void ToolpathDockWindow::itemSelectionChanged() {
    QList<QListWidgetItem*> selected = ui->Toolpath->selectedItems();
    std::set<int> tpSelection;
    for (QList<QListWidgetItem*>::iterator it = selected.begin(); it != selected.end(); ++it) {
        QListWidgetItem* item = (*it);
        QVariant data = item->data(Qt::UserRole);
        tpSelection.insert(data.toInt());
    }
    if (!updatingLineSelection)
        UIManager().setToolPathLineSelection(tpSelection);
}

/**
 * Receive messages of selection change (from console)
 */
void ToolpathDockWindow::updatedToolPathLineSelection(std::set<int> tpSelection) {

    // check if current selection is different
    QList<QListWidgetItem*> selected = ui->Toolpath->selectedItems();
    bool different = false;
    std::set<int> tpSelectionCheck(tpSelection);
    for (QList<QListWidgetItem*>::iterator it = selected.begin(); it != selected.end(); ++it) {
        QListWidgetItem* item = (*it);
        QVariant data = item->data(Qt::UserRole);
        if (tpSelectionCheck.count(data.toInt() == 0)) {
            different = true;
            break;
        }
        tpSelectionCheck.erase(data.toInt());
    }
    if (tpSelectionCheck.size() > 0)
        different = true;

    // update only if selection is different (since we will get a endless loop otherwise)
    if (different) {
        updatingLineSelection = true;
        ui->Toolpath->clearSelection();
        for (int i = 0; i < ui->Toolpath->count(); ++i) {
            QListWidgetItem* item = ui->Toolpath->item(i);
            if (tpSelection.count(item->data(Qt::UserRole).toInt()) == 1)
                item->setSelected(true);
        }
        updatingLineSelection = false;
    }
}

#include "moc_ToolpathDockWindow.cpp"

} /* namespace Cam */
