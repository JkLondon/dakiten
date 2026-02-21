/*
    This file is part of Kiten, a KDE Japanese Reference Tool
    SPDX-FileCopyrightText: 2025 Kiten developers

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kanjipage.h"

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

KanjiPage::KanjiPage(QWidget *parent)
    : QWidget(parent)
    , _browser(new QTextBrowser(this))
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_browser);

    _browser->setOpenLinks(false);
    _browser->setOpenExternalLinks(false);
    connect(_browser, &QTextBrowser::anchorClicked, this, &KanjiPage::handleLinkClicked);
}

QChar KanjiPage::currentKanji() const
{
    return _currentKanji;
}

void KanjiPage::setKanji(const QChar &kanji, DictionaryManager *dictManager)
{
    _currentKanji = kanji;

    // Search KANJIDIC for this kanji
    DictQuery kanjiQuery{QString{kanji}};
    EntryList *kanjiResults = dictManager->doSearch(kanjiQuery);

    // Find the EntryKanjidic entry
    EntryKanjidic *kanjiEntry = nullptr;
    for (int i = 0; i < kanjiResults->count(); ++i) {
        auto entry = dynamic_cast<EntryKanjidic *>(kanjiResults->at(i));
        if (entry) {
            kanjiEntry = entry;
            break;
        }
    }

    QString html;
    html += QStringLiteral("<html><head><style>%1</style></head><body>").arg(generateCSS());

    if (kanjiEntry) {
        // Large kanji character header
        html += QStringLiteral("<div class=\"kanji-header\">"
                               "<span class=\"kanji-char\">%1</span>")
                    .arg(kanji);

        // Metadata: grade and strokes
        html += QStringLiteral("<span class=\"kanji-meta\">");
        if (!kanjiEntry->getKanjiGrade().isEmpty()) {
            html += QStringLiteral("<br>%1 %2").arg(i18n("Grade:"), kanjiEntry->getKanjiGrade());
        }
        if (!kanjiEntry->getStrokesCount().isEmpty()) {
            html += QStringLiteral("<br>%1 %2").arg(i18n("Strokes:"), kanjiEntry->getStrokesCount());
        }
        // Frequency
        QString freq = kanjiEntry->getExtendedInfoItem(QStringLiteral("F"));
        if (!freq.isEmpty()) {
            html += QStringLiteral("<br>%1 %2").arg(i18n("Frequency:"), freq);
        }
        html += QStringLiteral("</span></div>");

        // Readings section
        html += QStringLiteral("<div class=\"section\">");

        if (!kanjiEntry->getOnyomiReadingsList().isEmpty()) {
            html += QStringLiteral("<p class=\"label\">%1 <span class=\"reading\">%2</span></p>")
                        .arg(i18n("Onyomi:"), kanjiEntry->getOnyomiReadings());
        }

        if (!kanjiEntry->getKunyomiReadingsList().isEmpty()) {
            html += QStringLiteral("<p class=\"label\">%1 <span class=\"reading\">%2</span></p>")
                        .arg(i18n("Kunyomi:"), kanjiEntry->getKunyomiReadings());
        }

        if (!kanjiEntry->getInNamesReadingsList().isEmpty()) {
            html += QStringLiteral("<p class=\"label\">%1 <span class=\"reading\">%2</span></p>")
                        .arg(i18n("In names:"), kanjiEntry->getInNamesReadings());
        }

        if (!kanjiEntry->getAsRadicalReadingsList().isEmpty()) {
            html += QStringLiteral("<p class=\"label\">%1 <span class=\"reading\">%2</span></p>")
                        .arg(i18n("As radical:"), kanjiEntry->getAsRadicalReadings());
        }

        html += QStringLiteral("</div>");

        // Meanings section
        if (!kanjiEntry->getMeaningsList().isEmpty()) {
            html += QStringLiteral("<div class=\"section\">"
                                   "<p class=\"section-title\">%1</p>"
                                   "<p class=\"meanings\">%2</p>"
                                   "</div>")
                        .arg(i18n("Meanings"), kanjiEntry->getMeanings());
        }
    } else {
        // No KANJIDIC entry found — just show the character
        html += QStringLiteral("<div class=\"kanji-header\">"
                               "<span class=\"kanji-char\">%1</span>"
                               "<span class=\"kanji-meta\"><br>%2</span>"
                               "</div>")
                    .arg(kanji)
                    .arg(i18n("No kanji dictionary entry found"));
    }

    // Compounds section: search EDICT for words containing this kanji
    DictQuery compoundQuery{QString{kanji}};
    compoundQuery.setMatchType(DictQuery::Anywhere);
    EntryList *compoundResults = dictManager->doSearch(compoundQuery);

    if (compoundResults && compoundResults->count() > 0) {
        html += QStringLiteral("<div class=\"section\">"
                               "<p class=\"section-title\">%1</p>")
                    .arg(i18n("Compound Words"));

        // Collect non-kanjidic entries (edict words)
        struct CompoundInfo {
            QString word;
            QString reading;
            QString meaning;
            bool isCommon;
        };
        QList<CompoundInfo> compounds;

        for (int i = 0; i < compoundResults->count(); ++i) {
            Entry *entry = compoundResults->at(i);
            // Skip kanjidic entries (we only want word entries)
            if (dynamic_cast<EntryKanjidic *>(entry)) {
                continue;
            }
            CompoundInfo info;
            info.word = entry->getWord();
            info.reading = entry->getReadings();
            info.meaning = entry->getMeanings();
            info.isCommon = entry->extendedItemCheck(QStringLiteral("common"), QStringLiteral("1"));
            compounds.append(info);
        }

        // Sort: common words first, then alphabetical
        std::sort(compounds.begin(), compounds.end(), [](const CompoundInfo &a, const CompoundInfo &b) {
            if (a.isCommon != b.isCommon) {
                return a.isCommon;
            }
            return a.word < b.word;
        });

        // Limit to a reasonable number
        int limit = qMin(compounds.count(), 50);
        for (int i = 0; i < limit; ++i) {
            const auto &comp = compounds.at(i);

            // Link the whole compound word to the word page
            QString wordLink = QStringLiteral("<a href=\"word:%1:%2\">%1</a>")
                                   .arg(comp.word, comp.reading);

            QString commonMark = comp.isCommon ? QStringLiteral(" <span class=\"common-tag\">%1</span>").arg(i18n("common")) : QString();

            QString meaningShort = comp.meaning;
            // Truncate long meanings
            if (meaningShort.length() > 80) {
                meaningShort = meaningShort.left(77) + QStringLiteral("...");
            }

            html += QStringLiteral("<div class=\"compound\">"
                                   "<span class=\"compound-word\">%1</span>%2"
                                   " <span class=\"compound-reading\">(%3)</span>"
                                   " <span class=\"compound-meaning\">%4</span>"
                                   "</div>")
                        .arg(wordLink, commonMark, comp.reading, meaningShort);
        }

        if (compounds.count() > limit) {
            html += QStringLiteral("<p class=\"more\">%1</p>")
                        .arg(i18n("...and %1 more", compounds.count() - limit));
        }

        html += QStringLiteral("</div>");
    }

    html += QStringLiteral("</body></html>");

    _browser->setHtml(html);

    kanjiResults->deleteAll();
    delete kanjiResults;
    compoundResults->deleteAll();
    delete compoundResults;
}

void KanjiPage::handleLinkClicked(const QUrl &url)
{
    QString urlStr = url.toString();

    if (urlStr.startsWith("kanji:"_L1)) {
        QString kanjiStr = urlStr.mid(6);
        if (!kanjiStr.isEmpty()) {
            Q_EMIT kanjiClicked(kanjiStr.at(0));
        }
    } else if (urlStr.startsWith("word:"_L1)) {
        // Format: word:WORD:READING
        QString rest = urlStr.mid(5);
        int sep = rest.indexOf(':'_L1);
        if (sep > 0) {
            QString word = rest.left(sep);
            QString reading = rest.mid(sep + 1);
            Q_EMIT wordClicked(word, reading);
        }
    } else if (urlStr.length() == 1 && isCJKCharacter(urlStr.at(0))) {
        Q_EMIT kanjiClicked(urlStr.at(0));
    }
}

QString KanjiPage::generateCSS() const
{
    KColorScheme scheme(QPalette::Active);
    QFont font = KitenConfigSkeleton::self()->font();

    return QStringLiteral(
               "body { background-color: %1; color: %2; font-family: \"%3\"; font-size: %4pt; }"
               "a { text-decoration: none; color: %5; }"
               "a:hover { color: %6; }"
               ".kanji-header { margin: 10px 0; }"
               ".kanji-char { font-size: %7pt; vertical-align: middle; }"
               ".kanji-meta { font-size: %4pt; color: %8; margin-left: 16px; display: inline-block; vertical-align: middle; }"
               ".section { margin: 8px 0; }"
               ".section-title { font-weight: bold; color: %8; border-bottom: 1px solid %9; margin-bottom: 4px; }"
               ".label { margin: 2px 0; }"
               ".reading { }"
               ".meanings { }"
               ".compound { margin: 2px 0; padding: 1px 0; }"
               ".compound-word { font-size: %10pt; }"
               ".compound-reading { color: %8; }"
               ".compound-meaning { }"
               ".common-tag { color: %11; font-size: 8pt; font-weight: bold; }"
               ".more { color: %8; font-style: italic; }")
        .arg(scheme.background(KColorScheme::NormalBackground).color().name())   // %1
        .arg(scheme.foreground().color().name())                                  // %2
        .arg(font.family())                                                       // %3
        .arg(font.pointSize())                                                    // %4
        .arg(scheme.foreground(KColorScheme::LinkText).color().name())            // %5
        .arg(scheme.foreground(KColorScheme::ActiveText).color().name())          // %6
        .arg(font.pointSize() + 20)                                              // %7
        .arg(scheme.foreground(KColorScheme::InactiveText).color().name())        // %8
        .arg(scheme.shade(KColorScheme::MidlightShade).name())                   // %9
        .arg(font.pointSize() + 4)                                               // %10
        .arg(scheme.foreground(KColorScheme::PositiveText).color().name());       // %11
}

bool KanjiPage::isCJKCharacter(const QChar &ch)
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

#include "moc_kanjipage.cpp"
