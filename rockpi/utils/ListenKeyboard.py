# from __future__ import print_function

# Libraries we need
import pyxhook
import time


# This function is called every time a key is presssed
def kbevent(event):
    global running
    # print key info
    print(event)

    # If the ascii value matches spacebar, terminate the while loop
    if event.Ascii == 32:
        running = False


def onMouseEvent(event):
    print(event)

# Create hookmanager
hookman = pyxhook.HookManager()
# Define our callback to fire when a key is pressed down
hookman.KeyDown = kbevent
# Hook the keyboard
hookman.HookKeyboard()

# Hook the mouse
hookman.MouseAllButtonsDown = onMouseEvent
hookman.HookMouse()

# Start our listener
hookman.start()

# Create a loop to keep the application running
running = True
while running:
    time.sleep(0.1)

# Close the listener when we are done
hookman.cancel()