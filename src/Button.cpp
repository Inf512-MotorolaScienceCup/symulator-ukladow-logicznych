#include "raylib.h"
//#include "String"


class Button {
private:
    float x;
    float y;
    float width;
    float height;
    float scrW;
    float scrH;
    Color buttonColor;
    Color textColor;
    const char* buttonText;
    float borderRadius;
    Rectangle r;

    Color buttonHovered() {
        if (CheckCollisionPointRec(GetMousePosition(), r)) {
            return Color{ 178, 206, 237,255 };
        }
        else {
            return PURPLE;
        }
    }


    bool isButtonClicked() {
        if (CheckCollisionPointRec(GetMousePosition(), r)) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                return true;
            }
        }
        return false;
    }

    


public:
    bool isClicked = false;
    
    Button(float x_par, float y_par, float width_par, float height_par, Color buttonColor_par, Color textColor_par, const char* buttonText_par, float borderRadius_par) {


        scrW = GetScreenWidth();
        scrH = GetScreenHeight();


        x = x_par;
        y = y_par;
        width = width_par;
        height = height_par;
        buttonColor = buttonColor_par;
        textColor = textColor_par;
        buttonText = buttonText_par;
        borderRadius = borderRadius_par;




        
    }


    void DrawButton() {

        scrW = GetScreenWidth();
        scrH = GetScreenHeight();

        r = Rectangle{ x*scrW,y*scrH,width*scrW,height*scrH };

        DrawRectangleRounded(r, borderRadius, 10, buttonColor);

        float textSize = scrW * 0.015f;
        int textWidth = MeasureText(buttonText, textSize);
        DrawText(buttonText, x*scrW + (width*scrW - textWidth) / 2, y*scrH + (height*scrH) / 2 - textSize / 2, textSize, textColor);


        buttonColor = buttonHovered();

        isClicked = isButtonClicked();

    }

    


};