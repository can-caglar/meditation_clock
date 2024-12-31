//============================================================================
// Product: "Dining Philosophers Problem" example, Zephyr RTOS kernel
// Last updated for: @ref qpc_7_3_2
// Last updated on  2023-12-13
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. <state-machine.com>
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <www.gnu.org/licenses/>.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpc.h"                 // QP/C real-time embedded framework
#include "dpp.h"                 // DPP Application interface
#include "bsp.h"                 // Board Support Package

#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/reboot.h>
// add other drivers if necessary...
#include <zephyr/drivers/rtc.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include <zephyr/drivers/pwm.h>

// The devicetree node identifier for the "led0" alias.
#define LED0_NODE DT_ALIAS(led0)
#define LED_EXTERNAL_NODE DT_NODELABEL(led_external)

Q_DEFINE_THIS_FILE

// Local-scope objects -----------------------------------------------------
static struct gpio_dt_spec const l_led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static struct gpio_dt_spec const l_ledExternal = GPIO_DT_SPEC_GET(LED_EXTERNAL_NODE, gpios);
static const struct pwm_dt_spec piezo_pwm = PWM_DT_SPEC_GET(DT_NODELABEL(buzzer));

static struct k_timer zephyr_tick_timer;
static uint32_t l_rnd; // random seed

static const struct device* rtc_pcf8563 = DEVICE_DT_GET(DT_NODELABEL(pcf8563t));

/* Custom Service Variables */
#define BT_UUID_CUSTOM_SERVICE_VAL \
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_CTS_VAL)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_CUSTOM_SERVICE_VAL),
};

static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

/* Custom Service Variables */
#define BT_UUID_CUSTOM_SERVICE_VAL \
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

static const struct bt_uuid_128 meditation_uuid = BT_UUID_INIT_128(BT_UUID_CUSTOM_SERVICE_VAL);

static const struct bt_uuid_128 meditation_start = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef1));

static ssize_t write_meditation_start(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags){

    static QEvt evt = QEVT_INITIALIZER(START_MEDITATION_SIG);
    QACTIVE_POST(AO_Meditation, &evt, NULL);
}

BT_GATT_SERVICE_DEFINE(meditation_service,
	BT_GATT_PRIMARY_SERVICE(&meditation_uuid),
	BT_GATT_CHARACTERISTIC(&meditation_start.uuid,
			       BT_GATT_CHRC_WRITE,
				   BT_GATT_PERM_WRITE,
			       NULL, &write_meditation_start, NULL));

// Helper functions
struct Notes
{
	uint32_t freq;
	uint32_t duration_ms;
};

#define sixteenth ((whole) / 16)
#define eigth ((whole) / 8)
#define quarter ((whole) / 4)
#define half ((whole) / 2)
#define three_quarters (half + quarter)
#define whole 600
#define whole_plus_half (whole + half)

#define PAUSE 0
#define C4 262
#define Db4 277
#define D4 294
#define Eb4 311
#define E4 330
#define F4 349
#define Gb4 370
#define G4 392
#define Ab4 415
#define A4 440
#define Bb4 466
#define B4 494
#define C5 523
#define Db5 554
#define D5 587
#define Eb5 622
#define E5 659
#define F5 698
#define Gb5 740
#define G5 784
#define Ab5 831
#define A5 880
#define Bb5 932
#define B5 988
#define C6 1046
#define Db6 1109
#define D6 1175
#define Eb6 1245
#define E6 1319
#define F6 1397

struct Notes ChurchBells[] = {
    {PAUSE, whole}, {A4, half}, {A4, half}, {A4, whole}, {E4, half}, {D4, whole_plus_half}, // Intro
    {G4, half}, {G4, whole}, {E4, whole}, {E4, quarter}, {F4, quarter}, // Main melody
    {E4, whole}, {E4, half}, {E4, whole}, {C4, whole}, // Variation
    {D4, whole}, {D4, whole}, // Repetition
    {C4, whole}, {E4, whole}, // Resolution back to the main motif
};

struct Notes ASpacemanCameTravelling[] = {
 	{F5, whole}, {G5, half}, {F5, half}, {E5, whole},
 	{D5, half}, {C5, half}, {D5, whole}, {D5, half},
 	{C5, half}, {D5, whole}, {E5, whole},

	{F5, whole}, {G5, half}, {F5, half}, {E5, whole},
 	{D5, half}, {C5, half}, {D5, whole},
};

