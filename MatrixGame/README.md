Game idea: 
            "Flappy Bird"/"Jetpack Joyride" where the player flies while you press the joystick button, and flies down when it's released.
            
            The game purpose is to evade as many obstacles as possible.

Video: https://youtu.be/fKGVIYG6WfQ

Instructions: 

     *Before Starting the Game*
     
            - If you cannot see anything on the menu, try changing the potentiometer value until you can see the text clearly
            
                        * the Pause Switch might be on (turn it off)
                        * if it's still not working, try resetting the Uno board
                        
            - You can move the joystick to the left & right to navigate through the menu
            
            - When you see "Press To Lock" means you have to press the joystick button in order to scroll or to change the displayed value
            
            - When it's locked you can: (joystick up = increase / scroll up; joystick down = decrease / scroll down)
            
                        increase/decrease the level and the matrix brightness (joystick up/down)
                        
                        scroll through highscores (joystick up/down)
                        
                        press again to unlock the menu (press joystick button)
                        
            - Sometimes you will be requested to hold pressed until the countdown finishes in order to confirm your choice
            
                        * whenever you feel ready, go to the first screen menu and hold pressed to start the game
                        
     *During the Game*
     
            - on the LCD are displayed: current level, score, remaining lives and remaining time
            - Press the joystick button to fly higher or release the button to fly lower
            
                        * multiple presses won't take into account
                        
                        * by pressing too fast, it might not be counted as a pressing (it has to be pressed when the played advances)
                        
            - Pause the game by switching on the Switch or switch it off in order to play again
            
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
       
            - if you did beat a highscore, you will be asked if you want to register your name or to remain "Unknown"
            
            - if you don't remain "Unknown", you will be requested to provide a name and that will be displayed in the Highscores Screen
            
            * it's quite hard to enter your name, so it will only be requested if you managed to beat a highscore
            
            - you will be redirected to the first menu screen (to try again)
            
     *Bonus*
            
            - You can reset the highscores in case you want a competition
            
HW components: (Also see the MatrixGame.jpg)

  - Arduino Uno + Shield Arduino
  - LCD Display
  - 8x8 Matrix + MAX7219 Driver
  - Joystick
  - Potentiometer
  - Button
  - Switch
  - Resistors (100k, 220)
  - Capacitors (10 micoF, 104 pF)
  - Wires + Breadboards (+ some duct tape)
