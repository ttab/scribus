/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
/***************************************************************************
                          vruler.cpp  -  description
                             -------------------
    begin                : Wed Apr 11 2001
    copyright            : (C) 2001 by Franz Schmid
    email                : Franz.Schmid@altmuehlnet.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "vruler.h"

#include <QCursor>
#include <QDebug>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QPolygon>
#include <QRect>
#include <QRubberBand>

#include <cmath>

#include "canvasgesture_rulermove.h"
#include "scribusdoc.h"
#include "scribusview.h"
#include "units.h"
#include "util_gui.h"

#ifdef Q_OS_MACOS
constexpr int topline = 1;
#else
constexpr int topline = 3;
#endif
constexpr int bottomline = 24;
constexpr int scaleS = bottomline - 4;
constexpr int scaleM = bottomline - 8;
constexpr int scaleL = bottomline - 12;

Vruler::Vruler(ScribusView *pa, ScribusDoc *doc) : QWidget(pa),
	m_doc(doc),
	m_view(pa)
{
	setBackgroundRole(QPalette::Window);
	//setAutoFillBackground(true);
	setMouseTracking(true);
	rulerGesture = new RulerGesture(m_view, RulerGesture::VERTICAL);
	unitChange();
}

void Vruler::mousePressEvent(QMouseEvent *m)
{
	m_mousePressed = true;
	if (!m_doc->guidesPrefs().guidesShown)
		return;
	QApplication::setOverrideCursor(QCursor(Qt::SplitHCursor));
	m_view->startGesture(rulerGesture);
	m_view->registerMousePress(m->globalPosition());
}

void Vruler::mouseReleaseEvent(QMouseEvent *m)
{
	if (!m_mousePressed)
		return;
	if (rulerGesture->isActive())
		rulerGesture->mouseReleaseEvent(m);
	QApplication::restoreOverrideCursor();
	m_mousePressed = false;
}

void Vruler::mouseMoveEvent(QMouseEvent *m)
{
	if (m_mousePressed && rulerGesture->isActive())
		rulerGesture->mouseMoveEvent(m);
}

void Vruler::paintEvent(QPaintEvent *e)
{
	if (m_doc->isLoading())
		return;
	QString tx;
	double sc = m_view->scale();
	
	const QPalette& palette = this->palette();
	const QColor& textColor = palette.color(QPalette::Text);
	
	QFont ff = font();
	ff.setPointSize(6);
	setFont(ff);
	QPainter p;
	p.begin(this);
	p.save();
	p.setClipRect(e->rect());
	p.setBrush(textColor);
	p.setPen(textColor);
	p.setFont(font());
	p.fillRect(rect(), palette.color(QPalette::Base));

	double cc = height() / sc;
	double firstMark = ceil(m_offset / m_iter) * m_iter - m_offset;
	while (firstMark < cc)
	{
		p.drawLine(scaleS, qRound(firstMark * sc), bottomline, qRound(firstMark * sc));
		firstMark += m_iter;
	}
	firstMark = ceil(m_offset / m_iter2) * m_iter2 - m_offset;
	int markC = static_cast<int>(ceil(m_offset / m_iter2));
	while (firstMark < cc)
	{
		int textY = qRound(firstMark * sc) + 10;
		int scaleOffset = scaleL;
		switch (m_doc->unitIndex())
		{
			case SC_MM:
				tx = QString::number(markC * m_iter2 / (m_iter2 / 100) / m_cor);
				break;
			case SC_IN:
				{
					double xl = (markC * m_iter2 / m_iter2) / m_cor;
					tx = QString::number(static_cast<int>(xl));
					double frac = fabs(xl - static_cast<int>(xl));
					if ((static_cast<int>(xl) == 0) && (frac > 0.1))
						tx.clear();
					if ((frac > 0.24) && (frac < 0.26))
						tx += QChar(0xBC);
					if ((frac > 0.49) && (frac < 0.51))
						tx += QChar(0xBD);
					if ((frac > 0.74) && (frac < 0.76))
						tx += QChar(0xBE);
				}
				break;
			case SC_P:
			case SC_C:
				tx = QString::number(markC * m_iter2 / (m_iter2 / 5) / m_cor);
				break;
			case SC_CM:
				tx = QString::number(markC * m_iter2 / m_iter2 / m_cor);
				break;
			default:
				tx = QString::number(markC * m_iter2);
				break;
		}
		p.drawLine(scaleOffset, qRound(firstMark * sc), bottomline, qRound(firstMark * sc));
		drawNumber(tx, textY, &p);
		firstMark += m_iter2;
		markC++;
	}

	if (m_doc->unitIndex() != SC_C || m_doc->unitIndex() != SC_P)
	{
		double tickStep = m_iter2 / 2.0;
		firstMark = ceil(m_offset / tickStep) * tickStep - m_offset;
		int markM = static_cast<int>(ceil(m_offset / tickStep));
		while (firstMark < cc)
		{
			p.drawLine(scaleM, qRound(firstMark * sc), bottomline, qRound(firstMark * sc));
			firstMark += tickStep;
			markM++;
		}
	}
	p.restore();
	if (m_drawMark)
	{
		// draw slim marker
		const QColor& markerColor = blendColor(isDarkColor(this->palette().color(QPalette::Base)), QColor(255, 117, 102), QColor(255, 71, 51));
		QPolygon cr;
		cr.setPoints(3,  5, m_whereToDraw, 0, m_whereToDraw + 2, 0, m_whereToDraw - 2);

	//	p.resetTransform();
		p.translate(0, -m_view->contentsY());
		p.setPen(markerColor);
		p.drawLine(0, m_whereToDraw, bottomline, m_whereToDraw);
		p.setRenderHints(QPainter::Antialiasing, true);
		p.setPen(Qt::NoPen);
		p.setBrush(markerColor);
		p.drawPolygon(cr);
		p.setRenderHints(QPainter::Antialiasing, false);
	}
	p.end();
}

void Vruler::drawNumber(const QString& num, int starty, QPainter *p) const
{
	int textY = starty;
	int fs = font().pointSize() + 2;

	for (int i = 0; i < num.length(); ++i)
	{
		p->drawText(scaleM - fs, textY, num.mid(i, 1));
		textY += fs;
	}
}

double Vruler::ruleSpacing() const
{
	return m_iter;
}

void Vruler::draw(int where)
{
	// erase old marker
	int currentCoor = where - m_view->contentsY();
	m_whereToDraw = where;
	m_drawMark = true;
	repaint(0, m_oldMark - 4, bottomline, 8);
//	m_drawMark = false;
	m_oldMark = currentCoor;
}

void Vruler::unitChange()
{
	double sc = m_view->scale();
	m_cor = 1;
	int docUnitIndex = m_doc->unitIndex();
	switch (docUnitIndex)
	{
		case SC_PT:
			if (sc > 1 && sc <= 4)
				m_cor = 2;
			if (sc > 4)
				m_cor = 10;
			if (sc < 0.3)
			{
				m_iter = unitRulerGetIter1FromIndex(docUnitIndex) * 3;
				m_iter2 = unitRulerGetIter2FromIndex(docUnitIndex) * 3;
			}
			else if (sc < 0.2)
			{
				m_iter = unitRulerGetIter1FromIndex(docUnitIndex) * 2;
				m_iter2 = unitRulerGetIter2FromIndex(docUnitIndex) * 2;
			}
			else
			{
				m_iter = unitRulerGetIter1FromIndex(docUnitIndex) / m_cor;
				m_iter2 = unitRulerGetIter2FromIndex(docUnitIndex) / m_cor;
	  		}
			break;
		case SC_MM:
			if (sc > 1)
				m_cor = 10;
			m_iter = unitRulerGetIter1FromIndex(docUnitIndex) / m_cor;
			m_iter2 = unitRulerGetIter2FromIndex(docUnitIndex) / m_cor;
			break;
		case SC_IN:
			m_iter = unitRulerGetIter1FromIndex(docUnitIndex);
			m_iter2 = unitRulerGetIter2FromIndex(docUnitIndex);
			if (sc > 1 && sc <= 4)
			{
				m_cor = 2;
				m_iter /= m_cor;
				m_iter2 /= m_cor;
				m_iter3 /= m_cor;
			}
			if (sc > 4)
			{
				m_cor = 4;
				m_iter /= m_cor;
				m_iter2 /= m_cor;
				m_iter3 /= m_cor;
			}
			if (sc < 0.25)
			{
				m_cor = 0.5;
				m_iter = 72.0*16.0;
				m_iter2 = 72.0*2.0;
				m_iter3 = 72.0*8.0;
			}
			break;
		case SC_P:
			m_iter = unitRulerGetIter1FromIndex(docUnitIndex);
			m_iter2 = unitRulerGetIter2FromIndex(docUnitIndex);
			if (sc >= 1 && sc <= 4)
			{
				m_cor = 1;
				m_iter = 12.0;
				m_iter2 = 60.0;
			}
			if (sc > 4)
			{
				m_cor = 2;
				m_iter = 6.0;
				m_iter2 = 12.0;
			}
			if (sc < 0.3)
			{
				m_cor = 0.25;
				m_iter = 12.0 * 4;
				m_iter2 = 60.0 * 4;
			}
			else if (sc < 1)
			{
				m_cor = 1;
				m_iter = 12.0;
				m_iter2 = 60.0;
			}
			break;
		case SC_CM:
			if (sc > 1 && sc <= 4)
				m_cor = 1;
			if (sc > 4)
				m_cor = 10;
			if (sc < 0.6)
			{
				m_cor = 0.1;
				m_iter = 720.0 / 25.4;
				m_iter2 = 7200.0 / 25.4;
			}
			else
			{
				m_iter = unitRulerGetIter1FromIndex(docUnitIndex) / m_cor;
				m_iter2 = unitRulerGetIter2FromIndex(docUnitIndex) / m_cor;
	  		}
			break;
		case SC_C:
			m_iter = unitRulerGetIter1FromIndex(docUnitIndex);
			m_iter2 = unitRulerGetIter2FromIndex(docUnitIndex);
			if (sc >= 1 && sc <= 4)
			{
				m_cor = 1;
				m_iter = 72.0 / 25.4 * 4.512;
				m_iter2 = 72.0 / 25.4 * 4.512 * 5.0;
			}
			if (sc > 4)
			{
				m_cor = 2;
				m_iter = 72.0 / 25.4 * 4.512 / 2.0;
				m_iter2 = 72.0 / 25.4 * 4.512;
			}
			if (sc < 0.3)
			{
				m_cor = 0.1;
				m_iter = 72.0 / 25.4 * 4.512 * 10;
				m_iter2 = 72.0 / 25.4 * 4.512 * 5.0 * 10;
			}
			else
				if (sc < 1)
			{
				m_cor = 1;
				m_iter = 72.0 / 25.4 * 4.512;
				m_iter2 = 72.0 / 25.4 * 4.512 * 5.0;
			}
			break;
		default:
			if (sc > 1 && sc <= 4)
				m_cor = 2;
			if (sc > 4)
				m_cor = 10;
			m_iter = unitRulerGetIter1FromIndex(0) / m_cor;
			m_iter2 = unitRulerGetIter2FromIndex(0) / m_cor;
			break;
	}
}