static void play_tone(uint32_t freq, uint32_t duration_ms);
static void play_song(struct Notes* song, size_t size);

#ifdef Q_SPY

    // QSpy source IDs
    static QSpyId const timerID = { 0U };

    enum AppRecords { // application-specific trace records
        PHILO_STAT = QS_USER,
        PAUSED_STAT,
    };

#endif

//============================================================================
// Error handler

Q_NORETURN Q_onError(char const * const module, int_t const id) {
    // NOTE: this implementation of the assertion handler is intended only
    // for debugging and MUST be changed for deployment of the application
    // (assuming that you ship your production code with assertions enabled).
    Q_UNUSED_PAR(module);
    Q_UNUSED_PAR(id);
    QS_ASSERTION(module, id, 10000U);
    Q_PRINTK("\nERROR in %s:%d\n", module, id);

#ifndef NDEBUG
    k_panic(); // debug build: halt the system for error search...
#else
    sys_reboot(SYS_REBOOT_COLD); // release build: reboot the system
#endif
    for (;;) { // explicitly no-return
    }
}
//............................................................................
void assert_failed(char const * const module, int_t const id); // prototype
void assert_failed(char const * const module, int_t const id) {
    Q_onError(module, id);
}

//............................................................................
static void zephyr_tick_function(struct k_timer *tid); // prototype
static void zephyr_tick_function(struct k_timer *tid) {
    Q_UNUSED_PAR(tid);

    QTIMEEVT_TICK_X(0U, &timerID);
}

//============================================================================

