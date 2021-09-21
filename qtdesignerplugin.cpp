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
#include "internals/qdesigner_integration_p.h"

K_PLUGIN_FACTORY(QtDesignerPluginFactory, registerPlugin<QtDesignerPlugin>(); )
K_EXPORT_PLUGIN(QtDesignerPluginFactory(KAboutData("kdevqtdesigner","kdevqtdesigner", ki18n("Qt Designer"), "0.1", ki18n("A GUI form designer for the Qt toolkit"), KAboutData::License_GPL)))

class QtDesignerDocumentFactory : public KDevelop::IDocumentFactory
{
public:
    QtDesignerDocumentFactory(QtDesignerPlugin* plugin)
        : KDevelop::IDocumentFactory(), m_plugin(plugin)
    {
    }

    KDevelop::IDocument* create( const QUrl& url, KDevelop::ICore* core) override
    {
		QMimeDatabase db;
        qDebug() << "creating doc for designer?";
        QMimeType* mimetype = new QMimeType(db.mimeTypeForUrl(url));
        qDebug() << "mimetype for" << url << "is" << mimetype->name();
        if( mimetype->name() == "application/x-designer" )
        {
            QtDesignerDocument* d = new QtDesignerDocument(url, core);
            d->setDesignerPlugin(m_plugin);
//             m_plugin->activateDocument(d);
            return d;
        }
        return 0;
    }
    private:
        QtDesignerPlugin* m_plugin;
};

class QtDesignerToolViewFactory : public KDevelop::IToolViewFactory
{
public:
    enum class Type
    {
        WidgetBox,
        PropertyEditor,
        ActionEditor,
        ObjectInspector,
        SignalSlotEditor,
        ResourceEditor
    };
    QtDesignerToolViewFactory( QtDesignerPlugin* plugin, Type typ )
        : IToolViewFactory(), m_plugin(plugin), m_type(typ)
    {
    }

    QWidget* create(QWidget *parent = 0) override
    {
        if( m_type == Type::WidgetBox )
            return m_plugin->designer()->widgetBox();
        else if( m_type == Type::PropertyEditor )
            return m_plugin->designer()->propertyEditor();
        else if( m_type == Type::ActionEditor )
            return m_plugin->designer()->actionEditor();
        else if( m_type == Type::ObjectInspector )
            return m_plugin->designer()->objectInspector();
        else if( m_type == Type::SignalSlotEditor )
            return QDesignerComponents::createSignalSlotEditor(m_plugin->designer(), 0);
        else if( m_type == Type::ResourceEditor )
            return QDesignerComponents::createResourceEditor(m_plugin->designer(), 0);
        qDebug() << "Type not found:";
        return 0;
    }

    Qt::DockWidgetArea defaultPosition() const override
    {
        if( m_type == Type::WidgetBox )
            return Qt::LeftDockWidgetArea;
        else if( m_type == Type::PropertyEditor )
            return Qt::RightDockWidgetArea;
        else if( m_type == Type::ActionEditor )
            return Qt::RightDockWidgetArea;
        else if( m_type == Type::ObjectInspector )
            return Qt::RightDockWidgetArea;
        else if( m_type == Type::SignalSlotEditor )
            return Qt::BottomDockWidgetArea;
        else if( m_type == Type::ResourceEditor )
            return Qt::BottomDockWidgetArea;
        qDebug() << "Type not found";
        return Qt::TopDockWidgetArea;
    }

    virtual QString id() const override
    {
        if( m_type == Type::WidgetBox )
            return "org.kevelop.qtdesigner.WidgetBox";
        else if( m_type == Type::PropertyEditor )
            return "org.kevelop.qtdesigner.PropertyEditor";
        else if( m_type == Type::ActionEditor )
            return "org.kevelop.qtdesigner.ActionEditor";
        else if( m_type == Type::ObjectInspector )
            return "org.kevelop.qtdesigner.ObjectInspector";
        else if( m_type == Type::SignalSlotEditor )
            return "org.kevelop.qtdesigner.SignalSlotEditor";
        else if( m_type == Type::ResourceEditor )
            return "org.kevelop.qtdesigner.ResourceEditor";
        return QString();
    }

private:
    QtDesignerPlugin* m_plugin;
    Type m_type;
};

