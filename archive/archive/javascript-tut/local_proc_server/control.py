import keyboard

if __name__ == "__main__":
    print("hello world")
    running = True
    while running:
        if keyboard.is_pressed('w') or keyboard.is_pressed('up arrow'):
            print("w")
        if keyboard.is_pressed('a') or keyboard.is_pressed('left arrow'):
            print("a")
        if keyboard.is_pressed('s') or keyboard.is_pressed('down arrow'):
            print("s")
        if keyboard.is_pressed('d') or keyboard.is_pressed('right arrow'):
            print("d")
        if keyboard.is_pressed('escape'):
            running = False