void BSP_init(void) {

#ifdef Q_SPY
    // wait a short while to give time for user to 
    // connect to CDC COM port so QSPY accesses the dictionaries
    k_sleep(K_MSEC(5000));
#endif

    int ret = gpio_pin_configure_dt(&l_led0, GPIO_OUTPUT_ACTIVE);
    Q_ASSERT(ret >= 0);

    ret = gpio_pin_configure_dt(&l_ledExternal, GPIO_OUTPUT_INACTIVE);
    Q_ASSERT(ret >= 0);

    ret = device_is_ready(rtc_pcf8563);
    Q_ASSERT(ret >= 0);

    ret = pwm_is_ready_dt(&piezo_pwm);
    Q_ASSERT(ret >= 0);

    ret = bt_enable(NULL);
    Q_ASSERT(ret >= 0);

	ret = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    Q_ASSERT(ret >= 0);

	QS_BEGIN_ID(PHILO_STAT, 0)
        QS_STR(__func__);
        QS_STR("Advertisement started");
    QS_END()

    k_timer_init(&zephyr_tick_timer, &zephyr_tick_function, NULL);

    BSP_randomSeed(1234U);

    // initialize the QS software tracing...
    if (QS_INIT((void *)0) == 0) {
        Q_ERROR();
    }

    QS_OBJ_DICTIONARY(&timerID);

    QS_USR_DICTIONARY(PHILO_STAT);
    QS_USR_DICTIONARY(PAUSED_STAT);

    QS_ONLY(produce_sig_dict());

    // setup the QS filters...
    QS_GLB_FILTER(QS_ALL_RECORDS); // all records
    QS_GLB_FILTER(-QS_QF_TICK);    // exclude the clock tick
}
//............................................................................
void BSP_start(void) {

    // initialize event pools
    static QF_MPOOL_EL(NewTimeEvt) smlPoolSto[10];
    QF_poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // initialize publish-subscribe
    static QSubscrList subscrSto[MAX_PUB_SIG];
    QActive_psInit(subscrSto, Q_DIM(subscrSto));

    // instantiate and start AOs/threads...

    static QEvt const *meditationQueSto[10];
    static K_THREAD_STACK_DEFINE(meditationStack, 1024);
    Meditation_ctor();
    QACTIVE_START(AO_Meditation,
        1,                          // QP prio. of the AO
        meditationQueSto,           // event queue storage
        Q_DIM(meditationQueSto),    // queue length [events]
        meditationStack, 
        K_THREAD_STACK_SIZEOF(meditationStack),
        (void *)0);                 // no initialization param
}
//............................................................................
void BSP_ledOn(void) {
    QS_BEGIN_ID(PHILO_STAT, 0)
        QS_STR(__func__);
    QS_END()
    gpio_pin_set_dt(&l_ledExternal, true);
}
//............................................................................
void BSP_ledOff(void) {
    QS_BEGIN_ID(PHILO_STAT, 0)
        QS_STR(__func__);
    QS_END()
    gpio_pin_set_dt(&l_ledExternal, false);
}
//............................................................................
void BSP_displayPhilStat(uint8_t n, char const *stat) {
    Q_UNUSED_PAR(n);

    if (stat[0] == 'e') {
        BSP_ledOn();
    }
    else {
        BSP_ledOff();
    }
    Q_PRINTK("Philo[%d]->%s\n", n, stat);

    // app-specific record
    QS_BEGIN_ID(PHILO_STAT, AO_Philo[n]->prio)
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
//............................................................................
void BSP_displayPaused(uint8_t paused) {
    if (paused) {
        //BSP_ledOn();
    }
    else {
        //BSP_ledOff();
    }

    QS_BEGIN_ID(PAUSED_STAT, 0U) // app-specific record
        QS_U8(1, paused);  // Paused status
    QS_END()
}
//............................................................................
uint32_t BSP_random(void) { // a very cheap pseudo-random-number generator
    // exercise the FPU with some floating point computations
    // NOTE: this code can be only called from a task that created with
    // the option OS_TASK_OPT_SAVE_FP.
    float volatile x = 3.1415926F;
    x = x + 2.7182818F;

    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    uint32_t rnd = l_rnd * (3U*7U*11U*13U*23U);
    l_rnd = rnd; // set for the next time

    return (rnd >> 8);
}
//............................................................................
void BSP_randomSeed(uint32_t seed) {
    l_rnd = seed;
}
//............................................................................
void BSP_terminate(int16_t result) {
    Q_UNUSED_PAR(result);
}
//............................................................................
K_SEM_DEFINE(audio_sem, 0, 1);
void play_audio_func(void* param1, void* param2, void* param3) {
    QS_BEGIN_ID(PHILO_STAT, 0U)
        QS_STR(__func__);
    QS_END()
    for (;;) {
        k_sem_take(&audio_sem, K_NO_WAIT);  // clear any semaphores raised so unblock just once
        if (k_sem_take(&audio_sem, K_FOREVER) == 0) {
            // play audio
            QS_BEGIN_ID(PHILO_STAT, 0U)
                QS_STR(__func__);     // String function
                QS_STR("Playing song.");     // String function
            QS_END()
            play_song(ASpacemanCameTravelling, ARRAY_SIZE(ASpacemanCameTravelling));
        }
    }
}
K_THREAD_DEFINE(play_audio, 1024, play_audio_func, NULL, NULL, NULL, CONFIG_NUM_PREEMPT_PRIORITIES-2, 0, 0);

void BSP_playAudio(void) {
    QS_BEGIN_ID(PHILO_STAT, 0U)
        QS_STR(__func__);
    QS_END()
    k_sem_give(&audio_sem);
}
//............................................................................
void BSP_setTime(struct tm newTime) {

    QS_BEGIN_ID(PHILO_STAT, 0U)
        QS_STR(__func__);     // String function
        QS_U8(1, newTime.tm_hour);
        QS_U8(1, newTime.tm_min);
        QS_U8(1, newTime.tm_sec);
    QS_END()

    struct rtc_time t = {
		.tm_year = newTime.tm_year,
		.tm_mon = newTime.tm_mon,
		.tm_mday = newTime.tm_mday,
		.tm_hour = newTime.tm_hour,
		.tm_min = newTime.tm_min,
		.tm_sec = newTime.tm_sec,
	};

	int ret = rtc_set_time(rtc_pcf8563, &t);
    if (ret < 0) {
        QS_BEGIN_ID(PHILO_STAT, 0U)
            QS_STR(__func__);     // String function
            QS_STR("Failed to set time");
        QS_END()
    }
}
//............................................................................
struct tm BSP_getTime(void) {
    struct tm ret = {0};
    struct rtc_time timeReceived = { 0 };

    rtc_get_time(rtc_pcf8563, &timeReceived);

    ret.tm_hour = timeReceived.tm_hour;
    ret.tm_min = timeReceived.tm_min;
    ret.tm_sec = timeReceived.tm_sec;

    QS_BEGIN_ID(PHILO_STAT, 0U)
        QS_STR(__func__);     // String function
        QS_U8(1, ret.tm_hour);
        QS_U8(1, ret.tm_min);
        QS_U8(1, ret.tm_sec);
    QS_END()

    return ret;
}

//============================================================================
// QF callbacks...
void QF_onStartup(void) {
    k_timeout_t timeout = K_MSEC(1000.0 / BSP_TICKS_PER_SEC);
    k_timer_start(&zephyr_tick_timer, timeout, timeout);
    Q_PRINTK("QF_onStartup\n");
}
//............................................................................
void QF_onCleanup(void) {
    Q_PRINTK("QF_onCleanup\n");
}

// QS callbacks ==============================================================
#ifdef Q_SPY

#include <zephyr/drivers/uart.h>

// select the Zephyr shell UART
// NOTE: you can change this to other UART peripheral if desired
static struct device const *uart_dev =
     DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));

