#include <algorithm>

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
    for (auto &connection : connections) {
        delete connection;
    }
    connections.clear();
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
    DrawRectangle(x + 15, y, 30, 30, BLUE);

    if (gate->inConn.size() == 1) {
        DrawLineEx({x + 5, y + 15}, {x + 15, y + 15}, 3.0, BLUE);
        DrawCircle(x + 5, y + 15, 5, gate->inConn[0].value ? RED : GRAY);
    } else {
        DrawLineEx({x + 5, y + 5}, {x + 15, y + 5}, 3.0, BLUE);
        DrawLineEx({x + 5, y + 25}, {x + 15, y + 25}, 3.0, BLUE);
        DrawCircle(x + 5, y + 5, 5, gate->inConn[0].value ? RED : GRAY);
        DrawCircle(x + 5, y + 25, 5, gate->inConn[1].value ? RED : GRAY);
    }

    DrawLineEx({x + 60, y + 15}, {x + 70, y + 15}, 3.0, BLUE);
    DrawCircle(x + 70, y + 15, 5, gate->outConn.value ? RED : GRAY);
    DrawText(gate->text.c_str(), x + 20, y + 10, 15, RAYWHITE);

    if (gate->collide) {
        DrawLine(x + 15, y, x + 55, y + 35, RED);
        DrawLine(x + 15, y + 35, x + 55, y, RED);
    }
}

void Symulator::DrawInput(Input *in) {
    Color color = in->conn.value ? RED : GRAY;
    DrawRectangle(0, in->y, 20, 40, color);
    DrawTriangle({20, in->y}, {20, in->y + 40}, {40, in->y + 20}, color);
    DrawCircle(45, in->y + 20, 5, color);
    DrawText(TextFormat("%d", in->conn.value), 5, in->y + 10, 20, RAYWHITE);

    // DrawRectangleLines(40, in->y + 15, 10, 10, PINK);
}

