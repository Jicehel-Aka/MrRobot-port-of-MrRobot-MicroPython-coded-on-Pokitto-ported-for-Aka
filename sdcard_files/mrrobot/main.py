# Copyright (C) 2020 Hannu Viitala
#
# The source code in this file is released under the MIT license.
# Go to http://opensource.org/licenses/MIT for the full license details.
#
# See README.md for all licenses.

# A simple game

import upygame
import umachine
import urandom
import data

# Setup the screen.
screen = upygame.display.set_mode()

# The Pico-8 palette
upygame.display.set_palette_16bit([
    0x0000, 0x5aa9, 0x0706, 0xff7c, 0xa286, 0x194a, 0xbe18, 0x0429, 
    0xf809, 0xfd00, 0xff44, 0x792a, 0x255f, 0x7392, 0xfbb4, 0xfe54])
    
# The list of rocket x and y coordinates
rocketList=[]

# Initialize the cookie.
cookieData = bytearray(1) # Only one byte, 0-255.
cookieObj = umachine.Cookie("MRROBOT", cookieData) # Create the cookie

# Load the cookie from EEPROM
cookieObj.load()
highScore = cookieData[0]

# Start audio
sound = upygame.mixer.Sound()

# The main loop.
robotX = 55
robotY = 74
robotAnimFrameNum = 0
rocketAnimFrameNum = 0
speedX = 0
exit = False
frameCounter = 0
score = 0
isGameRunning = False
while True:
    
    # Read key events.
    eventtype = upygame.event.poll()
    if eventtype != upygame.NOEVENT:
        if eventtype.type== upygame.KEYDOWN:
            if eventtype.key == upygame.K_RIGHT: speedX = 1
            if eventtype.key == upygame.K_LEFT: speedX = -1
            if eventtype.key == upygame.BUT_A:

                # Reset the game to the initial state
                isGameRunning = True
                frameCounter = 0
                rocketList=[[20,-40],[40,0],[60,-60],[80,-20],[100,-80]]
                
        if eventtype.type== upygame.KEYUP: speedX = 0
        
    if isGameRunning:

        # Animate the rocket.
        if frameCounter % 5 == 0:
            rocketAnimFrameNum += 1
            if rocketAnimFrameNum >= len(data.rocketAnimFrames): rocketAnimFrameNum = 0
            
        # Move and draw the rockets
        for rocket in rocketList:
    
            # Move the rocket
            rocket[1] += 1
    
            # Check for collision to the robot
            if( abs((rocket[0]+3) - (robotX+5))<6 and ((rocket[1] + 9) >= robotY) ):
                
                # Play a fanfare
                sound.play_sfx(data.gameOverSfx, len(data.gameOverSfx), True)
                
                # Save the cookie to EEPROM
                if score > highScore:
                    
                    highScore =  score
                    cookieData[0] = highScore
                    cookieObj.save()
                
                # Stop the game
                isGameRunning = False
                break
                
            # Respawn the rocket, if it goes out of screen
            if( rocket[1] > 88 ):
                rocket[1] = 0
                rocket[0] = 5 + urandom.getrandbits(7) * 100 // 128
                sound.play_sfx(data.launchRocketSfx, len(data.launchRocketSfx), 
                    True)
                
            # Draw the rocket animation frame #(10)
            screen.blit(data.rocketAnimFrames[rocketAnimFrameNum], rocket[0], rocket[1])
 
        # Move the robot
        robotX += speedX
        if(robotX < 0 ): robotX = 0
        if(robotX > 100 ): robotX = 100
        
        # Animate the robot.
        if speedX != 0:
            robotAnimFrameNum += 1
            if robotAnimFrameNum >= len(data.robotAnimFrames): robotAnimFrameNum = 0

        # Draw the robot
        screen.blit(data.robotAnimFrames[robotAnimFrameNum], robotX, robotY)
        
        # Calc score 
        score = (frameCounter // 100) % 256
    
    else:
        # Draw text.
        upygame.draw.text(5,44-6,   "Press "+chr(21)+ " to start",8)
    
    # Draw points
    text = "Score: " + str(score) 
    upygame.draw.text(1,1,text,3)
    text = "Best: " + str(highScore) 
    upygame.draw.text(60,1,text,3)

    # Update the display
    upygame.display.flip()
    
    # Increase the screen frame counter.
    frameCounter += 1
    
    
