#include "Symulator.h"
#include "raylib.h"

Symulator::~Symulator() {
    for (auto& gate : gates) {
        delete gate;
    }
    gates.clear();
    for (auto &gate : gateMenu) {
        delete gate;
    }
    gateMenu.clear();
    for (auto &in : inputs) {
        delete in;
    }
    inputs.clear();
    for (auto &out : outputs) {
        delete out;
    }
    outputs.clear();
}

void Symulator::DrawPanel() {
    DrawRectangleLines(0, 0, SCREEN_WIDTH, 40, YELLOW);
    DrawRectangleLines(0, 40, 40, SCREEN_HEIGHT - 40, YELLOW);
    DrawRectangleLines(SCREEN_WIDTH - 40, 40, 40, SCREEN_HEIGHT - 40, YELLOW);
    DrawRectangleLines(40, SCREEN_HEIGHT - 40, SCREEN_WIDTH - 2 * 40, 40, YELLOW);

    DrawGateMenu();
}

void Symulator::DrawGate(Gate* gate) {
    float x = gate->rect.x;
    float y = gate->rect.y;

    DrawCircle(x + 45, y + 15, 15, BLUE);
    DrawRectangle(x + 15, y + 0, 30, 30, BLUE);
    DrawLineEx({x + 5, y + 5}, {x + 15, y + 5}, 3.0, BLUE);
    DrawLineEx({x + 5, y + 25}, {x + 15, y + 25}, 3.0, BLUE);
    DrawLineEx({x + 60, y + 15}, {x + 70, y + 15}, 3.0, BLUE);
    DrawCircle(x + 5, y + 5, 5, BLUE);
    DrawCircle(x + 5, y + 25, 5, BLUE);
    DrawCircle(x + 70, y + 15, 5, BLUE);
    DrawText(gate->text.c_str(), x + 20, y + 10, 15, RAYWHITE);
}

void Symulator::DrawInput(Input *in) {
    // float x = pos * HEIGHT + 5;
    // float y = in->pos * (Input::HEIGHT + 5);
    DrawRectangle(0, in->y + 40, 20, 40, RED);
    DrawTriangle({20, in->y + 40}, {20, in->y + 80}, {40, in->y + 60}, RED);
    DrawCircle(45, in->y + 60, 5, RED);
    DrawText(TextFormat("%d", in->value), 5, in->y + 50, 20, RAYWHITE);
}

void Symulator::DrawOutput(Output *out) {
    // float x = 0;
    float y = out->pos * (Output::HEIGHT + 5);
    DrawRectangle(SCREEN_WIDTH - 20, y + 40, 20, 40, RED);
    DrawTriangle({SCREEN_WIDTH - 20, y + 40}, {SCREEN_WIDTH - 40, y + 60}, {SCREEN_WIDTH - 20, y + 80}, RED);
    DrawCircle(SCREEN_WIDTH - 45, y + 60, 5, RED);
    DrawText(TextFormat("%d", out->value), SCREEN_WIDTH - 20, y + 50, 20, RAYWHITE);
}

void Symulator::DrawGates() {
    for (auto &gate : gates) {
        DrawGate(gate);
    }
}

void Symulator::DrawInputs() {
    for (auto &in : inputs) {
        DrawInput(in);
    }
}

void Symulator::DrawOutputs() {
    for (auto &out : outputs) {
        DrawOutput(out);
    }
}

void Symulator::CreateGateMenu() {
    gateMenu.push_back(new Gate(5, 5, "AND"));
    gateMenu.push_back(new Gate(5 + Gate::WIDTH + 20, 5, "OR"));
    gateMenu.push_back(new Gate(5 + 2 * (Gate::WIDTH + 20), 5, "XOR"));
}

void Symulator::CreateInputs() {
    for (int i = 0; i < 8; i++)
        inputs.push_back(new Input(i));
}

void Symulator::CreateOutputs() {
    for (int i = 0; i < 8; i++)
        outputs.push_back(new Output(i));
}

void Symulator::Log(const char* text) {
    DrawText(text, 10, SCREEN_HEIGHT - 20, 10, RAYWHITE);
}

void Symulator::DrawGateMenu() {
    for (auto& gate : gateMenu) {
        DrawGate(gate);
    }
}

Gate* Symulator::CheckGateMenu(const Vector2& pos) {
    if (state != State::GATE_MOVING) {
        for (auto& gate : gateMenu) {
            if (pos.x >= gate->rect.x && pos.x < gate->rect.x + gate->rect.width &&
                pos.y >= gate->rect.y && pos.y < gate->rect.y + gate->rect.height) {

                return gate;
            }
        }
    }
    return nullptr;
}

Gate* Symulator::CheckGates(const Vector2& pos) {
    if (state != State::GATE_MOVING) {
        for (auto &gate : gates) {
            if (pos.x >= gate->rect.x && pos.x < gate->rect.x + gate->rect.width &&
                pos.y >= gate->rect.y && pos.y < gate->rect.y + gate->rect.height) {

                return gate;
            }
        }
    }
    return nullptr;
}

Input* Symulator::CheckInputs(const Vector2& pos) {
    for (auto &in : inputs) {
        Rectangle rect = in->getRectangle();
        if (pos.x >= rect.x && pos.x < rect.x + rect.width &&
            pos.y >= rect.y && pos.y < rect.y + rect.height) {

            return in;
        }
    }
    return nullptr;
}

void Symulator::Update() {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 pos = GetMousePosition();

        Input* in = CheckInputs(pos);
        if (in) {
            in->value = !in->value;
        }
    }

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 pos = GetMousePosition();

        Log(TextFormat("Mouse Pressed x:%f y:%f", pos.x, pos.y));

        Gate *gate = CheckGateMenu(pos);
        if (gate) {
            movingGate = new Gate(gate);
            gates.push_back(movingGate);
            state = State::GATE_MOVING;

            printf("%s %p\n", movingGate->text.c_str(), movingGate);
        } else {
            gate = CheckGates(pos);
            if (gate) {
                movingGate = gate;
                state = State::GATE_MOVING;

                printf("%s %p\n", movingGate->text.c_str(), movingGate);
            }
        }
    }

    if (state == State::GATE_MOVING) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2 delta = GetMouseDelta();
            if (!movingGate) {
                printf("NULL\n");
                return;
            }
            movingGate->rect.x += delta.x;
            movingGate->rect.y += delta.y;
            // printf("NULL\n");
        } else {
            state = State::ACTIVE;
            movingGate = nullptr;

            printf("Gate DROPPED\n");
        }
    }
}

void Symulator::Draw() {
    BeginDrawing();

    ClearBackground(GetColor(0x052c46ff));

    DrawPanel();
    DrawInputs();
    DrawOutputs();
    DrawGates();
    DrawText("Symulator v 1.0", SCREEN_WIDTH - 230, SCREEN_HEIGHT - 20, 10, RAYWHITE);

    EndDrawing();
}

int Symulator::MainLoop()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Symulator ukladow logicznych");
    SetTargetFPS(60);

    CreateGateMenu();
    CreateInputs();
    CreateOutputs();

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        Update();
        Draw();
    }

    CloseWindow();
    return 0;
}

int main() {
    Symulator symulator;
    return symulator.MainLoop();
}