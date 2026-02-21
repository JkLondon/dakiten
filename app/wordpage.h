/*
    This file is part of Kiten, a KDE Japanese Reference Tool
    SPDX-FileCopyrightText: 2025 Kiten developers

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WORDPAGE_H
#define WORDPAGE_H

#include <QWidget>

class QTextBrowser;
class DictionaryManager;

class WordPage : public QWidget
{
    Q_OBJECT

public:
    explicit WordPage(QWidget *parent = nullptr);

    void setWord(const QString &word, const QString &reading, DictionaryManager *dictManager);

Q_SIGNALS:
    void kanjiClicked(const QChar &kanji);

private Q_SLOTS:
    void handleLinkClicked(const QUrl &url);

private:
    QString generateCSS() const;
    static bool isCJKCharacter(const QChar &ch);

    QTextBrowser *_browser;
};

#endif
