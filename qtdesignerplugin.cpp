/***************************************************************************
 *   Copyright 2005 Roberto Raggi <roberto@kdevelop.org>            *
 *   Copyright 2005 Harald Fernengel <harry@kdevelop.org>           *
 *   Copyright 2006 Matt Rogers <mattr@kde.org>                     *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                  *
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
#include "qtdesignerplugin.h"

#include <QObject>
#include <QAction>
#include <QFile>
#include <QTextStream>
#include <QtDesigner/QtDesigner>
#include <QtDesigner/QDesignerComponents>
#include <QPluginLoader>
#include <QMdiArea>

#include <QMimeType>
#include <kxmlguiwindow.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kparts/mainwindow.h>
#include <kparts/partmanager.h>
#include <kstandardaction.h>
#include <kactioncollection.h>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iuicontroller.h>
#include "qtdesignerdocument.h"
#include "integration.h"

K_PLUGIN_FACTORY_WITH_JSON (QtDesignerPluginFactory, "kdevqtdesigner.json", registerPlugin <QtDesignerPlugin>(); )

class QtDesignerDocumentFactory : public KDevelop::IDocumentFactory
{
public:
	QtDesignerDocumentFactory(QtDesignerPlugin *plugin)
		: KDevelop::IDocumentFactory(), m_plugin(plugin)
	{
	}

	KDevelop::IDocument *create(const QUrl &url, KDevelop::ICore *core)
	{
		QMimeDatabase db;
		qDebug() << "creating doc for designer?";
		QMimeType *mimetype = new QMimeType(db.mimeTypeForUrl(url));
		qDebug() << "mimetype for" << url << "is" << mimetype->name();

		if (mimetype->name() == "application/x-designer")
		{
			QtDesignerDocument *d = new QtDesignerDocument(url, core);
			d->setDesignerPlugin(m_plugin);
//             m_plugin->activateDocument(d);
			return d;
		}

		return Q_NULLPTR;
	}
private:
	QtDesignerPlugin *m_plugin;
};

class QtDesignerToolViewFactory : public KDevelop::IToolViewFactory
{
public:
	enum Type
	{
		WidgetBox,
		PropertyEditor,
		ActionEditor,
		ObjectInspector,
		SignalSlotEditor,
		ResourceEditor
	};
	QtDesignerToolViewFactory(QtDesignerPlugin *plugin, Type typ)
		: IToolViewFactory(), m_plugin(plugin), m_type(typ)
	{
	}

	virtual QWidget *create(QWidget *parent = Q_NULLPTR)
	{
		if (m_type == WidgetBox)
		{
			return m_plugin->designer()->widgetBox();
		}
		else if (m_type == PropertyEditor)
		{
			return m_plugin->designer()->propertyEditor();
		}
		else if (m_type == ActionEditor)
		{
			return m_plugin->designer()->actionEditor();
		}
		else if (m_type == ObjectInspector)
		{
			return m_plugin->designer()->objectInspector();
		}
		else if (m_type == SignalSlotEditor)
		{
			return QDesignerComponents::createSignalSlotEditor(m_plugin->designer(), Q_NULLPTR);
		}
		else if (m_type == ResourceEditor)
		{
			return QDesignerComponents::createResourceEditor(m_plugin->designer(), Q_NULLPTR);
		}

		qDebug() << "Type not found:" << m_type;
		return Q_NULLPTR;
	}
	virtual Qt::DockWidgetArea defaultPosition()
	{
		if (m_type == WidgetBox)
		{
			return Qt::LeftDockWidgetArea;
		}
		else if (m_type == PropertyEditor)
		{
			return Qt::RightDockWidgetArea;
		}
		else if (m_type == ActionEditor)
		{
			return Qt::RightDockWidgetArea;
		}
		else if (m_type == ObjectInspector)
		{
			return Qt::RightDockWidgetArea;
		}
		else if (m_type == SignalSlotEditor)
		{
			return Qt::BottomDockWidgetArea;
		}
		else if (m_type == ResourceEditor)
		{
			return Qt::BottomDockWidgetArea;
		}

		qDebug() << "Type not found:" << m_type;
		return Qt::TopDockWidgetArea;
	}

	virtual QString id() const
	{
		if (m_type == WidgetBox)
		{
			return "org.kevelop.qtdesigner.WidgetBox";
		}
		else if (m_type == PropertyEditor)
		{
			return "org.kevelop.qtdesigner.PropertyEditor";
		}
		else if (m_type == ActionEditor)
		{
			return "org.kevelop.qtdesigner.ActionEditor";
		}
		else if (m_type == ObjectInspector)
		{
			return "org.kevelop.qtdesigner.ObjectInspector";
		}
		else if (m_type == SignalSlotEditor)
		{
			return "org.kevelop.qtdesigner.SignalSlotEditor";
		}
		else if (m_type == ResourceEditor)
		{
			return "org.kevelop.qtdesigner.ResourceEditor";
		}

		return QString();
	}

private:
	QtDesignerPlugin *m_plugin;
	Type m_type;
};

QtDesignerPlugin::QtDesignerPlugin(QObject *parent, const QVariantList &args)
	: KDevelop::IPlugin("kdevqtdesigner" /*componentName()*/, parent),
	  m_docFactory(new QtDesignerDocumentFactory(this)),
	  m_widgetBoxFactory(Q_NULLPTR), m_propertyEditorFactory(Q_NULLPTR),
	  m_objectInspectorFactory(Q_NULLPTR), m_actionEditorFactory(Q_NULLPTR)
{
	Q_UNUSED(args)

        setXMLFile ("kdevqtdesigner.rc");

	QDesignerComponents::initializeResources();
//     connect( idc, SIGNAL( documentActivated( KDevelop::IDocument* ) ),
//              this, SLOT( activateDocument( KDevelop::IDocument* ) ) );

	m_formeditor = QDesignerComponents::createFormEditor(this);
	QDesignerComponents::initializePlugins(m_formeditor);

	qDebug() << "integration:" << m_formeditor->integration();

	//TODO apaku: if multiple mainwindows exist, this needs to be changed on mainwindow-change
	m_formeditor->setTopLevel(core()->uiController()->activeMainWindow());

	m_formeditor->setWidgetBox(QDesignerComponents::createWidgetBox(m_formeditor, Q_NULLPTR));

//    load the standard widgets
	m_formeditor->widgetBox()->setFileName(QLatin1String(":/trolltech/widgetbox/widgetbox.xml"));
	m_formeditor->widgetBox()->load();

	m_formeditor->setPropertyEditor(QDesignerComponents::createPropertyEditor(m_formeditor, Q_NULLPTR));
	m_formeditor->setActionEditor(QDesignerComponents::createActionEditor(m_formeditor, Q_NULLPTR));
	m_formeditor->setObjectInspector(QDesignerComponents::createObjectInspector(m_formeditor, Q_NULLPTR));

	m_designer = new LocalDesignerIntegration(m_formeditor, this);
	QDesignerIntegration::initializePlugins(m_formeditor);

	qDebug() << "integration now:" << m_formeditor->integration();

	m_designer->core()->widgetBox()->setObjectName(tr("Widget Box"));
	m_designer->core()->propertyEditor()->setObjectName(tr("Property Editor"));
	m_designer->core()->actionEditor()->setObjectName(tr("Action Editor"));
	m_designer->core()->objectInspector()->setObjectName(tr("Object Inspector"));


	foreach (QObject *plugin, QPluginLoader::staticInstances())
	{
		QDesignerFormEditorPluginInterface *fep;

		if ((fep = qobject_cast<QDesignerFormEditorPluginInterface *>(plugin)))
		{
			fep->initialize(designer());
		}
	}

	m_widgetBoxFactory = new QtDesignerToolViewFactory(this,
			QtDesignerToolViewFactory::WidgetBox);
	m_propertyEditorFactory = new QtDesignerToolViewFactory(this,
			QtDesignerToolViewFactory::PropertyEditor);
	m_actionEditorFactory = new QtDesignerToolViewFactory(this,
			QtDesignerToolViewFactory::ActionEditor);
	m_objectInspectorFactory = new QtDesignerToolViewFactory(this,
			QtDesignerToolViewFactory::ObjectInspector);
	m_signalSlotEditorFactory = new QtDesignerToolViewFactory(this,
			QtDesignerToolViewFactory::SignalSlotEditor);
	m_resourceEditorFactory = new QtDesignerToolViewFactory(this,
			QtDesignerToolViewFactory::ResourceEditor);

	core()->uiController()->addToolView("Widget Box", m_widgetBoxFactory);
	core()->uiController()->addToolView("Property Editor", m_propertyEditorFactory);
	core()->uiController()->addToolView("Action Editor", m_actionEditorFactory);
	core()->uiController()->addToolView("Object Inspector", m_objectInspectorFactory);
	core()->uiController()->addToolView("Signal/Slot Editor", m_signalSlotEditorFactory);
	core()->uiController()->addToolView("Resource Editor", m_resourceEditorFactory);

	KDevelop::IDocumentController *idc = core()->documentController();
	idc->registerDocumentForMimetype("application/x-designer", m_docFactory);
}

