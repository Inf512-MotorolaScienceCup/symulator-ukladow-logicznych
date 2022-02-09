#include "raylib.h"
//#include "String"

class Button {
public:
    Rectangle r;
    Button(float x, float y, float width, float height, float scrW, float scrH, Color buttonColor, Color textColor, const char* buttonText) {
        r = Rectangle{ x,y,width,height };

        DrawRectangleRounded(r, 1.5f, 500, buttonColor);

        float textSize = scrW * 0.015f;
        int textWidth = MeasureText(buttonText, textSize);
        DrawText(buttonText, x + (width - textWidth) / 2, y + (height) / 2 - textSize / 2, textSize, textColor);
    }

    Color buttonHovered(Vector2 mousePos) {
        if (CheckCollisionPointRec(mousePos, r)) {
            return Color{ 178, 206, 237,255 };
        }
        else {
            return PURPLE;
        }
    }

    bool buttonClicked(Vector2 mousePos, bool btnState) {
        if (CheckCollisionPointRec(mousePos, r)) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                return !btnState;
            }
        }
        return btnState;
    }


};