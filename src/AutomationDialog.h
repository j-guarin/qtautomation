#pragma once

#include <QDialog>
#include <QList>
#include <QPoint>
#include <functional>

class QButtonGroup;
class QCheckBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QTimer;
class QWidget;

// Modal dialog that runs a two-state machine:
//   YIELD   (initial) : QTest drives the widgets automatically.
//   CONTROL           : automation paused, the user works normally.
//
//   YIELD   -> CONTROL : Ctrl+Shift+M  (Cmd+Shift+M on macOS)
//   CONTROL -> YIELD   : real mouse movement, or the dialog regaining focus.
class AutomationDialog : public QDialog
{
    Q_OBJECT
public:
    enum class State { Yield, Control };
    Q_ENUM(State)

    explicit AutomationDialog(QWidget *parent = nullptr);
    ~AutomationDialog() override;

    State state() const { return m_state; }

signals:
    void stateChanged(AutomationDialog::State state);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void done(int result) override;

private:
    // One automation step. action() returns true when the step is finished
    // (advance to the next one) or false to repeat it after delayMs.
    struct Step {
        std::function<bool()> action;
        int delayMs;
    };

    void buildUi();
    void buildSteps();
    void setState(State s);
    void runStep();
    void pollCursor();
    bool glideCursorTo(QWidget *w, const QPoint &pos);

    State m_state = State::Control;
    bool m_running = false;

    QLabel *m_stateLabel = nullptr;
    QButtonGroup *m_radioGroup = nullptr;
    QList<QRadioButton *> m_radios;
    QList<QCheckBox *> m_checks;
    QList<QLineEdit *> m_edits;

    QList<Step> m_steps;
    int m_index = 0;

    QTimer *m_stepTimer = nullptr;
    QTimer *m_cursorTimer = nullptr;
    QPoint m_lastCursor;
};
