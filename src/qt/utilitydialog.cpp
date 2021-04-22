// Copyright (c) 2011-2017 The Bitcoin Core developers
// Copyright (c) 2020 The Gapcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <qt/utilitydialog.h>

#include <qt/forms/ui_helpmessagedialog.h>

#include <qt/bitcoingui.h>
#include <qt/clientmodel.h>
#include <qt/guiconstants.h>
#include <qt/intro.h>
#include <qt/paymentrequestplus.h>
#include <qt/guiutil.h>

#include <clientversion.h>
#include <init.h>
#include <util.h>

#include <stdio.h>

#include <QCloseEvent>
#include <QLabel>
#include <QRegExp>
#include <QTextTable>
#include <QTextCursor>
#include <QVBoxLayout>

const char *story = R""""(<h3 style="margin-top:8em"><b>BEEN A LONG, LONG TIME — R.A. Lafferty</b> (<i>Fantastic</i> Vol 20, No 2. Dec. 1970)</h3>
<p>IT DOESN’T END with one—it <i>Begins</i> with a whimper.</p>
<p>It was a sundering Dawn—Incandescence to which all later lights are less than candles—Heat to which the heat of all later suns is but a burnt-out match—the Polarities that set up the tension forever.</p>
<p>And in the middle was a whimper, just as was felt the first jerk that indicated that time had begun.</p>
<p>The two Challenges stood taller than the radius of the space that was being born; and one weak creature, Boshel, stood in the middle, too craven to accept either challenge.</p>
<p>“Uh, how long you fellows going to be gone?” Boshel snuffled.</p>
<p>The Creative Event was the Revolt rending the Void in two. The two sides formed, opposing Nations of Lightning split above the steep chasm. Two Champions had it out with a bitterness that has never passed—Michael wrapped in white fire—and Helel swollen with black and purple blaze. And their followers with them. It has been put into allegory as Acceptance and Rejection, and as Good and Evil; but in the Beginning there was the Polarity by which the universes are sustained.</p>
<p>Between them, like a pigmy, stood Boshel alone in whimpering hesitation.</p>
<p>“Get the primordial metal out of it if you’re coming with us,” Helel growled like cracked thunder as he led his followers off in a fury to form a new settlement.</p>
<p>“Uh, you guys going to be back before night?” Boshel whimpered.</p>
<p>“Oh, get to Hell out of here.” Michael roared.</p>
<p>“Keep the little oaf!” Helel snorted. “He hasn’t enough brimstone in him to set fire to an outhouse.”</p>
<p>The two great hosts separated, and Boshel was left alone in the void. He was still standing there when there was a second little jerk and time began in earnest, bursting the pod into a shower of sparks that traveled and grew. He was still standing there when the sparks acquired form and spin; and he stood there yet when life began to appear on the soot specks thrown off from the sparks. He stood there quite a long, long time.</p>
<p>“What are we going to do with the little bugger?” an underling asked Michael. “We can’t have him fouling up the landscape forever.”</p>
<p>“I’ll go ask,” said Mike, and he did.</p>
<p>But Michael was told that the responsibility was his; that Boshel would have to be punished for his hesitancy; and that it was up to Michael to select the suitable punishment and see that it was carried out.</p>
<p>“You know, he made time itself stutter at the start,” Mike told the underling. “He set up a random that affected everything. It’s got to be a punishment with something to do with time.”</p>
<p>“You got any ideas?” the underling asked.</p>
<p>“I’ll think of something,” Michael said. Quite a while after this, Michael was thumbing through a book one afternoon at a news-stand in Los Angeles.</p>
<p>“It says here,” Michael intoned, “that if six monkeys were set down to six typewriters and typed for a long enough time, they would type all the words of Shakespeare exactly. Time is something we’ve got plenty of. Let’s try it, Kitabel, and see how long it takes.”</p>
<p>“What’s a monkey, Michael?”</p>
<p>“I don’t know.”</p>
<p>“What’s a typewriter?”</p>
<p>“I don’t know.”</p>
<p>“What’s a Shakespeare, Mike?”</p>
<p>“Anybody can ask questions, Kitabel. Get the things together and let’s get the project started.”</p>
<p>“It sounds like a lengthy project. Who will oversee it?”</p>
<p>“Boshel. It’s a natural for him. It will teach him patience and a sense of order, and impress on him the majesty of time. It’s exactly the punishment I’ve been looking for.”</p>
<p>They got the things together and turned them over to Boshel.</p>
<p>“As soon as the project is finished, Bosh, your period of waiting will be over. Then you can join the group and enjoy yourself with the rest of us.”</p>
<p>“Well, it’s better than standing here doing nothing,” Boshel said. “It’d go faster if I could educate the monkeys and let them copy it.”</p>
<p>“No, the typing has to be random, Bosh. It was you who introduced the random factor into the universe. So, suffer for it.”</p>
<p>“Any particular edition the copy has to correspond to?”</p>
<p>“The ‘Blackstone Readers’ Edition Thirty-Seven and a half Volumes in One that I have here in my hand will do fine,” Michael said. “I’ve had a talk with the monkeys, and they’re willing to stick with it. It took me eighty thousand years just to get them where they could talk, but that’s nothing when we’re talking about time.”</p>
<p>“Man, are we ever talking about Time!” Boshel moaned.</p>
<p>“I made a deal with the monkeys. They will be immune to fatigue and boredom. I cannot promise the same for you.”</p>
<p>“Uh, Michael, since it may be quite a while, I wonder if I could have some sort of clock to keep track of how fast things are going.”</p>
<p>So Michael made him a clock. It was a cube of dressed stone measuring a parsec on each edge.</p>
<p>“You don’t have to wind it, you don’t have to do a thing to it. Bosh,” Michael explained. “A small bird will come every millennium and sharpen its beak on this stone. You can tell the passing of time by the diminishing of the stone. It’s a good clock, and it has only one moving part, the bird. I will not guarantee that your project will be finished by the time the entire stone is worn away, but you will be able to tell that time has passed.”</p>
<p>“It’s better than nothing,” Boshel said, “but it’s going to be a drag. I think this concept of time is a little Mediaeval, though.”</p>
<p>“So am I,” Michael said. “I tell you what I can do, though, Bosh. I can chain you to that stone and have another large bird dive-bomb you and gouge out hunks of your liver. That was in a story in another book on that news-stand.”</p>
<p>“You slay me, Mike. That won’t be necessary. I’ll pass the time somehow.”</p>
<p>Boshel set the monkeys to work. They were conditioned to punch the typewriters at random. Within a short period of time (as the Larger Creatures count time) the monkeys had produced whole Shakespearian words: ‘Let’ which is found in scene two of act one of Richard ID; ‘Go’ which is in scene two of act two of Julius Caesar; and ‘Be’ which occurs in the very first scene and act of the Tempest. Boshel was greatly encouraged.</p>
<p>Some time after this, one of the monkeys produced two Shakespearian words in succession. By this time, the home world of Shakespeare (which was also the home world of the news-stand in Los Angeles where was bom a great idea) was long out of business.</p>
<p>After another while, the monkeys had done whole phrases. By then, quite a bit of time had run out.</p>
<p>The trouble with that little bird is that its beak did not seem to need much sharpening when it did come once every thousand years. Boshel discovered that Michael had played a dirty seraphic trick on him and had been feeding the bird entirely on bland custard. The bird would take two or three light swipes at the stone, and then be off for another thousand years. Yet, after no more than a thousand visitations, there was an unmistakable scratch on the stone. It was a hopeful sign.</p>
<p>Boshel began to see that the thing could be done. A monkey—and not the most brilliant of them—finally produced a whole sentence: “What say’st thou bully-rook?” And at that very moment another thing happened. It was surprising to Boshel, for it was the first time he had ever seen it. But he would see it milliards of times before it was finished.</p>
<p>A speck of cosmic dust, on the far out reaches of space, met another speck. This should not have been unusual; specks were always meeting specks. But this case was different. Each speck—in the opposite direction— had been the outmost in the whole cosmos. You can’t get farther apart than that. The speck (a teeming conglomerate of peopled worlds) looked at the other speck with eyes and instruments, and saw its own eyes and instruments looking back at it. What the speck saw was itself. The cosmic tetra-dimensionate sphere had been completed. The first speck had met itself coming from the other direction, and space had been transversed.</p>
<p>Then it all collapsed.</p>
<p>The stars went out, one by one, and billion by billion. Nightmares of falling! All the darkened orbs and oblates fell down into the void that was all bottom. There was nothing left but one tight pod in the void, and a few out of context things like Michael and his associates, and Boshel and his monkeys.</p>
<p>Boshel had a moment of unease: he had become used to the appearance of the expanding universe. But he need not have been uneasy. It began all over again.</p>
<p>A few billion centuries ticked by silently. Once more, the pod burst into a shower of sparks that traveled and grew. They acquired form and spin, and life appeared again on the soot specks thrown off from those sparks.</p>
<p>This happened again and again. Each cycle seemed damnably long while it was happening; but in retrospect, the cycles were only like a light blinking on and off.</p>
<p>And in the Longer Retrospect, they were like a high-frequency alternator, producing a dizzy number of such cycles every over-second, and continuing for tumbling ages. Yet Boshel was becoming bored. There was just no other word for it.</p>
<p>When only a few billion cosmic cycles had been completed, there was a gash in the clock-rock that you could hide a horse in. The little bird made very many journeys back to sharpen its beak. And Pithekos Pete, the most rapid of the monkeys, had now random-written the Tempest, complete and perfect. They shook hands all around, monkeys and angel. It was something of a moment.</p>
<p>The moment did not last. Pete, instead of pecking at furious random to produce the rest of the plays, wrote his own improved version of the Tempest. Boshel was furious.</p>
<p>“But it’s better, Bosh,” Pete protested. “And I have some ideas about stage-craft that will really set this thing up.”</p>
<p>“Of course it’s better! We don’t want them better. We want them just the same. Can’t you monkeys realize that we are working out a problem of random probabilities? Oh, you clunker-heads!” “Let me have that damned book for a month, Bosh, and I’ll copy the plagued things off and we’ll be finished,” Pithekos Pete suggested.</p>
<p>“Rules, you lunk-heads, rules!” Boshel grated out. “We have to abide by the rules. You know that isn’t allowed, and besides it would be found out. I have reason to suspect, and it cuts me to say this, that one of my own monkeys and associates here present is an informer. We’d never get by with it.”</p>
<p>After the brief misunderstanding, things went better. The monkeys stayed with their task. And after a number of cycles expressed by nine followed by zeros in pica type sufficient to stretch around the universe at a period just prior to its collapse (the radius and the circumference of the ultimate sphere are, of course, the same), the first complete version was ready.</p>
<p>It was faulty, of course, and it had to be rejected. But there were less than thirty thousand errors in it; it presaged great things to come, and ultimate triumph.</p>
<p>Later (People, was it ever later!) they had it quite close. By the time that the gash in the clock-rock would hold a medium-sized solar system, they had a version with only five errors.</p>
<p>“It will come,” Boshel said. “It will come in time. And time is the one thing we have plenty of.”</p>
<p>Later—much, much later—they seemed to have it perfect; and by this time, the bird had worn away nearly a fifth of the bulk of the great stone with its millennial visits.</p>
<p>Michael himself read the version and could find no error. This was not conclusive, of course, for Michael was an impatient and hurried reader. Three readings were required for verification, but never was hope so high.</p>
<p>It passed the second reading, by a much more careful angel, and was pronounced letter-perfect. But it was later at night when that reader had finished it, and he may have gotten a little careless at the end.</p>
<p>And it passed the third reading, through all the thirty-seven plays of it, and into the poems at the end. This was Kitabel, the scribing angel himself, who was appointed to that third reading. He was just about to sign the certification when he paused.</p>
<p>“There is something sticking in my mind,” he said, and he shook his head to clear it. “There is something like an echo that is not quite right. I wouldn’t want to make a mistake.”</p>
<p>He had written ‘Kitab—’, but he had not finished his signature.</p>
<p>“I won’t be able to sleep tonight if I don’t think of it,” he complained. “It wasn’t in the plays; I know that they were perfect. It was something in the poems—quite near the end—some dissonance. Either the bard wrote a remarkably malapropos line, or there was an error in the transcription that my eye overlooked but my ear remembered. I acknowledge that I was sleepy near the end.”</p>
<p>“Oh, by all the worlds that were ever made, sign!” pleaded Boshel.</p>
<p>“You have waited this long, a moment more won’t kill you, Bosh.”</p>
<p>“Don’t bet on it, Kit. I’m about to blow, I tell you.”</p>
<p>But Kitabel went back and he found it—a verse in the Phoenix and the Turtle:</p>
<p><pre>
    From this session interdict
    Every fowl of tyrant wing,
    Save the eagle, feather’d king:
    Keep the obsequy so strict.
</pre></p>
<p>That is what the book itself said. And what Pithekos Pete had written was nearly, but not quite, the same thing:</p>
<p><pre>
    From this session interdict
    Every fowl of tyrant wingg,
    Save the eaggle, feather’d kingg:
    Dam machine the g is sticked.
</pre></p>
<p>And if you never saw an angel cry, words cannot describe to you the show that Boshel put on then.</p>
<p>They are still at it tonight, typing away at random, for that last sad near-victory was less than a million billion cycles ago. And only a moment ago—half way back in the present cycle—one of the monkeys put together no less than nine Shakespearian words in a row.</p>
<p>There is still hope. And the bird has now worn the rock down to about half its bulk.</p>
<p><a href="https://archive.org/stream/Fantastic_v20n02_1970-12/"><i>https://archive.org/stream/Fantastic_v20n02_1970-12/</i></a></p>)"""";

