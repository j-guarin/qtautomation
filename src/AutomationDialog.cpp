#include "AutomationDialog.h"

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QCursor>
#include <QDialogButtonBox>
#include <QEvent>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QShortcut>
#include <QStyle>
#include <QStyleOptionButton>
#include <QTest>
#include <QTimer>
#include <QVBoxLayout>

namespace {
constexpr int kHoldMs       = 1000; // keep a radio/check selected for 1 s
constexpr int kGapMs        = 300;  // pause between widgets
constexpr int kKeyMs        = 500;  // 0.5 s between keystrokes / backspaces
constexpr int kCursorPollMs = 30;   // hardware-cursor polling in CONTROL
constexpr int kTypeCount    = 15;
constexpr char kSample[]    = "HelloFromQTest!";
static_assert(sizeof(kSample) - 1 == kTypeCount, "sample text must be 15 chars");

// Point on the button's indicator so the synthetic click always hits it,
// even if the layout stretches the widget wider than its label.
QPoint indicatorPoint(QAbstractButton *b)
{
    QStyleOptionButton opt;
    opt.initFrom(b);
    opt.text = b->text();
    const auto se = qobject_cast<QRadioButton *>(b) ? QStyle::SE_RadioButtonIndicator
                                                    : QStyle::SE_CheckBoxIndicator;
    const QRect r = b->style()->subElementRect(se, &opt, b);
    return r.isValid() ? r.center() : QPoint(8, b->height() / 2);
}
} // namespace

AutomationDialog::AutomationDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Modal - YIELD / CONTROL"));
    setModal(true);
    buildUi();

    // Qt::CTRL maps to Cmd on macOS automatically.
    auto *takeControl = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_M), this);
    takeControl->setContext(Qt::WindowShortcut);
    connect(takeControl, &QShortcut::activated, this, [this] {
        if (m_state == State::Yield)
            setState(State::Control);
    });

    m_stepTimer = new QTimer(this);
    m_stepTimer->setSingleShot(true);
    connect(m_stepTimer, &QTimer::timeout, this, &AutomationDialog::runStep);

    m_cursorTimer = new QTimer(this);
    m_cursorTimer->setInterval(kCursorPollMs);
    connect(m_cursorTimer, &QTimer::timeout, this, &AutomationDialog::pollCursor);

    buildSteps();

    // App-wide filter: sees mouse moves for every window/widget.
    qApp->installEventFilter(this);
}

AutomationDialog::~AutomationDialog()
{
    qApp->removeEventFilter(this);
}

void AutomationDialog::buildUi()
{
    m_stateLabel = new QLabel(this);
    QFont f = m_stateLabel->font();
    f.setBold(true);
    f.setPointSizeF(f.pointSizeF() * 1.3);
    m_stateLabel->setFont(f);

    auto *hint = new QLabel(tr("Ctrl/Cmd+Shift+M: take CONTROL. "
                               "Move the mouse or refocus this window: back to YIELD."), this);
    hint->setWordWrap(true);

    auto *radioBox = new QGroupBox(tr("Radio buttons"), this);
    auto *radioLayout = new QVBoxLayout(radioBox);
    m_radioGroup = new QButtonGroup(this);
    m_radioGroup->setExclusive(true);
    for (const QString &t : {tr("Alpha"), tr("Beta"), tr("Gamma")}) {
        auto *rb = new QRadioButton(t, radioBox);
        radioLayout->addWidget(rb);
        m_radioGroup->addButton(rb);
        m_radios << rb;
    }

    auto *checkBox = new QGroupBox(tr("Check boxes"), this);
    auto *checkLayout = new QVBoxLayout(checkBox);
    for (const QString &t : {tr("Option one"), tr("Option two"), tr("Option three")}) {
        auto *cb = new QCheckBox(t, checkBox);
        checkLayout->addWidget(cb);
        m_checks << cb;
    }

    auto *form = new QFormLayout;
    for (const QString &t : {tr("Name:"), tr("Comment:")}) {
        auto *le = new QLineEdit(this);
        form->addRow(t, le);
        m_edits << le;
    }

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_stateLabel);
    layout->addWidget(hint);
    layout->addWidget(radioBox);
    layout->addWidget(checkBox);
    layout->addLayout(form);
    layout->addWidget(buttons);
    resize(420, sizeHint().height());
}

