/***************************************************************************
 *   This file is part of KDevelop                                         *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>                           *
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

#ifndef QTDESIGNERDOCUMENT_H
#define QTDESIGNERDOCUMENT_H

#include <QtCore/QList>
#include <QUrl>

#include <sublime/urldocument.h>
#include <interfaces/idocument.h>
#include <QMimeType>
namespace KDevelop
{
	class ICore;
}

class QtDesignerPlugin;
class QDesignerFormWindowInterface;
class QMdiArea;

class QtDesignerDocument : public Sublime::UrlDocument, public KDevelop::IDocument
{
	Q_OBJECT
public:
	QtDesignerDocument( const QUrl&, KDevelop::ICore* );

	QUrl url() const override { return Sublime::UrlDocument::url(); }

	QMimeType mimeType() const override;
	KParts::Part* partForView(QWidget*) const override;
	KTextEditor::Document* textDocument() const override;
	bool save(KDevelop::IDocument::DocumentSaveMode = KDevelop::IDocument::Default) override;
	void reload() override;
	bool close(KDevelop::IDocument::DocumentSaveMode = KDevelop::IDocument::Default) override;
	bool isActive() const override;
	DocumentState state() const override;
	void setCursorPosition(const KTextEditor::Cursor&) override;
	void setTextSelection(const KTextEditor::Range &range) override;
	void activate(Sublime::View*, KParts::MainWindow*) override;
	KTextEditor::Cursor cursorPosition() const override;
	void setDesignerPlugin(QtDesignerPlugin*);
	QtDesignerPlugin* designerPlugin();
	QDesignerFormWindowInterface* form();
	virtual bool closeDocument();
private slots:
	void formChanged();
//     Sublime::View* newView(Sublime::Document* d) override;

private:
	QtDesignerPlugin* m_designerPlugin;
	KDevelop::IDocument::DocumentState m_state;
	QDesignerFormWindowInterface* m_form;
};

#endif