/** "Help message" or "About" dialog box */
HelpMessageDialog::HelpMessageDialog(QWidget *parent, bool about) :
    QDialog(parent),
    ui(new Ui::HelpMessageDialog)
{
    ui->setupUi(this);

    QString version = tr(PACKAGE_NAME) + " " + tr("version") + " " + QString::fromStdString(FormatFullVersion());
    /* On x86 add a bit specifier to the version so that users can distinguish between
     * 32 and 64 bit builds. On other architectures, 32/64 bit may be more ambiguous.
     */
#if defined(__x86_64__)
    version += " " + tr("(%1-bit)").arg(64);
#elif defined(__i386__ )
    version += " " + tr("(%1-bit)").arg(32);
#endif

    if (about)
    {
        setWindowTitle(tr("About %1").arg(tr(PACKAGE_NAME)));

        /// HTML-format the license message from the core
        QString licenseInfo = QString::fromStdString(LicenseInfo());
        QString licenseInfoHTML = licenseInfo;
        // Make URLs clickable
        QRegExp uri("<(.*)>", Qt::CaseSensitive, QRegExp::RegExp2);
        uri.setMinimal(true); // use non-greedy matching
        licenseInfoHTML.replace(uri, "<a href=\"\\1\">\\1</a>");
        // Replace newlines with HTML breaks
        licenseInfoHTML.replace("\n", "<br>");

        ui->aboutMessage->setTextFormat(Qt::RichText);
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        text = version + "\n" + licenseInfo;
        ui->aboutMessage->setText(version + "<br><br>" + licenseInfoHTML + story);
        ui->aboutMessage->setWordWrap(true);
        ui->helpMessage->setVisible(false);
    } else {
        setWindowTitle(tr("Command-line options"));
        QString header = tr("Usage:") + "\n" +
            "  gapcoin-qt [" + tr("command-line options") + "]                     " + "\n";
        QTextCursor cursor(ui->helpMessage->document());
        cursor.insertText(version);
        cursor.insertBlock();
        cursor.insertText(header);
        cursor.insertBlock();

        std::string strUsage = HelpMessage(HMM_BITCOIN_QT);
        const bool showDebug = gArgs.GetBoolArg("-help-debug", false);
        strUsage += HelpMessageGroup(tr("UI Options:").toStdString());
        if (showDebug) {
            strUsage += HelpMessageOpt("-allowselfsignedrootcertificates", strprintf("Allow self signed root certificates (default: %u)", DEFAULT_SELFSIGNED_ROOTCERTS));
        }
        strUsage += HelpMessageOpt("-chart", strprintf(tr("Render difficulty and hashrate graphs (default: %u)").toStdString(), DEFAULT_CHARTPLOTTING));
        strUsage += HelpMessageOpt("-choosedatadir", strprintf(tr("Choose data directory on startup (default: %u)").toStdString(), DEFAULT_CHOOSE_DATADIR));
        strUsage += HelpMessageOpt("-lang=<lang>", tr("Set language, for example \"de_DE\" (default: system locale)").toStdString());
        strUsage += HelpMessageOpt("-min", tr("Start minimized").toStdString());
        strUsage += HelpMessageOpt("-rootcertificates=<file>", tr("Set SSL root certificates for payment request (default: -system-)").toStdString());
        strUsage += HelpMessageOpt("-splash", strprintf(tr("Show splash screen on startup (default: %u)").toStdString(), DEFAULT_SPLASHSCREEN));
        strUsage += HelpMessageOpt("-resetguisettings", tr("Reset all settings changed in the GUI").toStdString());
        if (showDebug) {
            strUsage += HelpMessageOpt("-uiplatform", strprintf("Select platform to customize UI for (one of windows, macosx, other; default: %s)", BitcoinGUI::DEFAULT_UIPLATFORM));
        }
        QString coreOptions = QString::fromStdString(strUsage);
        text = version + "\n" + header + "\n" + coreOptions;

        QTextTableFormat tf;
        tf.setBorderStyle(QTextFrameFormat::BorderStyle_None);
        tf.setCellPadding(2);
        QVector<QTextLength> widths;
        widths << QTextLength(QTextLength::PercentageLength, 35);
        widths << QTextLength(QTextLength::PercentageLength, 65);
        tf.setColumnWidthConstraints(widths);

        QTextCharFormat bold;
        bold.setFontWeight(QFont::Bold);

        for (const QString &line : coreOptions.split("\n")) {
            if (line.startsWith("  -"))
            {
                cursor.currentTable()->appendRows(1);
                cursor.movePosition(QTextCursor::PreviousCell);
                cursor.movePosition(QTextCursor::NextRow);
                cursor.insertText(line.trimmed());
                cursor.movePosition(QTextCursor::NextCell);
            } else if (line.startsWith("   ")) {
                cursor.insertText(line.trimmed()+' ');
            } else if (line.size() > 0) {
                //Title of a group
                if (cursor.currentTable())
                    cursor.currentTable()->appendRows(1);
                cursor.movePosition(QTextCursor::Down);
                cursor.insertText(line.trimmed(), bold);
                cursor.insertTable(1, 2, tf);
            }
        }

        ui->helpMessage->moveCursor(QTextCursor::Start);
        ui->scrollArea->setVisible(false);
        ui->aboutLogo->setVisible(false);
    }
}

