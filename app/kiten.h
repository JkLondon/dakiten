/*
    This file is part of Kiten, a KDE Japanese Reference Tool...
    SPDX-FileCopyrightText: 2001 Jason Katz-Brown <jason@katzbrown.com>
    SPDX-FileCopyrightText: 2005 Paul Temple <paul.temple@gmx.net>
    SPDX-FileCopyrightText: 2006 Joseph Kerian <jkerian@gmail.com>
    SPDX-FileCopyrightText: 2006 Eric Kjeldergaard <kjelderg@gmail.com>
    SPDX-FileCopyrightText: 2011 Daniel E. Moctezuma <democtezuma@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KITEN_H
#define KITEN_H

#include <KXmlGuiWindow>

#include <QVariant>
#include <QList>

#include "dictionarymanager.h"
#include "dictquery.h"
#include "entry.h"
#include "historyptrlist.h"

class QAction;
class KProcess;
class QStackedWidget;
class QStatusBar;
class KToggleAction;
class KitenConfigSkeleton;

class QDockWidget;

class ConfigureDialog;
class DictionaryUpdateManager;
class EntryListView;
class KanjiPage;
class ResultsView;
class SearchResultsPage;
class SearchStringInput;
class WordPage;

class Kiten : public KXmlGuiWindow
{
    Q_OBJECT

    // Constructors and other setup/takedown related methods
public:
    explicit Kiten(QWidget *parent = nullptr, const char *name = nullptr);
    ~Kiten() override;

    KitenConfigSkeleton *getConfig();

public Q_SLOTS:
    void searchTextAndRaise(const QString &str);
    void addExportListEntry(int index);

protected:
    void setupActions();
    void setupExportListDock();
    bool queryClose() override; // overridden from KXmlGuiWindow (called@shutdown) override

private Q_SLOTS:
    void finishInit();
    void focusResultsView();

    // Searching related methods
    void searchFromEdit();
    void searchText(const QString &);
    void searchClipboard();
    void searchAndDisplay(const DictQuery &);
    void searchInResults();
    void displayResults(EntryList *);
    void radicalSearch();
    void kanjiBrowserSearch();

    // Navigation slots for kanji/word pages
    void navigateToKanji(const QChar &kanji);
    void navigateToWord(const QString &word, const QString &reading);
    void showSearchResults();

    // Configuration related slots
    void slotConfigure();
    void configureToolBars();
    void newToolBarConfig();
    void updateConfiguration();
    void loadDictionaries();
    void loadDictConfig(const QString &);

    // Other
    void print();

    // History related methods
    void back();
    void forward();
    void goInHistory(int);
    void displayHistoryItem();
    void addHistory(EntryList *);
    void enableHistoryButtons();
    void setCurrentScrollValue(int value);

private:
    // Page type identifiers for navigation history
    enum class PageType {
        SearchResults,
        Kanji,
        Word,
    };

    struct PageState {
        PageType type;
        QVariant data; // QChar for Kanji, QStringList{word,reading} for Word
    };

    void pushPageState(const PageState &state);

    QStatusBar *_statusBar = nullptr;
    DictionaryManager _dictionaryManager;
    DictionaryUpdateManager *_dictionaryUpdateManager = nullptr;
    SearchStringInput *_inputManager = nullptr;

    // Page stack navigation
    QStackedWidget *_pageStack = nullptr;
    SearchResultsPage *_searchResultsPage = nullptr;
    KanjiPage *_kanjiPage = nullptr;
    WordPage *_wordPage = nullptr;

    // Page navigation history
    QList<PageState> _pageHistory;
    int _pageHistoryIndex = -1;
    bool _navigatingHistory = false;

    DictQuery _lastQuery;
    KToggleAction *_autoSearchToggle = nullptr;
    QAction *_irAction = nullptr;
    QAction *_backAction = nullptr;
    QAction *_forwardAction = nullptr;
    KProcess *_radselect_proc = nullptr;
    KProcess *_kanjibrowser_proc = nullptr;

    // Export list related:
    QDockWidget *_exportListDock = nullptr;
    QWidget *_exportListDockContents = nullptr;
    EntryListView *_exportList = nullptr;

    ConfigureDialog *_optionDialog = nullptr;
    KitenConfigSkeleton *_config = nullptr;

    HistoryPtrList _historyList;
    QString _personalDict;
};

#endif