void Symulator::DrawOutput(Output *out) {
    Color color = out->conn.value ? RED : GRAY;
    DrawRectangle(SCREEN_WIDTH - 20, out->y, 20, 40, color);
    DrawTriangle({SCREEN_WIDTH - 20, out->y}, {SCREEN_WIDTH - 40, out->y + 20}, {SCREEN_WIDTH - 20, out->y + 40}, color);
    DrawCircle(SCREEN_WIDTH - 45, out->y + 20, 5, color);
    DrawText(TextFormat("%d", out->conn.value), SCREEN_WIDTH - 20, out->y + 10, 20, RAYWHITE);

    // DrawRectangleLines(SCREEN_WIDTH - 40 - 10, out->y + 15, 10, 10, PINK);
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

void Symulator::DrawConnections() {
    int i = 1;
    for (auto &con : connections) {
        float y1 = con->start->pos.y;
        float y2 = con->end->pos.y;
        float w1 = (con->end->pos.x - con->start->pos.x) * i++ / 10;
        DrawLineEx({con->start->pos.x, y1}, {con->start->pos.x + w1, y1}, 3.0, RAYWHITE);
        DrawLineEx({con->start->pos.x + w1, y2}, {con->end->pos.x, y2 }, 3.0, RAYWHITE);
        DrawLineEx({con->start->pos.x + w1, y1}, {con->start->pos.x + w1, y2}, 3.0, RAYWHITE);
    }
}

void Symulator::CreateGateMenu() {
    gateMenu.push_back(new Gate(5, 5, "AND", Gate::Type::AND));
    gateMenu.push_back(new Gate(5 + Gate::WIDTH + 20, 5, "OR", Gate::Type::OR));
    gateMenu.push_back(new Gate(5 + 2 * (Gate::WIDTH + 20), 5, "XOR", Gate::Type::XOR));
    gateMenu.push_back(new Gate(5 + 3 * (Gate::WIDTH + 20), 5, "NOT", Gate::Type::NOT, true));
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

void Symulator::AddConnection(Line* line) {
}

Connector* Symulator::GetNextConnector(Connector* conn) {
    if (!conn) return nullptr;

    for (auto& line : connections) {
        if (line->start == conn)
            return line->end;
        if (line->end == conn)
            return line->start;
    }
    return nullptr;
}

void Symulator::UpdateConnections() {
    for (auto& in : inputs) {
        Connector* next = GetNextConnector(&in->conn);
        bool value = in->conn.value;
        while (next) {
            if (next->type == Connector::Type::IN) {
                next->value = value;
                if (next->gate) {
                    next->gate->calc();
                    next = &next->gate->outConn;
                    value = next->gate->outConn.value;
                } else {
                    next = nullptr;
                }
            }
            next = GetNextConnector(next);
        }
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

Connector* Symulator::CheckGateEndpoints(Gate *gate, const Vector2 &pos) {
    for (auto& in : gate->inConn) {
        if (CheckCollisionPointRec(pos, {in.pos.x - 5, in.pos.y - 5, 10, 10})) {
            return &in;
        }
    }
    if (CheckCollisionPointRec(pos, {gate->outConn.pos.x - 5, gate->outConn.pos.y - 5, 10, 10})) {
        return &gate->outConn;
    }
    return nullptr;
}

Connector* Symulator::CheckGatesEndpoints(const Vector2 &pos) {
    for (auto &gate : gates) {
        Connector *endPos = CheckGateEndpoints(gate, pos);
        if (endPos) return endPos;
    }
    return nullptr;
}

Connector* Symulator::CheckInputEndpoints(const Vector2 &pos) {
    float x = 40;
    for (auto &in : inputs) {
        float y = in->y + 15;
        if (CheckCollisionPointRec(pos, {x, y, 10, 10})) {
            return &in->conn;
        }
    }
    return nullptr;
}

Connector* Symulator::CheckOutputEndpoints(const Vector2 &pos) {
    float x = SCREEN_WIDTH - 40 - 10;
    for (auto& out : outputs) {
        float y = out->y + 15;
        if (CheckCollisionPointRec(pos, {x, y, 10, 10})) {
            return &out->conn;
        }
    }
    return nullptr;
}

bool Symulator::GateCollide(Gate *gate) {
    // With Gate Menu
    if (CheckCollisionRecs(gate->rect, {0, 0, SCREEN_WIDTH, 40}))
        return true;
    // With Inputs
    if (CheckCollisionRecs(gate->rect, {0, 40, 40, SCREEN_HEIGHT - 40}))
        return true;
    // With Outputs
    if (CheckCollisionRecs(gate->rect, {SCREEN_WIDTH - 40, 40, 40, SCREEN_HEIGHT - 40}))
        return true;
    // With Menu
    if (CheckCollisionRecs(gate->rect, {40, SCREEN_HEIGHT - 40, SCREEN_WIDTH - 2 * 40}))
        return true;

    for (auto& g : gates) {
        if (g != gate) {
            if (CheckCollisionRecs(g->rect, gate->rect))
                return true;
        }
    }
    return false;
}

void Symulator::Update() {
    if (state == State::ACTIVE) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 pos = GetMousePosition();

            Input* in = CheckInputs(pos);
            if (in) {
                in->conn.value = !in->conn.value;
            }
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2 pos = GetMousePosition();

            Log(TextFormat("x:%.0f y:%.0f", pos.x, pos.y));

            Gate *gate = CheckGateMenu(pos);
            if (gate) {
                movingGate = new Gate(gate);
                gates.push_back(movingGate);
                state = State::GATE_MOVING;

                printf("%s %p\n", movingGate->text.c_str(), movingGate);
            } else {
                gate = CheckGates(pos);
                if (gate) {
                    lineStart = CheckGateEndpoints(gate, pos);
                    if (lineStart) {
                        state = State::LINE_DRAWING;

                        printf("Line start\n");
                    }
                    else {
                        movingGate = gate;
                        state = State::GATE_MOVING;

                        printf("%s %p\n", movingGate->text.c_str(), movingGate);
                    }
                }
            }
        }
    } else if (state == State::GATE_MOVING) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2 delta = GetMouseDelta();
            movingGate->rect.x += delta.x;
            movingGate->rect.y += delta.y;
            movingGate->outConn.pos.x += delta.x;
            movingGate->outConn.pos.y += delta.y;
            for (auto& in : movingGate->inConn) {
                in.pos.x += delta.x;
                in.pos.y += delta.y;
            }
            movingGate->collide = GateCollide(movingGate);
        } else {
            if (movingGate->collide) {
                gates.erase(std::remove(gates.begin(), gates.end(), movingGate), gates.end());
                delete movingGate;
            }
            state = State::ACTIVE;
            movingGate = nullptr;

            printf("Gate DROPPED\n");
        }
    } else if (state == State::LINE_DRAWING) {
        Vector2 pos = GetMousePosition();
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            DrawLineEx(lineStart->pos, pos, 3.0, RAYWHITE);
        } else {
            Connector* conn;
            if ((conn = CheckInputEndpoints(pos)) != nullptr) {
                connections.push_back(new Line(lineStart, conn));
            } else if ((conn = CheckOutputEndpoints(pos)) != nullptr) {
                connections.push_back(new Line(lineStart, conn));
            } else if ((conn = CheckGatesEndpoints(pos)) != nullptr) {
                connections.push_back(new Line(lineStart, conn));
            }
            state = State::ACTIVE;
            printf("Line finished\n");
        }
    }

    UpdateConnections();
}

void Symulator::Draw() {
    BeginDrawing();

    ClearBackground(GetColor(0x052c46ff));

    DrawPanel();
    DrawConnections();
    DrawInputs();
    DrawOutputs();
    DrawGates();
    DrawText("Symulator v 1.0", SCREEN_WIDTH - 230, SCREEN_HEIGHT - 20, 10, RAYWHITE);

    EndDrawing();
}

int Symulator::MainLoop()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Symulator");
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
    Symulator sym;
    return sym.MainLoop();
}
