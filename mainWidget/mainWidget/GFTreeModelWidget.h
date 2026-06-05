#pragma once
#include <QTreeWidget>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QComboBox>


#include <TopoDS_Shape.hxx>
#include "GFTreeWidget.h"
#include "WordExporter.h"

#include "ProjectPropertyWidge.h"
#include "GeomPropertyWidget.h"
#include "MaterialPropertyWidget.h"
#include "DatabasePropertyWidget.h"



class GFTreeModelWidget :public QWidget
{
	Q_OBJECT
public:
	GFTreeModelWidget(QWidget*parent = nullptr);
	~GFTreeModelWidget();

	void init();

	void bindConnect();

	GFTreeWidget* GetGFTreeWidget() {return m_TreeWidget;}

public:
	void updataIcon();

	//void updateCheckBoxFunc(QModelIndex index);

protected:
	void contextMenuEvent(QContextMenuEvent *event) override;

signals:
	void itemClicked(const QString& itemData);

private slots:
	void onTreeItemClicked(QTreeWidgetItem* item, int column);

	void exportWord(const QString& directory, QTreeWidgetItem* item);

private:
	QMenu* m_ContextMenu = nullptr;
	GFTreeWidget* m_TreeWidget = nullptr;

	TopoDS_Shape* m_TopoDSShape = nullptr;
	WordExporter* m_WordExporter = nullptr;
};

