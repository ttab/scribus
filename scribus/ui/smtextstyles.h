/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#ifndef SMTEXTSTYLES_H
#define SMTEXTSTYLES_H

#include "styleitem.h"
#include "styles/styleset.h"
#include "styles/paragraphstyle.h"

class CharStyle;
class ScribusDoc;
class SMCStyleWidget;
class SMPStyleWidget;
class SMCharacterStyle;

class SMParagraphStyle : public StyleItem
{
	Q_OBJECT
public:
	SMParagraphStyle(SMCharacterStyle* cstyleItem);
	~SMParagraphStyle();

	QTabWidget* widget();
	QString typeNamePlural();
	QString typeNameSingular();
	void setCurrentDoc(ScribusDoc *doc);
	QList<StyleName> styles(bool reloadFromDoc = true);
	void reload();
	void selected(const QStringList &styleNames);
	QString fromSelection() const;
	void toSelection(const QString &styleName) const;
	QString newStyle();
	QString newStyle(const QString &fromStyle);
	void apply();
	void editMode(bool isOn);
	bool isDefaultStyle(const QString &stylename) const;
	void setDefaultStyle(bool ids);
	QString shortcut(const QString &stylename) const;
	void setShortcut(const QString &shortcut);
	void deleteStyles(const QList<RemoveItem> &removeList);
	void nameChanged(const QString &newName);
	QString getUniqueName(const QString &name);
	void languageChange();
	void unitChange();
	StyleSet<ParagraphStyle>* tmpStyles(); // butt ugly

signals:
	void selectionDirty();

private:
	SMPStyleWidget *m_pwidget = nullptr;
	SMCharacterStyle* m_cstyleItem = nullptr;
	ScribusDoc *m_doc = nullptr;
	bool m_selectionIsDirty = false;
	double m_unitRatio = 1.0;
	StyleSet<ParagraphStyle> m_tmpStyles;
	StyleSet<CharStyle> *m_cstyles = nullptr;
	QList<ParagraphStyle*> m_selection;
	QList<RemoveItem> m_deleted;

	void setupConnections();
	void removeConnections();
	void updateStyleList();
	void reloadTmpStyles();
	QList<CharStyle> getCharStyles() const;

private slots:
	// pstyle
	void slotLineSpacingMode(int mode);
	void slotLineSpacing();
	void slotParentParaEffects();
	void slotPargraphEffects(int index);
	void slotDropCapLines(int lines);
	void slotParEffectOffset();
	void slotParEffectIndent(bool);
	void slotParEffectCharStyle(int);
	void slotBulletStr(const QString &str);
	void slotNumName(const QString &str);
	void slotNumNew();
	void slotSelectionDirty();
	void slotNumFormat(int);
	void slotNumLevel(int level);
	void slotNumPrefix(const QString &str);
	void slotNumSuffix(const QString &str);
	void slotNumStart(int start);
	void slotNumRestart(int restart);
	void slotNumOther(bool isOn);
	void slotNumHigher(bool isOn);
	void slotSpaceAbove();
	void slotSpaceBelow();
	void slotAlignment();
	void slotDirection();
	void slotOpticalMargin(int i);
	void slotOpticalMarginSelector();
	void slotMinSpace();
	void slotMinGlyphExt();
	void slotMaxGlyphExt();
	void slotConsecutiveLines();
	void handleKeepLinesStart();
	void handleKeepLinesEnd();
	void handleKeepTogether();
	void handleKeepWithNext();
	void slotTabRuler();
	void slotLeftIndent();
	void slotRightIndent();
	void slotFirstLine();
	void slotBackPColor();
	// cstyle
	void slotFontSize();
	void slotEffects(int e);
	void slotEffectProperties();
	void slotFillColor();
	void slotStrokeColor();
	void slotBackColor();
	void slotLanguage();
	void slotWordMin();
	void slotHyphenChar();
	void slotScaleH();
	void slotScaleV();
	void slotTracking();
	void slotWordTracking();
	void slotBaselineOffset();
	void slotFont(const QString& s);
	void slotParentChanged(const QString &parent);
	void slotCharParentChanged(const QString &parent);
	void slotFontFeatures();
	void slotCharStylesDeleted(const QList<RemoveItem> &removeList);
};

class SMCharacterStyle : public StyleItem
{
	Q_OBJECT
public:
	SMCharacterStyle();
	~SMCharacterStyle();

	QTabWidget* widget();
	QString typeNamePlural();
	QString typeNameSingular();
	void setCurrentDoc(ScribusDoc *doc);
	QList<StyleName> styles(bool reloadFromDoc = true);
	void reload();
	void selected(const QStringList &styleNames);
	QString fromSelection() const;
	void toSelection(const QString &styleName) const;
	QString newStyle();
	QString newStyle(const QString &fromStyle);
	void apply();
	void editMode(bool isOn);
	bool isDefaultStyle(const QString &stylename) const;
	void setDefaultStyle(bool ids);
	QString shortcut(const QString &stylename) const;
	void setShortcut(const QString &shortcut);
	void deleteStyles(const QList<RemoveItem> &removeList);
	void nameChanged(const QString &newName);
	QString getUniqueName(const QString &name);
	void languageChange();
	void unitChange();
	StyleSet<CharStyle>* tmpStyles();

signals:
	void selectionDirty();
	void charStylesDeleted(const QList<RemoveItem> &removeList);

private:
	QTabWidget   *m_widget = nullptr;
	SMCStyleWidget *m_page = nullptr;
	ScribusDoc   *m_doc = nullptr;
	StyleSet<CharStyle> m_tmpStyles;
	QList<CharStyle*> m_selection;
	QList<RemoveItem> m_deleted;
	bool m_selectionIsDirty = false;

	void reloadTmpStyles();
	void setupConnections();
	void removeConnections();

private slots:
	void slotFontSize();
	void slotEffects(int e);
	void slotEffectProperties();
	void slotFillColor();
	void slotBackColor();
	void slotStrokeColor();
	void slotLanguage();
	void slotScaleH();
	void slotScaleV();
	void slotTracking();
	void slotWordTracking();
	void slotBaselineOffset();
	void slotHyphenChar();
	void slotSmallestWord();
	void slotFont(const QString& s);
	void slotParentChanged(const QString &parent);
	void slotFontFeatures();
	void slotSelectionDirty();
};

#endif