HelpMessageDialog::~HelpMessageDialog()
{
    delete ui;
}

void HelpMessageDialog::printToConsole()
{
    // On other operating systems, the expected action is to print the message to the console.
    fprintf(stdout, "%s\n", qPrintable(text));
}

void HelpMessageDialog::showOrPrint()
{
#if defined(WIN32)
    // On Windows, show a message box, as there is no stderr/stdout in windowed applications
    exec();
#else
    // On other operating systems, print help text to console
    printToConsole();
#endif
}

void HelpMessageDialog::on_okButton_accepted()
{
    close();
}


/** "Shutdown" window */
ShutdownWindow::ShutdownWindow(QWidget *parent, Qt::WindowFlags f):
    QWidget(parent, f)
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(new QLabel(
        tr("%1 is shutting down...").arg(tr(PACKAGE_NAME)) + "<br /><br />" +
        tr("Do not shut down the computer until this window disappears.")));
    setLayout(layout);
}

QWidget *ShutdownWindow::showShutdownWindow(BitcoinGUI *window)
{
    if (!window)
        return nullptr;

    // Show a simple window indicating shutdown status
    QWidget *shutdownWindow = new ShutdownWindow();
    shutdownWindow->setWindowTitle(window->windowTitle());

    // Center shutdown window at where main window was
    const QPoint global = window->mapToGlobal(window->rect().center());
    shutdownWindow->move(global.x() - shutdownWindow->width() / 2, global.y() - shutdownWindow->height() / 2);
    shutdownWindow->show();
    return shutdownWindow;
}

void ShutdownWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
}
