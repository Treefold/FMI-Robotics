Game idea: "Flappy Bird"/"Jetpack Joyride" where the player flies while you press the joystick button, and flies down when it's released.

        The game objective is to evade as many obstacles as possible.
        BUT the game purpuse is to make you have fun while challanging your limits.
Video: <TBD>

Instructions:

 *Before Starting the Game*
                    
        - You can move the joystick to the left & right to navigate through the menu
        
        - When you see "Press To Lock" means you have to press the joystick button in order to scroll or to change the displayed value
        
        - When it's locked you can: (joystick up = increase / scroll up; joystick down = decrease / scroll down)
        
                    increase/decrease the level and the matrix brightness (joystick up/down)
                    
                    scroll through highscores (joystick up/down)
                    
                    press again to unlock the menu (press joystick button)
                    
        - Sometimes you will be requested to hold pressed until the countdown finishes in order to confirm your choice
        
                    * whenever you feel ready, go to the first screen menu and hold pressed to start the game
                    
        * you have 2 play modes: 
            - Solo: you can register/authenticate in order to save the highscore or play as a guest (cannot save the highscore)
            - Duo:  2 players are starting the same game - the one with the highest score wins (the scores are not saved)
            
 *During the Game*
 
        - on the LCD are displayed: current level, score, remaining lives and remaining time
        - Press the joystick button to fly higher or release the button to fly lower
        
                    * multiple presses won't take into account
                    
                    * by pressing too fast, it might not be counted as a pressing (it has to be pressed when the played advances)
        
        * the score is calculated by the formula:
        
                    (Time * 100.0) / ObstaclesToPassTheLevel / Speed, where:
                    
                                Time = remaining time to pass the level
                                
                                ObstaclesToPassTheLevel = the number of consecutive obstacles to pass the level
                                
                                Speed = how fast you move 
                                
        * when one level is finished, you will get a bonus life (you cannot have more than 9 lives)
        
        * in order to pass a level, you have to pass through a number of consecutive obstacles within the time limit
        
        * colliding with incoming objects will make you lose one life and resets the current level
        
        * if you run out of lives or time, the game is over (during the last level, the time is infinite)
                    
 *End of the Game*
   
        -  if you are loggedin and you are playing solo than the highscore is updated, else it is not
        
BOM (Bill of Materials):
        - 4 arduino (1 uno + 3 nano)
        - 4 radio trancereceiver (nrf24L01+)
        - 2 "8x8" matrix moduls (+ 8x8 matrixes)
        - 2 joysticks
        - 1 fingerprint sensor (r305/r307)
        - 1 numapad
        - 1 lcd screen with I2C (4 pins)
        * 4 roker switches + batteries if you don't to always keep them pluged in
   
