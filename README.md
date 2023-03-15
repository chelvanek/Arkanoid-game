Welcome to my Arkanoid game!

I've performed development of win64 version using Visual Studio 2022, based on given SDL Framework.
To launch the game start "Arkanoid_win64.exe" file.
If needed, window size can be set by starting via command line: .....\Arkanoid_win64.exe -window 800x600. Supported ratio is 4:3, but other can be attempted too.
Game sets 800x600 window by default.
I have implemented all 4 tasks:

1) Game will calculate a score and output it in console window every time new block gets destroyed, giving 100 per one.
In case of destroying more than 3 in a row, every next destroyed block is rewarded with 300 score points and output is marked with (x3).

2) Some of blocks are orange, which, in case of destruction, will destroy all nearest blocks(they will be counted into score).

3) Here two abilities exist:
-First(green) is saving wall that prevents ball from falling down and deflects it once before disappearing, you can't have more than one wall active at a time;
-Second(red): my version of "damaging" the player. In case of receiving this negative ability you get debuff and move 50% slower for 3 seconds.

4) Abilities fly down from the very top of the screen, starting at random positions and in random times. If player catches an ability its effect is received.

There is also a little detail I've added regarding ball deflection from the platform: ball bounce direction will depend on how far it landed from middle of the platform.

Have fun!

P.S. Only part of the project that i own is game.cpp file.
