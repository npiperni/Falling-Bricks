# Falling Bricks
![image](https://github.com/user-attachments/assets/9a2f67b2-6aa1-4b6f-9d90-93c52f2528fc)

---

## Game Controls

- **Left Arrow** – Move piece left
- **Right Arrow** – Move piece right
- **Down Arrow** – Move piece down
- **Up Arrow or Z** – Rotate piece clockwise
- **X** - Rotate piece counterclockwise
- **Space** – Hard drop all the way down
- **P** – Pause game
- **M** – Toggle music
- **S** – Toggle sound effects
- **Esc** – Quit

---

## Game Rules

- Clear rows by filling them with blocks. Each cleared row adds to your score.
- As rows clear and blocks split, gravity can cause more rows to fill up and clear, granting combo bonus points
- After clearing a certain number of lines you level up.
- The game gets faster as your level increases.
- It takes more line clears each level to level up to the next level.
- The game ends when blocks reach the top, or the gamemode goal is reached.

---

-  The points awarded during line clears is calculated using the formula:  10 * (lines_cleared ^ 2) * level * gravity_combo
-  lines_cleared is the number of lines cleared at once (since this is multiplied by 10 it effectively means 10 points for a single, 40 points for a double, 90 points for a triple, and 160 points for a quadruple)
-  level is your current level
-  gravity_combo equals 3 if the line or lines were cleared due to a falling brick caused by the last line clear (triple points bonus). Otherwise it equals 1 and has no effect.

---

## Gamemodes

- 40 Lines: Clear 40 lines as fast as you can with the highest score possible.
- Blitz: Clear as much as you can in 2 minutes.
- Endless: Play for as long as you can to achieve your best score possible.

##  Credits

### Fonts
- **Bubble Pixel-7 Font** https://www.fontspace.com/bubble-pixel-7-font-f15527
- **American Captain Font** https://www.fontspace.com/american-captain-font-f10784
- **Polt Font** https://www.fontspace.com/polt-font-f23964

### Music
- **Distorted Worlds** https://pixabay.com/music/video-games-distorted-worlds-146471/
- **Bit Beats 6 (5)** https://pixabay.com/music/video-games-bit-beats-6-5-171021/
- **Game Music Loop 6** https://pixabay.com/sound-effects/game-music-loop-6-144641/
- **Game Music Loop 5** https://pixabay.com/sound-effects/game-music-loop-5-144569/
- **Game Music Loop 2** https://pixabay.com/music/beats-game-music-loop-2-144037/

### Sound Effects
- **Game Over Arcade** https://pixabay.com/sound-effects/game-over-arcade-6435/
- **90s Game UI 6** https://pixabay.com/sound-effects/90s-game-ui-6-185099/
- **block drop** https://pixabay.com/sound-effects/block-drop-287678/
- **Video Game Menu Click Sounds** https://pixabay.com/sound-effects/video-game-menu-click-sounds-148373/

---

## Built With

- **C**
- **SDL2**
- **SDL2_mixer**
- **SDL2_ttf**
---

## License

- This game was developed for fun and is provided as-is with no warranties or guarantees of functionality or support. Feel free to play and explore.
- Fonts, audio, and other assets included in this project are subject to their own licenses. Redistribution or modification of these assets must follow the terms set by their original creators. Refer to the Credits section for sources.
