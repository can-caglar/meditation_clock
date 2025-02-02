//$file${.::dpp.h} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: dpp.qm
// File:  ${.::dpp.h}
//
// This code has been generated by QM 6.2.3 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
//                 ____________________________________
//                /                                   /
//               /    GGGGGGG    PPPPPPPP   LL       /
//              /   GG     GG   PP     PP  LL       /
//             /   GG          PP     PP  LL       /
//            /   GG   GGGGG  PPPPPPPP   LL       /
//           /   GG      GG  PP         LL       /
//          /     GGGGGGG   PP         LLLLLLL  /
//         /___________________________________/
//
// Copyright (c) 2005 Quantum Leaps, LLC
// SPDX-License-Identifier: GPL-3.0-or-later
//
// This generated code is open-source software licensed under the GNU
// General Public License (GPL) as published by the Free Software Foundation
// (see <https://www.gnu.org/licenses>).
//
// NOTE:
// The GPL does NOT permit the incorporation of this code into proprietary
// programs. Please contact Quantum Leaps for commercial licensing options,
// which expressly supersede the GPL and are designed explicitly for licensees
// interested in retaining the proprietary status of the generated code.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${.::dpp.h} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#ifndef DPP_H_
#define DPP_H_

//$declare${Shared} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${Shared::AppSignals} ......................................................
enum AppSignals {
    EAT_SIG = Q_USER_SIG, // published by Table to let a Philo eat
    DONE_SIG,       // published by Philo when done eating
    PAUSE_SIG,      // published by BSP to pause the application
    SERVE_SIG,      // published by BSP to serve re-start serving forks
    TEST_SIG,       // published by BSP to test the application
    MAX_PUB_SIG,    // the last published signal

    NEW_TIME_SIG,
    START_MEDITATION_SIG,
    MORNING_MEDITATION_START_SIG,
    MORNING_MEDITATION_END_SIG,
    MINUTE_QUARTER_SIG,
    ALARM_SIG,
    TIMEOUT_SIG,    // posted by time event to Philo
    HUNGRY_SIG,     // posted by hungry Philo to Table
    MAX_SIG         // the last signal
};

//${Shared::produce_sig_dict} ................................................
#ifdef Q_SPY
static inline void produce_sig_dict(void) {
    QS_SIG_DICTIONARY(EAT_SIG,     (void *)0);
    QS_SIG_DICTIONARY(DONE_SIG,    (void *)0);
    QS_SIG_DICTIONARY(PAUSE_SIG,   (void *)0);
    QS_SIG_DICTIONARY(SERVE_SIG,   (void *)0);
    QS_SIG_DICTIONARY(TEST_SIG,    (void *)0);

    QS_SIG_DICTIONARY(START_MEDITATION_SIG, (void *)0);
    QS_SIG_DICTIONARY(MORNING_MEDITATION_START_SIG, (void *)0);
    QS_SIG_DICTIONARY(MORNING_MEDITATION_END_SIG, (void *)0);
    QS_SIG_DICTIONARY(MINUTE_QUARTER_SIG, (void *)0);
    QS_SIG_DICTIONARY(TIMEOUT_SIG, (void *)0);
    QS_SIG_DICTIONARY(HUNGRY_SIG,  (void *)0);
    QS_SIG_DICTIONARY(NEW_TIME_SIG,  (void *)0);
}
#endif // def Q_SPY

//${Shared::N_PHILO} .........................................................
#define N_PHILO ((uint8_t)5U)

//${Shared::TableEvt} ........................................................
typedef struct {
// protected:
    QEvt super;

// public:
    uint8_t philoId;
} TableEvt;

// public:

#ifdef QEVT_DYN_CTOR
static inline TableEvt * TableEvt_ctor(TableEvt * const me,
    uint8_t id)
{
    if (me != (TableEvt *)0) {
        // don't call QEvt_ctor() because the initialization of all
        // QEvt attributes is already done in QF_QF_newX_()
        me->philoId = id;
    }
    return me;
}
#endif // def QEVT_DYN_CTOR

//${Shared::AO_Philo[N_PHILO]} ...............................................
extern QActive * const AO_Philo[N_PHILO];

//${Shared::Philo_ctor} ......................................................
void Philo_ctor(uint_fast8_t const id);

//${Shared::AO_Table} ........................................................
extern QActive * const AO_Table;

//${Shared::Table_ctor} ......................................................
void Table_ctor(void);

//${Shared::Meditation_ctor} .................................................
void Meditation_ctor(void);

//${Shared::AO_Meditation} ...................................................
extern QActive * const AO_Meditation;

//${Shared::NewTimeEvt} ......................................................
typedef struct {
// protected:
    QEvt super;

// public:
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} NewTimeEvt;

//${Shared::QuarterEvt} ......................................................
typedef struct {
// protected:
    QEvt super;

// public:
    uint8_t quarter;
} QuarterEvt;
//$enddecl${Shared} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#ifdef QXK_H_

extern QXThread * const TH_XThread1;
void XThread1_ctor(void);

extern QXThread * const TH_XThread2;
void XThread2_ctor(void);

extern QXSemaphore TH_sema;
extern QXMutex TH_mutex;

#endif // QXK_H_

#endif // DPP_H_
