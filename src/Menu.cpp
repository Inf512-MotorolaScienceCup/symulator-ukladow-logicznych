#include "raylib.h"
#include "Button.cpp"
#include "InputBox.cpp"
#include "fstream"
#include "string"








bool btnNewProjClicked = false;             //setting New Project Button is not clicked (false)


 


int main(void)
{


    const int x = GetScreenWidth();             
    const int y = GetScreenHeight();


    int currentLevel = 0;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(x, y, "Symlator Układów Logicznych");
    MaximizeWindow();
    ShowCursor();

    SetTargetFPS(60);

    //initialize folder
    system("mkdir Projects");


    //Font font = LoadFontEx("assets/Inter-Regular.ttf", 10, 0, 0);

    Button button1 = Button(0.4f, 0.45f, 0.2f, 0.08f, PURPLE, WHITE, "Stwórz nowy projekt", 1.5f);
    Button button2 = Button(0.4f, 0.65f, 0.2f, 0.08f, PURPLE, WHITE, "Wczytaj Projekt", 1.5f);
    Button button3 = Button(0.01f, 0.01f, 0.045f, 0.04f, PURPLE, WHITE, "<-", 1.5f);
    
    

    InputBox inputBox = InputBox(0.325f, 0.3f, 0.35f, 0.1f);
    Button button4 = Button(0.325f, 0.45f, 0.35f, 0.1f, PURPLE, WHITE, "Stwórz", 0.3f);

    while (!WindowShouldClose())
    {

       
        int width = GetScreenWidth();
        int height = GetScreenHeight();

        BeginDrawing();

        ClearBackground(Color{ 94, 88, 196,255 });
        float textSize = width * 0.06;


        

        if (button1.isClicked) {
            currentLevel = 1;
        }

        if (button3.isClicked) {
            currentLevel = 0;
        }

        if (button4.isClicked) {
            currentLevel = 2;
        }

        button1.isClicked = false;
        button3.isClicked = false;
        button4.isClicked = false;

        
        
        switch (currentLevel)
        {
        default:
            //Main Menu

            button1.isClicked = false;
            button3.isClicked = false;



            DrawText("Symulator Bramek Logicznych", (width - MeasureText("Symulator Bramek Logicznych", textSize)) * 0.5f, height * 0.2f, textSize, WHITE);


            button1.DrawButton();



            button2.DrawButton();
            break;

            case 1:
            button3.isClicked = false;

            button3.DrawButton();               //Drawing Back Button
            inputBox.DrawInputBox();
            button4.DrawButton();
            break;


            case 2:
                   //Deprecated, but only working







            //std::string("Projects/" + inputBox.input + ".sue")


            std::ofstream outfile("Projects/" + std::string(inputBox.input) + ".sue");

            //outfile << "my text here!" << std::endl;

            outfile.close();
            std::cout << "Plik stworzony" << std::endl;





            //Symulator sym;



            break;

            
        }



        /*
        if (!button1.isClicked || button3.isClicked) {

            //Main Menu

            button1.isClicked = false;
            button3.isClicked = false;


            int textWidth = MeasureText("Symulator Bramek Logicznych", textSize);
            DrawText("Symulator Bramek Logicznych", (width - textWidth) * 0.5f, height * 0.2f, textSize, WHITE);


            button1.DrawButton();
            


            button2.DrawButton();
            
           

        }
        else if (button1.isClicked) {
            button3.isClicked = false;

            button3.DrawButton();               //Drawing Back Button




            
            inputBox.DrawInputBox();
            button4.DrawButton();

            








        } else if (button4.isClicked) {



            system("mkdir Projects");       //Deprecated, but only working







            //std::string("Projects/" + inputBox.input + ".sue")


            std::ofstream outfile("Projects/" + std::string(inputBox.input) + ".sue");

            //outfile << "my text here!" << std::endl;

            outfile.close();
            std::cout << "Plik stworzony" << std::endl;
        }

        */
        EndDrawing();



    }


    CloseWindow();


    return 0;
}
