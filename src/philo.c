//$file${.::philo.c} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: dpp.qm
// File:  ${.::philo.c}
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
//$endhead${.::philo.c} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "qpc.h"               // QP/C real-time embedded framework
#include "dpp.h"               // DPP Application interface
#include "bsp.h"               // Board Support Package

//$declare${AOs::Philo} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::Philo} ..............................................................
typedef struct Philo {
// protected:
    QActive super;

// private:
    QTimeEvt timeEvt;
    uint8_t id;

// public:
} Philo;

extern Philo Philo_inst[N_PHILO];

// protected:
static QState Philo_initial(Philo * const me, void const * const par);
static QState Philo_thinking(Philo * const me, QEvt const * const e);
static QState Philo_hungry(Philo * const me, QEvt const * const e);
static QState Philo_eating(Philo * const me, QEvt const * const e);
//$enddecl${AOs::Philo} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//----------------------------------------------------------------------------
Q_DEFINE_THIS_FILE

// helper function to provide a randomized think time for Philos
static QTimeEvtCtr think_time(void); // prototype
static inline QTimeEvtCtr think_time(void) {
    return (QTimeEvtCtr)((BSP_random() % BSP_TICKS_PER_SEC)
                                        + (BSP_TICKS_PER_SEC/2U));
}

// helper function to provide a randomized eat time for Philos
static QTimeEvtCtr eat_time(void); // prototype
static inline QTimeEvtCtr eat_time(void) {
    return (QTimeEvtCtr)((BSP_random() % BSP_TICKS_PER_SEC)
                                        + BSP_TICKS_PER_SEC);
}
//----------------------------------------------------------------------------

//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 730U) || (QP_VERSION != ((QP_RELEASE^4294967295U)%0x2710U))
#error qpc version 7.3.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${Shared::Philo_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${Shared::Philo_ctor} ......................................................
void Philo_ctor(uint_fast8_t const id) {
    Philo * const me = &Philo_inst[id];
    QActive_ctor(&me->super, Q_STATE_CAST(&Philo_initial));
    QTimeEvt_ctorX(&me->timeEvt, &me->super, TIMEOUT_SIG, 0U),
    me->id = (uint8_t)id;
}
//$enddef${Shared::Philo_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${Shared::AO_Philo[N_PHILO]} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${Shared::AO_Philo[N_PHILO]} ...............................................
QActive * const AO_Philo[N_PHILO] = {
    &Philo_inst[0].super,
    &Philo_inst[1].super,
    &Philo_inst[2].super,
    &Philo_inst[3].super,
    &Philo_inst[4].super
};
//$enddef${Shared::AO_Philo[N_PHILO]} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${AOs::Philo} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::Philo} ..............................................................
Philo Philo_inst[N_PHILO];

//${AOs::Philo::SM} ..........................................................
static QState Philo_initial(Philo * const me, void const * const par) {
    //${AOs::Philo::SM::initial}
    Q_UNUSED_PAR(par);

    QS_OBJ_ARR_DICTIONARY(&Philo_inst[me->id], me->id);
    QS_OBJ_ARR_DICTIONARY(&Philo_inst[me->id].timeEvt, me->id);

    QActive_subscribe(&me->super, EAT_SIG);
    QActive_subscribe(&me->super, TEST_SIG);

    QS_FUN_DICTIONARY(&Philo_thinking);
    QS_FUN_DICTIONARY(&Philo_hungry);
    QS_FUN_DICTIONARY(&Philo_eating);

    return Q_TRAN(&Philo_thinking);
}

//${AOs::Philo::SM::thinking} ................................................
static QState Philo_thinking(Philo * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::Philo::SM::thinking}
        case Q_ENTRY_SIG: {
            QTimeEvt_armX(&me->timeEvt, think_time(), 0U);
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::Philo::SM::thinking}
        case Q_EXIT_SIG: {
            (void)QTimeEvt_disarm(&me->timeEvt);
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::Philo::SM::thinking::TIMEOUT}
        case TIMEOUT_SIG: {
            status_ = Q_TRAN(&Philo_hungry);
            break;
        }
        //${AOs::Philo::SM::thinking::EAT, DONE}
        case EAT_SIG: // intentionally fall through
        case DONE_SIG: {
            // EAT or DONE must be for other Philos than this one
            Q_ASSERT(Q_EVT_CAST(TableEvt)->philoId != me->id);
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::Philo::SM::thinking::TEST}
        case TEST_SIG: {
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}

//${AOs::Philo::SM::hungry} ..................................................
static QState Philo_hungry(Philo * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::Philo::SM::hungry}
        case Q_ENTRY_SIG: {
            #ifdef QEVT_DYN_CTOR
            TableEvt const *pe = Q_NEW(TableEvt, HUNGRY_SIG, me->id);
            #else
            TableEvt *pe = Q_NEW(TableEvt, HUNGRY_SIG);
            pe->philoId = me->id;
            #endif
            QACTIVE_POST(AO_Table, &pe->super, &me->super);
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::Philo::SM::hungry::EAT}
        case EAT_SIG: {
            //${AOs::Philo::SM::hungry::EAT::[e->philoId==me->iid]}
            if (Q_EVT_CAST(TableEvt)->philoId == me->id) {
                status_ = Q_TRAN(&Philo_eating);
            }
            else {
                status_ = Q_UNHANDLED();
            }
            break;
        }
        //${AOs::Philo::SM::hungry::DONE}
        case DONE_SIG: {
            // DONE must be for other Philos than this one
            Q_ASSERT(Q_EVT_CAST(TableEvt)->philoId != me->id);
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}

//${AOs::Philo::SM::eating} ..................................................
static QState Philo_eating(Philo * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::Philo::SM::eating}
        case Q_ENTRY_SIG: {
            QTimeEvt_armX(&me->timeEvt, eat_time(), 0U);
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::Philo::SM::eating}
        case Q_EXIT_SIG: {
            (void)QTimeEvt_disarm(&me->timeEvt);

            #ifdef QEVT_DYN_CTOR
            TableEvt const *pe = Q_NEW(TableEvt, DONE_SIG, me->id);
            #else
            TableEvt *pe = Q_NEW(TableEvt, DONE_SIG);
            pe->philoId = me->id;
            #endif
            QACTIVE_PUBLISH(&pe->super, &me->super);
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::Philo::SM::eating::TIMEOUT}
        case TIMEOUT_SIG: {
            status_ = Q_TRAN(&Philo_thinking);
            break;
        }
        //${AOs::Philo::SM::eating::EAT, DONE}
        case EAT_SIG: // intentionally fall through
        case DONE_SIG: {
            // EAT or DONE must be for other Philos than this one
            Q_ASSERT(Q_EVT_CAST(TableEvt)->philoId != me->id);
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}
//$enddef${AOs::Philo} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
