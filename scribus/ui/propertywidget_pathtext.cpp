/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "propertywidget_pathtext.h"

#include <QSignalBlocker>

#include "iconmanager.h"
#include "localemgr.h"
#include "scribus.h"
#include "scribusapp.h"
#include "scribusdoc.h"
#include "selection.h"

PropertyWidget_PathText::PropertyWidget_PathText(QWidget* parent) : QFrame(parent)
{
	setupUi(this);

	layout()->setAlignment(Qt::AlignTop);

	startOffset->setValues(0, 30000, 2, 0);
	startOffset->setSingleStep(10);

	distFromCurve->setValues(-300, 300, 2, 0);
	distFromCurve->setSingleStep(10);

	languageChange();
	iconSetChange();

	connect(ScQApp, SIGNAL(localeChanged()), this, SLOT(localeChange()));
	connect(ScQApp, SIGNAL(iconSetChanged()), this, SLOT(iconSetChange()));
	connect(ScQApp, SIGNAL(labelVisibilityChanged(bool)), this, SLOT(toggleLabelVisibility(bool)));
}

void PropertyWidget_PathText::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;

	connect(m_ScMW, SIGNAL(UpdateRequest(int)), this  , SLOT(handleUpdateRequest(int)));
}

void PropertyWidget_PathText::setDoc(ScribusDoc *d)
{
	if (d == (ScribusDoc*) m_doc)
		return;

	if (m_doc)
	{
		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
	}

	m_doc  = d;
	m_item = nullptr;

	if (m_doc.isNull())
	{
		disconnectSignals();
		return;
	}

	m_unitRatio = m_doc->unitRatio();
	m_unitIndex = m_doc->unitIndex();

	startOffset->setMaximum( 30000 );
	startOffset->setMinimum( 0 );
	startOffset->setSingleStep(10);
	distFromCurve->setMaximum( 300 );
	distFromCurve->setMinimum( -300 );
	distFromCurve->setSingleStep(10);

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
}

void PropertyWidget_PathText::setCurrentItem(PageItem *item)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	//CB We shouldn't really need to process this if our item is the same one
	//maybe we do if the item has been changed by scripter.. but that should probably
	//set some status if so.
	//FIXME: This won't work until when a canvas deselect happens, m_item must be nullptr.
	//if (m_item == i)
	//	return;

	if (item && m_doc.isNull())
		setDoc(item->doc());

	m_item = item;

	disconnectSignals();
	configureWidgets();

	if (m_item)
	{
		if (m_item->isPathText())
		{
			pathTextType->setCurrentIndex(m_item->textPathType);
			flippedPathText->setChecked(m_item->textPathFlipped);
			showCurveCheckBox->setChecked(m_item->PoShow);
			distFromCurve->showValue(m_item->BaseOffs * -1 * m_unitRatio);
			if (m_item->itemText.paragraphStyle(0).alignment() == 1)
				startOffset->setMinimum(-3000);
			else
			{
				if (m_item->textToFrameDistLeft() < 0)
					m_item->setTextToFrameDistLeft(0);
				startOffset->setMinimum(0);
			}
			startOffset->showValue(m_item->textToFrameDistLeft() * m_unitRatio);
		}
		connectSignals();
	}
}

void PropertyWidget_PathText::connectSignals()
{
	connect(showCurveCheckBox, SIGNAL(clicked())     , this, SLOT(handlePathLine()));
	connect(pathTextType     , SIGNAL(activated(int)), this, SLOT(handlePathType()));
	connect(flippedPathText  , SIGNAL(toggled(bool))     , this, SLOT(handlePathFlip()));
	connect(startOffset      , SIGNAL(valueChanged(double)), this, SLOT(handlePathDist()));
	connect(distFromCurve    , SIGNAL(valueChanged(double)), this, SLOT(handlePathOffs()));
}

void PropertyWidget_PathText::disconnectSignals()
{
	disconnect(showCurveCheckBox, SIGNAL(clicked())     , this, SLOT(handlePathLine()));
	disconnect(pathTextType     , SIGNAL(activated(int)), this, SLOT(handlePathType()));
	disconnect(flippedPathText  , SIGNAL(toggled(bool))     , this, SLOT(handlePathFlip()));
	disconnect(startOffset      , SIGNAL(valueChanged(double)), this, SLOT(handlePathDist()));
	disconnect(distFromCurve    , SIGNAL(valueChanged(double)), this, SLOT(handlePathOffs()));
}

