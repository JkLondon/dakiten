/*
    This file is part of Kiten, a KDE Japanese Reference Tool
    SPDX-FileCopyrightText: 2025 Kiten developers

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KANJIPAGE_H
#define KANJIPAGE_H

#include <QWidget>

class QTextBrowser;
class DictionaryManager;

class KanjiPage : public QWidget
{
    Q_OBJECT

public:
    explicit KanjiPage(QWidget *parent = nullptr);

    void setKanji(const QChar &kanji, DictionaryManager *dictManager);
    QChar currentKanji() const;

Q_SIGNALS:
    void kanjiClicked(const QChar &kanji);
    void wordClicked(const QString &word, const QString &reading);

private Q_SLOTS:
    void handleLinkClicked(const QUrl &url);

private:
    QString generateCSS() const;
    static bool isCJKCharacter(const QChar &ch);

    QTextBrowser *_browser;
    QChar _currentKanji;
};

#endif
