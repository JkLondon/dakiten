/*
    This file is part of Kiten, a KDE Japanese Reference Tool
    SPDX-FileCopyrightText: 2025 Kiten developers

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SEARCHRESULTSPAGE_H
#define SEARCHRESULTSPAGE_H

#include <QWidget>

class ResultsView;

class SearchResultsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SearchResultsPage(QWidget *parent = nullptr);

    ResultsView *resultsView() const;

Q_SIGNALS:
    void kanjiClicked(const QChar &kanji);
    void wordSearchRequested(const QString &text);

private Q_SLOTS:
    void handleUrlClicked(const QString &url);

private:
    ResultsView *_resultsView;

    static bool isCJKCharacter(const QChar &ch);
};

#endif