//............................................................................
static void uart_cb(const struct device *dev, void *user_data) {
    if (!uart_irq_update(uart_dev)) {
        return;
    }

    if (uart_irq_rx_ready(uart_dev)) {
        uint8_t buf[32];
        int n = uart_fifo_read(uart_dev, buf, sizeof(buf));
        for (uint8_t const *p = buf; n > 0; --n, ++p) {
            QS_RX_PUT(*p);
        }
    }
}
//............................................................................
uint8_t QS_onStartup(void const *arg) {
    Q_UNUSED_PAR(arg);

    Q_REQUIRE(uart_dev != (struct device *)0);

    static uint8_t qsTxBuf[2*1024]; // buffer for QS-TX channel
    QS_initBuf  (qsTxBuf, sizeof(qsTxBuf));

    static uint8_t qsRxBuf[128];  // buffer for QS-RX channel
    QS_rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    // configure interrupt and callback to receive data
    uart_irq_callback_user_data_set(uart_dev, &uart_cb, (void *)0);
    uart_irq_rx_enable(uart_dev);

    return 1U; // return success
}
//............................................................................
void QS_onCleanup(void) {
}
//............................................................................
QSTimeCtr QS_onGetTime(void) {  // NOTE: invoked with interrupts DISABLED
    return k_cycle_get_32();
}
//............................................................................
// NOTE:
// No critical section in QS_onFlush() to avoid nesting of critical sections
// in case QS_onFlush() is called from Q_onError().
void QS_onFlush(void) {
    uint16_t len = 0xFFFFU; // to get as many bytes as available
    uint8_t const *buf;
    while ((buf = QS_getBlock(&len)) != (uint8_t*)0) {
        for (; len != 0U; --len, ++buf) {
            uart_poll_out(uart_dev, *buf);
        }
        len = 0xFFFFU; // to get as many bytes as available
    }
}
//............................................................................
void QS_doOutput(void) {
    uint16_t len = 0xFFFFU; // big number to get all available bytes

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    uint8_t const *buf = QS_getBlock(&len);
    QF_CRIT_EXIT();

    // transmit the bytes via the UART...
    for (; len != 0U; --len, ++buf) {
        uart_poll_out(uart_dev, *buf);
    }
}
//............................................................................
void QS_onReset(void) {
    sys_reboot(SYS_REBOOT_COLD);
}
//............................................................................
void QS_onCommand(uint8_t cmdId,
                  uint32_t param1, uint32_t param2, uint32_t param3)
{
    Q_UNUSED_PAR(cmdId);
    Q_UNUSED_PAR(param1);
    Q_UNUSED_PAR(param2);
    Q_UNUSED_PAR(param3);
}

#endif // Q_SPY

void play_tone(uint32_t freq, uint32_t duration_ms) {

	// toggle_leds();
    
	bool pause = false;
	if (duration_ms > 30)
	{
		duration_ms -= 30;
		pause = true;
	}

	if (freq == PAUSE)
	{
		pwm_set_dt(&piezo_pwm, PWM_HZ(1000), 1);
	}
	else
	{
		pwm_set_dt(&piezo_pwm, PWM_HZ(freq), PWM_HZ(freq)/2);
	}
	k_sleep(K_MSEC(duration_ms));

	// Turn off the tone
	pwm_set_dt(&piezo_pwm, PWM_HZ(1000), 1);

	if (pause)
		k_sleep(K_MSEC(10));
	
}

void play_song(struct Notes* song, size_t size) {
	for (int i = 0; i < size; i++)
	{
		play_tone(song[i].freq, song[i].duration_ms);
	}
}
