/*
    This file is part of Kiten, a KDE Japanese Reference Tool
    SPDX-FileCopyrightText: 2025 Kiten developers

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "searchresultspage.h"

#include "resultsview.h"

#include <QVBoxLayout>

SearchResultsPage::SearchResultsPage(QWidget *parent)
    : QWidget(parent)
    , _resultsView(new ResultsView(this, "mainView"))
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_resultsView);

    connect(_resultsView, &ResultsView::urlClicked, this, &SearchResultsPage::handleUrlClicked);
}

ResultsView *SearchResultsPage::resultsView() const
{
    return _resultsView;
}

void SearchResultsPage::handleUrlClicked(const QString &url)
{
    // A single CJK character link means the user clicked a kanji
    if (url.length() == 1 && isCJKCharacter(url.at(0))) {
        Q_EMIT kanjiClicked(url.at(0));
    } else {
        Q_EMIT wordSearchRequested(url);
    }
}

bool SearchResultsPage::isCJKCharacter(const QChar &ch)
{
    ushort value = ch.unicode();
    if (value < 255) {
        return false;
    }
    // Exclude kana ranges (hiragana 0x3040-0x309F, katakana 0x30A0-0x30FF)
    if (0x3040 <= value && value <= 0x30FF) {
        return false;
    }
    return true;
}

#include "moc_searchresultspage.cpp"
