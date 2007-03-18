/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#include "multipleduplicate.h"
//#include "multipleduplicate.moc"

#include <q3buttongroup.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qtabwidget.h>

#include "scrspinbox.h"
#include "units.h"

MultipleDuplicate::MultipleDuplicate( int unitIndex, QWidget* parent, const char* name, Qt::WFlags fl )
	: QDialog(parent, name, fl),
	m_unitIndex(unitIndex)
{
	setupUi(this);
	//set tab order
	QWidget::setTabOrder(createGapRadioButton, horizShiftSpinBox);
	QWidget::setTabOrder(horizShiftSpinBox, vertShiftSpinBox);
	QWidget::setTabOrder(gridColsSpinBox, horizRCGapSpinBox);
	QWidget::setTabOrder(horizRCGapSpinBox, vertRCGapSpinBox);
	QWidget::setTabOrder(vertRCGapSpinBox, rotationSpinBox);
	
	//set up mspinboxes
	horizShiftSpinBox->setNewUnit(unitIndex);
	vertShiftSpinBox->setNewUnit(unitIndex);
	horizRCGapSpinBox->setNewUnit(unitIndex);
	vertRCGapSpinBox->setNewUnit(unitIndex);
	horizShiftSpinBox->setMinimum(-1000);
	vertShiftSpinBox->setMinimum(-1000);
	horizRCGapSpinBox->setMinimum(-1000);
	vertRCGapSpinBox->setMinimum(-1000);
	horizShiftSpinBox->setMaximum(1000);
	vertShiftSpinBox->setMaximum(1000);
	horizRCGapSpinBox->setMaximum(1000);
	vertRCGapSpinBox->setMaximum(1000);

	rotationSpinBox->setValues(-180.0, 180.0, 10, 0.0);
	
	copiesCreateButtonGroup->setButton(0);
	// signals and slots connections
	connect(copiesCreateButtonGroup, SIGNAL(clicked(int)), this, SLOT(setCopiesShiftGap(int)));
}

MultipleDuplicate::~MultipleDuplicate()
{
}

void MultipleDuplicate::setCopiesShiftGap(int sel)
{
	if (sel==0)
	{
		horizShiftLabel->setText( tr("&Horizontal Shift:"));
		vertShiftLabel->setText( tr("&Vertical Shift:"));
	}
	else
	{
		horizShiftLabel->setText( tr("&Horizontal Gap:"));
		vertShiftLabel->setText( tr("&Vertical Gap:"));
	}
}

void MultipleDuplicate::getMultiplyData(ItemMultipleDuplicateData& mdData)
{
	mdData.type = tabWidget->currentPageIndex();
	mdData.copyCount = numberOfCopiesSpinBox->value();
	mdData.copyShiftOrGap = copiesCreateButtonGroup->selectedId();
	mdData.copyShiftGapH = horizShiftSpinBox->value();
	mdData.copyShiftGapV = vertShiftSpinBox->value();
	mdData.copyRotation = rotationSpinBox->value();
	mdData.gridRows = gridRowsSpinBox->value();
	mdData.gridCols = gridColsSpinBox->value();
	mdData.gridGapH = horizRCGapSpinBox->value();
	mdData.gridGapV = vertRCGapSpinBox->value();
}
