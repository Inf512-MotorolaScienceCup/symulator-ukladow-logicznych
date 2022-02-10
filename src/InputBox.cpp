#include "raylib.h"
#include <iostream>


class InputBox
{
private:
    int scrW;
    int scrH;
    float x;
    float y;
    float width;
    float height;
    float timer = 0.0f;
    
    int letterCount = 0;

    static const int maxInputChars = 20;
    

    
public:
    char input[maxInputChars] = "\0";


    InputBox(float x_par, float y_par, float width_par, float height_par) {
        x = x_par ;
        y = y_par ;
        width = width_par;
        height = height_par;


    }

    void DrawInputBox(float animationTime = 1) {



            scrW = GetScreenWidth();
            scrH = GetScreenHeight();



        

        DrawRectangleRounded(Rectangle{ x * scrW,y * scrH,width * scrW,height * scrH }, 0.1f, 4, Color{ 220, 220, 230,255 });
        DrawRectangleRoundedLines(Rectangle{ x * scrW,y * scrH,width * scrW,height * scrH }, 0.1f, 4, 2, Color{ 31, 31, 41,255 });

        /*
        if (letterCount == 0) {
            timer = timer + GetFrameTime();
            //std::cout << timer<< " - ";
            if (timer >= animationTime) {
                timer = 0.0;
            }
            else if (timer >= animationTime / 2) {
                DrawRectangle(x*scrW + width*scrW * 0.02f, y*scrH + height*scrH * 0.075f, width*scrW * 0.014, height*scrH * 0.85f, Color{ 31, 31, 41,255 });

            }
        }
        else {
            DrawText(input, x*scrW + width*scrW * 0.01, y * scrH + (height * scrH-width*scrW*0.1)/2, width*scrW * 0.1, RED);
        }

        */

        DrawText(input, x * scrW + width * scrW * 0.01, y * scrH + (height * scrH - width * scrW * 0.1) / 2, width * scrW * 0.1, PURPLE);
        int inputWidth = MeasureText(input, width * scrW * 0.1);
        timer = timer + GetFrameTime();
        if (timer >= animationTime) {
            timer = 0.0;
        }
        else if (timer >= animationTime / 2) {
            DrawRectangle(x * scrW + inputWidth + width * scrW * 0.02f, y * scrH + height * scrH * 0.075f, width * scrW * 0.014, height * scrH * 0.85f, Color{ 31, 31, 41,255 });

        }

            

        //inputing keys
        int key = GetCharPressed();
        
        while (key > 0) {

            if ((key >= 32) && (key <= 125) && (letterCount < maxInputChars)) {
                input[letterCount] = (char)key;
                input[letterCount+1] = '\0';
                timer = animationTime/2;
                letterCount++;
            }
            
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            letterCount--;
            if (letterCount < 0) letterCount = 0;
            if (letterCount == 0) timer = 0;
            input[letterCount] = '\0';
            std::cout << GetKeyPressed() << " - ";
        }

        /*
        for (int i = 0; i < sizeof(input); i++) {
            std::cout << input[i];
        }
        std::cout << "\n";
        */
        
    }
};