QtDesignerPlugin::QtDesignerPlugin(QObject *parent, const QVariantList &args)
//	: KDevelop::IPlugin(QtDesignerPluginFactory::componentName(),parent),
	: KDevelop::IPlugin( componentName(), parent), 
	  m_docFactory(new QtDesignerDocumentFactory(this)),
      m_widgetBoxFactory(0), m_propertyEditorFactory(0),
      m_objectInspectorFactory(0), m_actionEditorFactory(0)
{
    Q_UNUSED(args)
    QDesignerComponents::initializeResources();
//     connect( idc, SIGNAL( documentActivated( KDevelop::IDocument* ) ),
//              this, SLOT( activateDocument( KDevelop::IDocument* ) ) );

    QDesignerFormEditorInterface* formeditor = QDesignerComponents::createFormEditor(this);
    QDesignerComponents::initializePlugins( formeditor );

    qDebug() << "integration:" << formeditor->integration();

    //TODO apaku: if multiple mainwindows exist, this needs to be changed on mainwindow-change
    formeditor->setTopLevel(core()->uiController()->activeMainWindow());

    formeditor->setWidgetBox(QDesignerComponents::createWidgetBox(formeditor, 0));

//    load the standard widgets
    formeditor->widgetBox()->setFileName(QLatin1String(":/trolltech/widgetbox/widgetbox.xml"));
    formeditor->widgetBox()->load();

    formeditor->setPropertyEditor(QDesignerComponents::createPropertyEditor(formeditor, 0));
    formeditor->setActionEditor(QDesignerComponents::createActionEditor(formeditor, 0));
    formeditor->setObjectInspector(QDesignerComponents::createObjectInspector(formeditor, 0));

    m_designer = new QDesignerIntegration(formeditor, this);
    //m_designer = new qdesigner_internal::QDesignerIntegration( formeditor, this );
    //qdesigner_internal::QDesignerIntegration::initializePlugins( formeditor );

    qDebug() << "integration now:" << formeditor->integration();

    m_designer->core()->widgetBox()->setObjectName( tr("Widget Box") );
    m_designer->core()->propertyEditor()->setObjectName( tr("Property Editor") );
    m_designer->core()->actionEditor()->setObjectName( tr("Action Editor") );
    m_designer->core()->objectInspector()->setObjectName( tr("Object Inspector") );


    foreach (QObject *plugin, QPluginLoader::staticInstances())
    {
        QDesignerFormEditorPluginInterface *fep;

        if ( (fep = qobject_cast<QDesignerFormEditorPluginInterface*>(plugin)) )
        {
            fep->initialize(designer());
        }
    }

    m_widgetBoxFactory = new QtDesignerToolViewFactory{this, QtDesignerToolViewFactory::Type::WidgetBox};
    m_propertyEditorFactory = new QtDesignerToolViewFactory( this,
            QtDesignerToolViewFactory::Type::PropertyEditor);
    m_actionEditorFactory = new QtDesignerToolViewFactory( this,
            QtDesignerToolViewFactory::Type::ActionEditor);
    m_objectInspectorFactory = new QtDesignerToolViewFactory( this,
            QtDesignerToolViewFactory::Type::ObjectInspector);
    m_signalSlotEditorFactory = new QtDesignerToolViewFactory( this,
            QtDesignerToolViewFactory::Type::SignalSlotEditor);
    m_resourceEditorFactory = new QtDesignerToolViewFactory( this,
            QtDesignerToolViewFactory::Type::ResourceEditor);

    core()->uiController()->addToolView("Widget Box", m_widgetBoxFactory );
    core()->uiController()->addToolView("Property Editor", m_propertyEditorFactory );
    core()->uiController()->addToolView("Action Editor", m_actionEditorFactory );
    core()->uiController()->addToolView("Object Inspector", m_objectInspectorFactory );
    core()->uiController()->addToolView("Signal/Slot Editor", m_signalSlotEditorFactory );
    core()->uiController()->addToolView("Resource Editor", m_resourceEditorFactory );

    KDevelop::IDocumentController* idc = core()->documentController();
    idc->registerDocumentForMimetype("application/x-designer", m_docFactory);
}

QtDesignerPlugin::~QtDesignerPlugin()
{
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