void AutomationDialog::buildSteps()
{
    // Radio buttons: select, hold 1 s, unselect.
    for (QRadioButton *rb : m_radios) {
        m_steps.push_back({[rb] {
            rb->setFocus(Qt::OtherFocusReason);
            QTest::mouseClick(rb, Qt::LeftButton, {}, indicatorPoint(rb));
            return true;
        }, kHoldMs});
        // An exclusive group refuses to uncheck its checked button,
        // so relax exclusivity just for the unselect click.
        m_steps.push_back({[this, rb] {
            m_radioGroup->setExclusive(false);
            QTest::mouseClick(rb, Qt::LeftButton, {}, indicatorPoint(rb));
            m_radioGroup->setExclusive(true);
            return true;
        }, kGapMs});
    }

    // Check boxes: check, hold 1 s, uncheck.
    for (QCheckBox *cb : m_checks) {
        for (int delay : {kHoldMs, kGapMs}) {
            m_steps.push_back({[cb] {
                cb->setFocus(Qt::OtherFocusReason);
                QTest::mouseClick(cb, Qt::LeftButton, {}, indicatorPoint(cb));
                return true;
            }, delay});
        }
    }

    // Text fields: type 15 chars (0.5 s apart), then backspace until empty.
    for (QLineEdit *le : m_edits) {
        m_steps.push_back({[le] {
            QTest::mouseClick(le, Qt::LeftButton);
            le->setFocus(Qt::OtherFocusReason);
            le->end(false);
            return true;
        }, kKeyMs});

        for (int i = 0; i < kTypeCount; ++i) {
            const char c = kSample[i];
            m_steps.push_back({[le, c] {
                QTest::keyClick(le, c);
                return true;
            }, kKeyMs});
        }

        m_steps.push_back({[le] {
            if (le->text().isEmpty())
                return true;
            le->end(false); // user may have moved the caret while in CONTROL
            QTest::keyClick(le, Qt::Key_Backspace);
            return le->text().isEmpty(); // repeat this step until empty
        }, kKeyMs});
    }
}

void AutomationDialog::setState(State s)
{
    m_state = s;

    if (s == State::Yield) {
        m_cursorTimer->stop();
        m_stateLabel->setText(tr("State: YIELD (automation running)"));
        m_stepTimer->start(kGapMs);           // resume where we left off
    } else {
        m_stepTimer->stop();
        m_lastCursor = QCursor::pos();
        m_cursorTimer->start();
        m_stateLabel->setText(tr("State: CONTROL (manual)"));
    }
    emit stateChanged(s);
}

void AutomationDialog::runStep()
{
    if (!m_running || m_state != State::Yield || m_steps.isEmpty())
        return;

    const Step &step = m_steps[m_index];
    const int delay = step.delayMs;
    const bool finished = step.action();

    if (!m_running || m_state != State::Yield)
        return;
    if (finished)
        m_index = (m_index + 1) % m_steps.size(); // loop forever while in YIELD
    m_stepTimer->start(delay);
}

void AutomationDialog::pollCursor()
{
    // QTest never moves the OS cursor, so any change here is the real mouse
    // (also catches movement outside the dialog).
    if (m_state == State::Control && QCursor::pos() != m_lastCursor)
        setState(State::Yield);
}

bool AutomationDialog::eventFilter(QObject *watched, QEvent *event)
{
    // Automation only runs in YIELD, so in CONTROL every event below is real.
    if (m_running && m_state == State::Control) {
        if (event->type() == QEvent::MouseMove)
            setState(State::Yield);
        else if (watched == this && event->type() == QEvent::WindowActivate)
            setState(State::Yield); // regained focus
    }
    return QDialog::eventFilter(watched, event);
}

void AutomationDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    m_running = true;
    m_index = 0;
    setState(State::Yield); // initial state
}

void AutomationDialog::done(int result)
{
    m_running = false;
    m_stepTimer->stop();
    m_cursorTimer->stop();
    QDialog::done(result);
}