void PropertyWidget_PathText::configureWidgets()
{
	bool enabled = false;
	if (m_item && m_doc)
	{
		enabled  = m_item->isPathText();
		enabled &= (m_doc->m_Selection->count() == 1);
	}
	setEnabled(enabled);
}

void PropertyWidget_PathText::handleSelectionChanged()
{
	if (!m_doc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	setCurrentItem(currItem);
	updateGeometry();
}

void PropertyWidget_PathText::handleUpdateRequest(int /*updateFlags*/)
{
	// Nothing to do
}

void PropertyWidget_PathText::handlePathDist()
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_item->setTextToFrameDistLeft(startOffset->value() / m_unitRatio);
	m_doc->adjustItemSize(m_item);
	m_item->updatePolyClip();
	m_item->update();
	m_doc->regionsChanged()->update(QRect());
}

void PropertyWidget_PathText::handlePathFlip()
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_item->textPathFlipped = flippedPathText->isChecked();
	m_item->updatePolyClip();
	m_item->update();
	m_doc->regionsChanged()->update(QRect());
}

void PropertyWidget_PathText::handlePathLine()
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_item->PoShow = showCurveCheckBox->isChecked();
	m_item->update();
}

void PropertyWidget_PathText::handlePathOffs()
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_item->BaseOffs = -distFromCurve->value() / m_unitRatio;
	m_doc->adjustItemSize(m_item);
	m_item->updatePolyClip();
	m_item->update();
	m_doc->regionsChanged()->update(QRect());
}

void PropertyWidget_PathText::handlePathType()
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_item->textPathType = pathTextType->currentIndex();
	m_item->update();
	m_doc->regionsChanged()->update(QRect());
}

void PropertyWidget_PathText::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}


void PropertyWidget_PathText::iconSetChange()
{
	IconManager &im = IconManager::instance();

	startOffsetLabel->setPixmap(im.loadPixmap("text-on-path-offset"));
	distFromCurveLabel->setPixmap(im.loadPixmap("text-on-path-distance"));
	flippedPathText->setIcon(im.loadIcon("text-on-path-flip"));
}

void PropertyWidget_PathText::languageChange()
{
	QSignalBlocker pathTextTypeBlocker(pathTextType);
	int oldPathType = pathTextType->currentIndex();
	pathTextType->clear();
	pathTextType->addItem( tr("Default"));
	pathTextType->addItem( tr("Stair Step"));
	pathTextType->addItem( tr("Skew"));
	pathTextType->setCurrentIndex(oldPathType);
	
	showCurveCheckBox->setText( tr("Show Curve"));
	pathTextTypeLabel->setText( tr("Type"));
	startOffsetLabel->setText( tr("Start Offset"));
	distFromCurveLabel->setText( tr("Distance"));
	
	QString ptSuffix = tr(" pt");
	QString unitSuffix = m_doc ? unitGetSuffixFromIndex(m_doc->unitIndex()) : ptSuffix;
	startOffset->setSuffix(unitSuffix);
	distFromCurve->setSuffix(unitSuffix);
}

void PropertyWidget_PathText::unitChange()
{
	if (!m_doc)
		return;

	QSignalBlocker startOffsetBlocker(startOffset);
	QSignalBlocker distFromCurveBlocker(distFromCurve);

	m_unitRatio = m_doc->unitRatio();
	m_unitIndex = m_doc->unitIndex();

	startOffset->setNewUnit(m_unitIndex);
	distFromCurve->setNewUnit(m_unitIndex);
}

void PropertyWidget_PathText::localeChange()
{
	const QLocale& l(LocaleManager::instance().userPreferredLocale());
	startOffset->setLocale(l);
	distFromCurve->setLocale(l);
}

void PropertyWidget_PathText::toggleLabelVisibility(bool v)
{
	distFromCurveLabel->setLabelVisibility(v);
	startOffsetLabel->setLabelVisibility(v);
	pathTextTypeLabel->setLabelVisibility(v);
	curveLabel->setLabelVisibility(v);
	flipLabel->setLabelVisibility(v);
}
