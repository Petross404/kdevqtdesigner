#include "integration.h"

#include <QtDesigner/QtDesigner>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/icore.h>

#include <QtGlobal>

LocalDesignerIntegration::LocalDesignerIntegration (QDesignerFormEditorInterface *core, QObject *parent)
   : QDesignerIntegration (core, parent),
     m_formEditor (core)
{
   setFeatures (features () | SlotNavigationFeature);

   connect (this, SIGNAL(navigateToSlot(QString, QString, QStringList)),
            this, SLOT(slotNavigateToSlot(QString, QString, QStringList)));
}

void LocalDesignerIntegration::slotNavigateToSlot(const QString &objectName, const QString &signalSignature, const QStringList &parameterNames)
{
    qInfo () << "Navigate to slot ";
    // kDebug () << "UI file:" << m_formEditor->activeEditor()->activeFormWindow()->file()->fileName();
    qInfo () << "object name:" << objectName;
    qInfo () << "signal:" << signalSignature;

    QUrl url = KDevelop::ICore::self()->documentController()->activeDocument()->url();
    if (url.isLocalFile())
    {
       QString fileName = url.path();
       qInfo () << "file name:" << fileName;
       if (fileName.endsWith (".ui"))
       {
           fileName.replace (fileName.size()-3, 3, ".cc");
           url = QUrl (fileName);
           qInfo () << "url:" << url;
           KDevelop::IDocument* doc = KDevelop::ICore::self()->documentController()->documentForUrl (url);
           if (doc != NULL)
              qInfo () << "document:" << doc;

           if (doc != NULL && doc->isTextDocument())
           {
              qInfo () << "inserting text";
              // KTextEditor::Range range = doc->textSelection();
              // if ( !range.isValid() )
              //     range = KTextEditor::Range(doc->cursorPosition(), doc->cursorPosition());

              QString text = "void on_" + objectName + "_" + signalSignature + "\n";
              text += "{\n";
              text += "}\n";

              KTextEditor::Document* document = doc->textDocument();
              KTextEditor::Cursor cursor = document->documentEnd();
              document->insertText (cursor, text);

              if (doc->activeTextView() != NULL)
                 doc->activeTextView()->setFocus();
           }
       }
    }
}

#include "integration.moc"
