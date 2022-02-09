#include "raylib.h"
#include "Button.cpp"
#include "InputBox.cpp"

using namespace std;

bool btnNewProjClicked = false;
//char name[InputBox] = "\0";




int main(void)
{

    const int x = GetScreenWidth();
    const int y = GetScreenHeight();



    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(x, y, "raylib [core] example - basic window");
    MaximizeWindow();
    ShowCursor();

    SetTargetFPS(60);

    Color btn1Color = PURPLE;
    Color btn2Color = PURPLE;
    Color btn3Color = PURPLE;
    //Font font = LoadFontEx("assets/Inter-Regular.ttf", 10, 0, 0);

    

    InputBox inputBox = InputBox(0.325f, 0.3f, 0.35f, 0.1f);


    while (!WindowShouldClose())
    {

       
        int width = GetScreenWidth();
        int height = GetScreenHeight();

        BeginDrawing();

        ClearBackground(Color{ 94, 88, 196,255 });





        float textSize = width * 0.06;

        if (!btnNewProjClicked) {

            int textWidth = MeasureText("Symulator Bramek Logicznych", textSize);
            DrawText("Symulator Bramek Logicznych", (width - textWidth) * 0.5f, height * 0.2f, textSize, WHITE);

            Button button1 = Button(width * 0.4f, height * 0.45f, width * 0.2f, height * 0.08f, width, height, btn1Color, WHITE, "Stwórz nowy projekt");
            btn1Color = button1.buttonHovered(GetMousePosition());
            btnNewProjClicked = button1.buttonClicked(GetMousePosition(), btnNewProjClicked);

            Button button2 = Button(width * 0.4f, height * 0.65f, width * 0.2f, height * 0.08f, width, height, btn2Color, WHITE, "Załaduj Projekt");
            btn2Color = button2.buttonHovered(GetMousePosition());

        }
        else if (btnNewProjClicked) {
            Button button3 = Button(10, 10, width * 0.045f, height * 0.04f, width, height, btn3Color, WHITE, "<-");
            btn3Color = button3.buttonHovered(GetMousePosition());
            btnNewProjClicked = button3.buttonClicked(GetMousePosition(), btnNewProjClicked);


            
            inputBox.DrawInputBox();


           








        }
        EndDrawing();



    }


    CloseWindow();


    return 0;
}
