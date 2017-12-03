/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "qtdesignerwidget.h"

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormWindowManagerInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormEditorPluginInterface>
#include <QtCore/QPluginLoader>
#include <QMdiSubWindow>

#include <kdebug.h>
#include <QDebug>

//#include <klocale.h>
#include <QLocale>

//#include <kaction.h>
#include <QAction>

#include <kstandardaction.h>
#include <kactioncollection.h>


#include <sublime/view.h>

#include "qtdesignerdocument.h"
#include "qtdesignerplugin.h"

QtDesignerWidget::QtDesignerWidget( QWidget* parent, QtDesignerDocument* document )
    : QMdiArea( parent ), KXMLGUIClient(), m_document( document )
{
    //     area->setScrollBarsEnabled( true ); //FIXME commented just to make it compile with the new qt-copy
    //     area->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    //     area->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );

    QDesignerFormWindowInterface* form = m_document->form();

    setComponentData( m_document->designerPlugin()->componentData() );
    setXMLFile( "kdevqtdesigner.rc" );

    QMdiSubWindow* window = addSubWindow(form, Qt::Window | Qt::WindowShadeButtonHint | Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
    const QSize containerSize = form->mainContainer()->size();
    const QSize containerMinimumSize = form->mainContainer()->minimumSize();
    const QSize containerMaximumSize = form->mainContainer()->maximumSize();
    const QSize decorationSize = window->geometry().size() - window->contentsRect().size();
    window->resize(containerSize+decorationSize);
    window->setMinimumSize(containerMinimumSize+decorationSize);
    if( containerMaximumSize == QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX) )
        window->setMaximumSize(containerMaximumSize);
    else
        window->setMaximumSize(containerMaximumSize+decorationSize);
    window->setWindowTitle( form->mainContainer()->windowTitle() );

    setupActions();
}

void QtDesignerWidget::setupActions()
{

    QDesignerFormWindowManagerInterface* manager = m_document->form()->core()->formWindowManager();
    KActionCollection* ac = actionCollection();

    KStandardAction::save( this, SLOT( save() ), ac);
    ac->addAction( "adjust_size", manager->actionAdjustSize() );
    ac->addAction( "break_layout", manager->actionBreakLayout() );
    ac->addAction( "designer_cut", manager->actionCut() );
    ac->addAction( "designer_copy", manager->actionCopy() );
    ac->addAction( "designer_paste", manager->actionPaste() );
    ac->addAction( "designer_delete", manager->actionDelete() );
    ac->addAction( "layout_grid", manager->actionGridLayout() );
    ac->addAction( "layout_horiz", manager->actionHorizontalLayout() );
    ac->addAction( "layout_vertical", manager->actionVerticalLayout() );
    ac->addAction( "layout_split_horiz", manager->actionSplitHorizontal() );
    ac->addAction( "layout_split_vert", manager->actionSplitVertical() );
    ac->addAction( "designer_undo", manager->actionUndo() );
    ac->addAction( "designer_redo", manager->actionRedo() );
    ac->addAction( "designer_select_all", manager->actionSelectAll() );
    QAction* action = ac->addAction( "widgeteditor" );
    action->setCheckable( true );
    action->setChecked( true );
    action->setText( i18n("Edit Widgets") );
    connect( action, SIGNAL(triggered()), SLOT(editWidgets()));
    foreach (QObject *plugin, QPluginLoader::staticInstances())
    {
        if ( !plugin )
            continue;

        kDebug() << "checking plugin:" << plugin;
        QDesignerFormEditorPluginInterface *fep;

        if ( (fep = qobject_cast<QDesignerFormEditorPluginInterface*>(plugin)) )
        {
            // action name may have '&', remove them
            QString actionText = fep->action()->text();
            actionText = actionText.remove('&');

            fep->action()->setCheckable(true);
            if( actionText == "Edit Signals/Slots" ) {
                connect(fep->action(), SIGNAL(triggered()), SLOT(editSignals()));
                actionCollection()->addAction("signaleditor", fep->action());
            }
            if( actionText == "Edit Buddies" ) {
                connect(fep->action(), SIGNAL(triggered()), SLOT(editBuddys()));
                actionCollection()->addAction("buddyeditor", fep->action());
            }
            if( actionText == "Edit Tab Order" ) {
                connect(fep->action(), SIGNAL(triggered()), SLOT(editTabOrder()));
                actionCollection()->addAction("tabordereditor", fep->action());
            }

            kDebug(9038) << "Added action:" << fep->action()->objectName() << "|" << fep->action()->text();
        }
    }
}

void QtDesignerWidget::editWidgets()
{
    QDesignerFormWindowInterface* form = m_document->form();
    form->editWidgets();
    actionCollection()->action("signaleditor")->setChecked(false);
    actionCollection()->action("buddyeditor")->setChecked(false);
    actionCollection()->action("tabordereditor")->setChecked(false);
}

void QtDesignerWidget::editBuddys()
{
    actionCollection()->action("widgeteditor")->setChecked(false);
    actionCollection()->action("signaleditor")->setChecked(false);
    actionCollection()->action("tabordereditor")->setChecked(false);
}

void QtDesignerWidget::editSignals()
{
    actionCollection()->action("widgeteditor")->setChecked(false);
    actionCollection()->action("buddyeditor")->setChecked(false);
    actionCollection()->action("tabordereditor")->setChecked(false);
}

void QtDesignerWidget::editTabOrder()
{
    actionCollection()->action("widgeteditor")->setChecked(false);
    actionCollection()->action("buddyeditor")->setChecked(false);
    actionCollection()->action("signaleditor")->setChecked(false);
}

void QtDesignerWidget::save()
{
    m_document->save();
}

#include "qtdesignerwidget.moc"

