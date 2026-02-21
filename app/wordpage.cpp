/*
    This file is part of Kiten, a KDE Japanese Reference Tool
    SPDX-FileCopyrightText: 2025 Kiten developers

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "wordpage.h"

#include "DictKanjidic/entrykanjidic.h"
#include "dictionarymanager.h"
#include "dictquery.h"
#include "entrylist.h"
#include "kitenconfig.h"

#include <KColorScheme>
#include <KLocalizedString>

#include <QTextBrowser>
#include <QUrl>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

WordPage::WordPage(QWidget *parent)
    : QWidget(parent)
    , _browser(new QTextBrowser(this))
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_browser);

    _browser->setOpenLinks(false);
    _browser->setOpenExternalLinks(false);
    connect(_browser, &QTextBrowser::anchorClicked, this, &WordPage::handleLinkClicked);
}

void WordPage::setWord(const QString &word, const QString &reading, DictionaryManager *dictManager)
{
    // Search for this word
    DictQuery query(word);
    EntryList *results = dictManager->doSearch(query);

    // Find the best matching entry (prefer exact word match with the given reading)
    Entry *bestEntry = nullptr;
    for (int i = 0; i < results->count(); ++i) {
        Entry *entry = results->at(i);
        // Skip kanjidic entries
        if (dynamic_cast<EntryKanjidic *>(entry)) {
            continue;
        }
        if (entry->getWord() == word) {
            if (!reading.isEmpty() && entry->getReadings().contains(reading)) {
                bestEntry = entry;
                break;
            }
            if (!bestEntry) {
                bestEntry = entry;
            }
        }
    }

    // If no exact match, use the first non-kanjidic entry
    if (!bestEntry) {
        for (int i = 0; i < results->count(); ++i) {
            Entry *entry = results->at(i);
            if (!dynamic_cast<EntryKanjidic *>(entry)) {
                bestEntry = entry;
                break;
            }
        }
    }

    QString html;
    html += QStringLiteral("<html><head><style>%1</style></head><body>").arg(generateCSS());

    if (bestEntry) {
        // Word header with clickable kanji
        QString linkedWord;
        for (int i = 0; i < bestEntry->getWord().length(); ++i) {
            QChar ch = bestEntry->getWord().at(i);
            if (isCJKCharacter(ch)) {
                linkedWord += QStringLiteral("<a href=\"kanji:%1\">%1</a>").arg(ch);
            } else {
                linkedWord += ch;
            }
        }

        html += QStringLiteral("<div class=\"word-header\">"
                               "<span class=\"word-main\">%1</span>"
                               "</div>")
                    .arg(linkedWord);

        // Reading
        if (!bestEntry->getReadings().isEmpty()) {
            html += QStringLiteral("<div class=\"word-reading\">%1</div>")
                        .arg(bestEntry->getReadings());
        }

        // Meanings
        if (!bestEntry->getMeaningsList().isEmpty()) {
            html += QStringLiteral("<div class=\"section\">"
                                   "<p class=\"section-title\">%1</p><ol>")
                        .arg(i18n("Meanings"));
            for (const QString &meaning : bestEntry->getMeaningsList()) {
                html += QStringLiteral("<li>%1</li>").arg(meaning);
            }
            html += QStringLiteral("</ol></div>");
        }

        // Kanji breakdown
        QList<QChar> kanjiChars;
        for (int i = 0; i < bestEntry->getWord().length(); ++i) {
            QChar ch = bestEntry->getWord().at(i);
            if (isCJKCharacter(ch)) {
                kanjiChars.append(ch);
            }
        }

        if (!kanjiChars.isEmpty()) {
            html += QStringLiteral("<div class=\"section\">"
                                   "<p class=\"section-title\">%1</p>")
                        .arg(i18n("Kanji in this word"));

            for (const QChar &ch : kanjiChars) {
                // Look up each kanji
                DictQuery kanjiQuery{QString{ch}};
                EntryList *kanjiResults = dictManager->doSearch(kanjiQuery);

                EntryKanjidic *kanjiEntry = nullptr;
                for (int j = 0; j < kanjiResults->count(); ++j) {
                    auto entry = dynamic_cast<EntryKanjidic *>(kanjiResults->at(j));
                    if (entry) {
                        kanjiEntry = entry;
                        break;
                    }
                }

                html += QStringLiteral("<div class=\"kanji-breakdown\">"
                                       "<a href=\"kanji:%1\" class=\"kanji-link\">%1</a>")
                            .arg(ch);

                if (kanjiEntry) {
                    QString readings;
                    if (!kanjiEntry->getOnyomiReadingsList().isEmpty()) {
                        readings += kanjiEntry->getOnyomiReadings();
                    }
                    if (!kanjiEntry->getKunyomiReadingsList().isEmpty()) {
                        if (!readings.isEmpty()) {
                            readings += QStringLiteral(" / ");
                        }
                        readings += kanjiEntry->getKunyomiReadings();
                    }

                    html += QStringLiteral(" <span class=\"kanji-mini-reading\">%1</span>"
                                           " <span class=\"kanji-mini-meaning\">%2</span>")
                                .arg(readings, kanjiEntry->getMeanings());

                    if (!kanjiEntry->getStrokesCount().isEmpty()) {
                        html += QStringLiteral(" <span class=\"kanji-mini-meta\">(%1 %2)</span>")
                                    .arg(kanjiEntry->getStrokesCount(), i18n("strokes"));
                    }
                }

                html += QStringLiteral("</div>");
                kanjiResults->deleteAll();
                delete kanjiResults;
            }

            html += QStringLiteral("</div>");
        }
    } else {
        html += QStringLiteral("<p>%1</p>").arg(i18n("No entry found for \"%1\"", word));
    }

    html += QStringLiteral("</body></html>");

    _browser->setHtml(html);

    results->deleteAll();
    delete results;
}

void WordPage::handleLinkClicked(const QUrl &url)
{
    QString urlStr = url.toString();

    if (urlStr.startsWith("kanji:"_L1)) {
        QString kanjiStr = urlStr.mid(6);
        if (!kanjiStr.isEmpty()) {
            Q_EMIT kanjiClicked(kanjiStr.at(0));
        }
    } else if (urlStr.length() == 1 && isCJKCharacter(urlStr.at(0))) {
        Q_EMIT kanjiClicked(urlStr.at(0));
    }
}

QString WordPage::generateCSS() const
{
    KColorScheme scheme(QPalette::Active);
    QFont font = KitenConfigSkeleton::self()->font();

    return QStringLiteral(
               "body { background-color: %1; color: %2; font-family: \"%3\"; font-size: %4pt; }"
               "a { text-decoration: none; color: %5; }"
               "a:hover { color: %6; }"
               ".word-header { margin: 10px 0; }"
               ".word-main { font-size: %7pt; }"
               ".word-reading { font-size: %8pt; color: %9; margin-bottom: 8px; }"
               ".section { margin: 8px 0; }"
               ".section-title { font-weight: bold; color: %9; border-bottom: 1px solid %10; margin-bottom: 4px; }"
               "ol { margin: 4px 0; padding-left: 20px; }"
               "li { margin: 2px 0; }"
               ".kanji-breakdown { margin: 4px 0; padding: 4px; }"
               ".kanji-link { font-size: %8pt; }"
               ".kanji-mini-reading { color: %9; }"
               ".kanji-mini-meaning { }"
               ".kanji-mini-meta { color: %9; font-size: 9pt; }")
        .arg(scheme.background(KColorScheme::NormalBackground).color().name())   // %1
        .arg(scheme.foreground().color().name())                                  // %2
        .arg(font.family())                                                       // %3
        .arg(font.pointSize())                                                    // %4
        .arg(scheme.foreground(KColorScheme::LinkText).color().name())            // %5
        .arg(scheme.foreground(KColorScheme::ActiveText).color().name())          // %6
        .arg(font.pointSize() + 10)                                              // %7
        .arg(font.pointSize() + 4)                                               // %8
        .arg(scheme.foreground(KColorScheme::InactiveText).color().name())        // %9
        .arg(scheme.shade(KColorScheme::MidlightShade).name());                  // %10
}

bool WordPage::isCJKCharacter(const QChar &ch)
{
    ushort value = ch.unicode();
    if (value < 255) {
        return false;
    }
    if (0x3040 <= value && value <= 0x30FF) {
        return false;
    }
    return true;
}

#include "moc_wordpage.cpp"