QtDesignerPlugin::~QtDesignerPlugin()
{
	m_formeditor->setParent(Q_NULLPTR); // ugly, but otherwise segmentation fault occurs
	// delete m_formeditor;
	delete m_designer;
	delete m_docFactory;
}

QDesignerFormEditorInterface *QtDesignerPlugin::designer() const
{
	return m_designer->core();
}

// bool QtDesignerPlugin::saveFile()
// {
//     KSaveFile uiFile( localFilePath() );
//     //FIXME: find a way to return an error. KSaveFile
//     //doesn't use the KIO error codes
//     if ( !uiFile.open() )
//         return false;
//
//     QTextStream stream ( &uiFile );
//     QByteArray windowXml = m_window->contents().toUtf8();
//     stream << windowXml;
//
//     if ( !uiFile.finalize() )
//         return false;
//
//     m_window->setDirty(false);
//     setModified(false);
//     return true;
// }
/*
void QtDesignerPlugin::saveActiveDocument()
{
    kDebug(9038) << "going to save:" << m_activeDoc;
    if( m_activeDoc )
    {
        m_activeDoc->save( KDevelop::IDocument::Default );
    }
}

void QtDesignerPlugin::activateDocument( KDevelop::IDocument* doc )
{
    if( doc->mimeType()->is( "application/x-designer" ) )
    {
        kDebug(9038) << "Doc activated:" << doc;
        m_activeDoc = doc;
    }
}*/

#include "qtdesignerplugin.moc"
// kate: indent-mode cstyle; indent-width 8; replace-tabs off; tab-width 8; 
