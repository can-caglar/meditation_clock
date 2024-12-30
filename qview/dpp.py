# This is an example of QView customization for a specific application
# (DPP in this case). This example animates the Phil images on the
# QView canvas. Additionally, there is a button in the middle of the screen,
# which, when clicked once pauses the DPP ("forks" are not being served).
# A second click on the button, "un-pauses" the DPP ("forks" are served
# to all hungry Philosophers).
#
# This version of the DPP customization uses the application-specific
# trace record QS_USER_00 (PHILO_STAT) produced when the status of
# a Philo changes.
#
# NOTE: this is a desktop application, which you cannot reset (and restarted).
# Therefore, the desktop applications must be started *after* the QView is
# already running and is attached to the QSPY host application.

class DPP:
    def __init__(self):

        # add commands to the Custom menu...
        QView.custom_menu.add_command(label="Custom command",
                                      command=self.cust_command)

        # configure the custom QView.canvas...
        QView.show_canvas() # make the canvas visible
        QView.canvas.configure(width=400, height=260)

        # tuple of activity images (correspond to self._philo_state)
        self._img_led_on = PhotoImage(file=HOME_DIR + "/img/red_led.png")
        self._img_led_off = PhotoImage(file=HOME_DIR + "/img/off_led.png")

        # tuple of philo canvas images (correspond to self._philo_obj)
        self.led = QView.canvas.create_image(210,  75, image=self._img_led_off)

        self.the_time = QView.canvas.create_text(100, 20, 
            text="Time goes here", fill="black", font=('Helvetica 15 bold'))
        
        self.song_count = QView.canvas.create_text(100, 150, text="song count", fill="black", font=('Helvetica 15 bold'))
        
        self.btn_get_time = Button(QView.canvas, text="Get time")
        self.get_time = QView.canvas.create_window(100, 55, window = self.btn_get_time, width=100, height=30)
        
        self.entry_time = Entry(QView.canvas, font=("Arial", 14))
        self.entry_time.bind("<Return>", self.on_enter)
        self.time_entry = QView.canvas.create_window(100, 100, width=100, height=30, window=self.entry_time)

        # button images for UP and DOWN
        self.img_UP  = PhotoImage(file=HOME_DIR + "/img/BTN_UP.gif")
        self.img_DWN = PhotoImage(file=HOME_DIR + "/img/BTN_DWN.gif")

        # images of a button for pause/serve
        self.btn_start_meditation = Button(QView.canvas, text="Start Med", command=self.on_start_meditation)
        self.start_meditation = QView.canvas.create_window(100, 200, 
            window = self.btn_start_meditation, width=100, height=30)

        self.entry_tick = Entry(QView.canvas, font=("Arial", 14))
        self.entry_tick.bind("<Return>", self.on_enter_ticker)
        self.tick_entry = QView.canvas.create_window(320, 20, width=100, height=30, window=self.entry_tick)

        # request target reset on startup...
        # NOTE: Normally, for an embedded application you would like
        # to start with resetting the Target, to start clean with
        # Qs dictionaries, etc.
        #
        # However, this is a desktop application, which you cannot reset
        # (and restart). Therefore, the desktop applications must be started
        # *after* the QView is already running.
        #reset_target()


    # on_reset() callback
    def on_reset(self):
        self.songs_played = 0
        glb_filter("QS_USER_00", GRP_ALL, "-QS_QF_TICK")

        # NOTE: the names of objects for current_obj() must match
        # the QS Object Dictionaries produced by the application.
        current_obj(OBJ_AO, "Meditation_inst")
        current_obj(OBJ_TE, "Meditation_timeEvt")

    # on_run() callback
    def on_run(self):
        pass

    def on_enter(self, event):
        w = event.widget
        QView.print_text(f"Pressed enter on textbox, with data: {w.get()}")
        
        try:
            hours, minutes, seconds = map(int, w.get().split(":"))
            QView.print_text(f"Received time ({hours}:{minutes}:{seconds})")
            try:
                post("NEW_TIME_SIG", pack("<BBB", hours, minutes, seconds))
            except Exception as e:
                QView.print_text(f"Failed to post: {e}")

        except Exception:
            QView.print_text(f"Bad data, needs to be in format hh:mm:ss")
        
        w.delete(0, END)
    
    def on_enter_ticker(self, event):
        try:
            tick_amount = int(event.widget.get())
            QView.print_text(f"Received tick_amount ({tick_amount})")
            try:
                for i in range(tick_amount):
                    tick()
            except Exception as e:
                QView.print_text(f"Failed to tick {e}")

        except Exception:
            QView.print_text(f"Bad data, needs to be integer.")
        
        event.widget.delete(0, END)

    def on_start_meditation(self):
        post("START_MEDITATION_SIG")

    # example of a custom command
    def cust_command(self):
        command(1, 12345)

    # Intercept the QS_USER_00 application-specific trace record.
    # This record has the following structure (see bsp.c:displayPhilStat()):
    # Seq-Num, Record-ID, Timestamp, format-byte, Philo-num,
    #    format-byte, Zero-terminated string (status)
    def QS_USER_00(self, packet):
        # unpack: Timestamp->data[0], Function name
        data = qunpack("xxTxZ", packet)
        function = data[1]

        # perform operation on function called
        if function == "BSP_displayPhilStat":
            data = qunpack("xxTxZxBxZ", packet)
            i = data[2]
            # print a message to the text view
            QView.print_text("%010d Philo %1d is %s"%(data[0], i, data[3]))
        
        if function == "BSP_getTime":
            data = qunpack("xxTxZxBxBxB", packet)
            h = data[2]
            m = data[3]
            s = data[4]
            QView.canvas.itemconfig(self.the_time, text=f"{h:02}:{m:02}:{s:02}")
        
        if function == "BSP_ledOn":
            QView.canvas.itemconfig(self.led, image=self._img_led_on)
            
        if function == "BSP_ledOff":
            QView.canvas.itemconfig(self.led, image=self._img_led_off)
        
        if function == "BSP_playAudio":
            self.songs_played += 1
            QView.canvas.itemconfig(self.song_count, text=f"Songs Played: {self.songs_played}")

#=============================================================================
QView.customize(DPP()) # set the QView customization
