#include <algorithm>
#include <vector>

#include "Symulator.h"
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

namespace sym {

    bool IsInputComponent(Component* comp) {
        return (comp->type == Component::Type::INPUT1 || comp->type == Component::Type::INPUT4 ||
            comp->type == Component::Type::INPUT8);
    }

    bool IsOutputComponent(Component* comp) {
        return (comp->type == Component::Type::OUTPUT1);
    }

    Connector* GetNextConnector(std::vector<Line*>& connections, Connector* conn) {
        if (!conn) return nullptr;

        for (auto& line : connections) {
            if (line->start == conn)
                return line->end;
            if (line->end == conn)
                return line->start;
        }
        return nullptr;
    }

    void UpdateConnections(std::vector<Line*>& connections, Connector* conn) {
        if (!conn) return;

        bool value = conn->value;
        Connector* next = GetNextConnector(connections, conn);
        if (next && next->type == Connector::Type::IN) {
            next->value = value;
            if (next->parent) {
                std::vector<Connector*> outConns;
                next->parent->Calc(outConns);
                for (auto& out : outConns) {
                    UpdateConnections(connections, out);
                }
            }
        }
    }

    void UpdateConnections(std::vector<Component*> comps, std::vector<Line*>& connections) {
        for (auto& comp : comps) {
            if (IsInputComponent(comp))
                UpdateConnections(connections, &comp->outConn);
        }
    }

    Component* Component::Create(Component* comp) {
        switch (comp->type) {
        case Component::Type::INPUT1:
            return new Input(static_cast<Input*>(comp));
        case Component::Type::OUTPUT1:
            return new Output(static_cast<Output*>(comp));
        case Component::Type::GATE:
            return new Gate(static_cast<Gate*>(comp));
        case Component::Type::BLOCK:
            return new Block(static_cast<Block*>(comp));
        }
        return nullptr;
    }

    void Component::Move(const Vector2& delta) {
        rect.x += delta.x;
        rect.y += delta.y;
        outConn.pos.x += delta.x;
        outConn.pos.y += delta.y;
        for (auto& in : inConns) {
            in.pos.x += delta.x;
            in.pos.y += delta.y;
        }
    }

    void Component::Draw() {
        Vector2 pos = GetMousePosition();
        if (CheckCollisionPointCircle(pos, outConn.pos, 5)) {
            DrawRectangleLines(outConn.pos.x - 5, outConn.pos.y - 5, 10, 10, PINK);
        }
        for (auto& in : inConns) {
            if (CheckCollisionPointCircle(pos, in.pos, 5)) {
                DrawRectangleLines(in.pos.x - 5, in.pos.y - 5, 10, 10, PINK);
            }
        }
        if (CheckCollisionPointRec(pos, rect)) {
            DrawRectangleLines(rect.x, rect.y, rect.width, rect.height, PINK);
        }
        if (collide) {
            DrawLine(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height, RED);
            DrawLine(rect.x, rect.y + rect.height, rect.x + rect.width, rect.y, RED);
        }
    }

    Connector* Component::CheckEndpoints(const Vector2& pos) {
        for (auto& in : inConns) {
            if (CheckCollisionPointRec(pos, { in.pos.x - 5, in.pos.y - 5, 10, 10 })) {
                return &in;
            }
        }
        if (CheckCollisionPointRec(pos, { outConn.pos.x - 5, outConn.pos.y - 5, 10, 10 })) {
            return &outConn;
        }
        return nullptr;
    }

    void Gate::Calc(std::vector<Connector*>& outConns) {
        switch (gateType) {
        case Type::NOT:
            outConn.value = !inConns[0].value;
            break;
        case Type::AND:
            outConn.value = inConns[0].value & inConns[1].value;
            break;
        case Type::OR:
            outConn.value = inConns[0].value | inConns[1].value;
            break;
        case Type::XOR:
            outConn.value = inConns[0].value ^ inConns[1].value;
            break;
        }
        return outConns.push_back(&outConn);
    }

    void Gate::Draw() {
        float x = rect.x;
        float y = rect.y;

        DrawCircle(x + 45, y + 15, 15, BLUE);
        DrawRectangle(x + 15, y, 30, HEIGHT, BLUE);

        Vector2 pos = GetMousePosition();
        if (inConns.size() == 1) {
            DrawLineEx({ x + 5, y + 15 }, { x + 15, y + 15 }, 3.0, BLUE);
            DrawCircle(x + 5, y + 15, 5, inConns[0].value ? RED : GRAY);
        }
        else {
            DrawLineEx({ x + 5, y + 5 }, { x + 15, y + 5 }, 3.0, BLUE);
            DrawLineEx({ x + 5, y + 25 }, { x + 15, y + 25 }, 3.0, BLUE);
            DrawCircle(x + 5, y + 5, 5, inConns[0].value ? RED : GRAY);
            DrawCircle(x + 5, y + 25, 5, inConns[1].value ? RED : GRAY);
        }

        DrawLineEx({ x + 60, y + 15 }, { x + 70, y + 15 }, 3.0, BLUE);
        DrawCircle(x + 70, y + 15, 5, outConn.value ? RED : GRAY);
        DrawText(text.c_str(), x + 20, y + 10, 15, RAYWHITE);

        Component::Draw();
    }

    void Input::Draw() {
        Color color = outConn.value ? RED : GRAY;

        DrawRectangle(rect.x, rect.y, 20, HEIGHT, color);
        DrawTriangle({ rect.x + 20, rect.y }, { rect.x + 20, rect.y + HEIGHT }, { rect.x + 30, rect.y + HEIGHT / 2 }, color);
        DrawCircle(outConn.pos.x, outConn.pos.y, 5, color);
        DrawText(TextFormat("%d", outConn.value), rect.x + 5, rect.y + 5, 18, RAYWHITE);

        Component::Draw();
    }

    void Output::Draw() {
        Color color = inConns[0].value ? RED : GRAY;

        DrawRectangle(rect.x + 20, rect.y, 20, HEIGHT, color);
        DrawTriangle({ rect.x + 20, rect.y }, { rect.x + 10, rect.y + HEIGHT / 2 }, { rect.x + 20, rect.y + HEIGHT }, color);
        DrawCircle(inConns[0].pos.x, inConns[0].pos.y, 5, color);
        DrawText(TextFormat("%d", inConns[0].value), rect.x + 20, rect.y + 5, 18, RAYWHITE);

        Component::Draw();
    }

    Block::Block(float x, float y, const char* text, Color color, std::vector<Component*> comps,
        std::vector<Line*> connections)
        : Component(x, y, WIDTH, HEIGHT, text, Component::Type::BLOCK), color(color) {

        for (auto& comp : comps) {
            this->comps.push_back(comp);
        }
        for (auto& line : connections) {
            this->connections.push_back(line);
        }
    }

    Block::Block(const Block* block)
        : Component(block), color(block->color), isIcon(false) {
        numInputs = 0;
        numOutputs = 0;

        for (auto& comp : block->comps) {
            switch (comp->type) {
            case Type::INPUT1:
                numInputs += 1;
                break;
            case Type::INPUT2:
                numInputs += 2;
                break;
            case Type::INPUT4:
                numInputs += 4;
                break;
            case Type::INPUT8:
                numInputs += 8;
                break;
            case Type::OUTPUT1:
                numOutputs += 1;
                break;
            }
            comps.push_back(comp);
        }
        for (auto& line : block->connections) {
            connections.push_back(line);
        }
        rect.height = std::max<float>(HEIGHT, std::max(numInputs, numOutputs) * 15.0 + 10);
        rect.width += 20;
        printf("New height:%f %d %d\n", rect.height, numInputs, numOutputs);

        int inIdx = 0;
        int outIdx = 0;
        for (auto& comp : comps) {
            if (IsInputComponent(comp)) {
                BlockConnector bconn = { this, {rect.x + 5, 5 + rect.y + 15 * inIdx++}, Connector::Type::IN, &comp->outConn };
                inputs.push_back(bconn);
            }
            else if (IsOutputComponent(comp)) {
                BlockConnector bconn = { this, {rect.x + rect.width - 5, 5 + rect.y + 15 * outIdx++}, Connector::Type::OUT, &comp->inConns[0] };
                outputs.push_back(bconn);
            }
        }
    }

    void Block::Calc(std::vector<Connector*>& outConns) {
        for (auto& in : inputs) {
            if (in.conn)
                in.conn->value = in.value;
        }
        UpdateConnections(comps, connections);
        Connector* result = nullptr;
        for (auto& out : outputs) {
            if (out.conn) {
                out.value = out.conn->value;
                result = &out;
            }
            outConns.push_back(&out);
        }
    }

    void Block::Move(const Vector2& delta) {
        rect.x += delta.x;
        rect.y += delta.y;
        for (auto& in : inputs) {
            in.pos.x += delta.x;
            in.pos.y += delta.y;
        }
        for (auto& out : outputs) {
            out.pos.x += delta.x;
            out.pos.y += delta.y;
        }
    }

    void Block::Draw() {
        if (isIcon) {
            DrawRectangleRounded({ rect.x, rect.y, rect.width, rect.height }, 0.3, 5, color);
            DrawText(TextFormat("%s", text.c_str()), rect.x + 5, rect.y + 5, 18, RAYWHITE);
        }
        else {
            DrawRectangleRounded({ rect.x + 10, rect.y, rect.width - 2 * 10, rect.height }, 0.3, 5, color);
            DrawText(TextFormat("%s", text.c_str()), rect.x + 15, rect.y + 5, 18, RAYWHITE);

            Vector2 pos = GetMousePosition();
            for (auto& in : inputs) {
                Color connColor = in.value ? RED : GRAY;
                DrawCircle(in.pos.x, in.pos.y, 5, connColor);

                if (CheckCollisionPointCircle(pos, in.pos, 5)) {
                    DrawRectangleLines(in.pos.x - 5, in.pos.y - 5, 10, 10, PINK);
                }
            }
            for (auto& out : outputs) {
                Color connColor = out.value ? RED : GRAY;
                DrawCircle(out.pos.x, out.pos.y, 5, connColor);

                if (CheckCollisionPointCircle(pos, out.pos, 5)) {
                    DrawRectangleLines(out.pos.x - 5, out.pos.y - 5, 10, 10, PINK);
                }
            }
        }

        Component::Draw();
    }

    Connector* Block::CheckEndpoints(const Vector2& pos) {
        for (auto& in : inputs) {
            if (CheckCollisionPointRec(pos, { in.pos.x - 5, in.pos.y - 5, 10, 10 })) {
                return &in;
            }
        }
        for (auto& out : outputs) {
            if (CheckCollisionPointRec(pos, { out.pos.x - 5, out.pos.y - 5, 10, 10 })) {
                return &out;
            }
        }
        return nullptr;
    }

    Block::~Block() {
        printf("Block dtor num comps:%d\n", comps.size());
        for (auto& comp : comps) {
            printf("Block dtor delete comp\n");
            delete comp;
            comp = nullptr;
        }
        comps.clear();

        for (auto& line : connections) {
            printf("Block dtor delete comp\n");
            delete line;
            line = nullptr;
        }
        connections.clear();
    }

    void MenuPanel::Update() {
        for (auto& button : buttons) {
            button.rec.y = GetScreenHeight() - 40 + yOffset;
        }
    }

    void MenuPanel::Draw() {
        Vector2 pos = GetMousePosition();
        for (auto& button : buttons) {
            bool selected = CheckCollisionPointRec(pos, button.rec);
            Color bg = selected ? ORANGE : DARKBLUE;
            Color fg = selected ? DARKBLUE : ORANGE;
            DrawRectangleRec(button.rec, bg);
            DrawText(button.text, button.rec.x + 10, button.rec.y, 20, fg);
        }
    }

    void CreateBlockDialog::Draw() {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RAYWHITE, 0.8f));

        float width = 250;
        float height = 150;
        Vector2 pos = { (float)GetScreenWidth() / 2 - width / 2, (float)GetScreenHeight() / 2 - height / 2 };
        result = GuiTextInputBox({ pos.x, pos.y, width, height }, "Create Block", "Select name for a block", "Ok;Cancel", name);
        color = GuiColorPicker({ pos.x + width, pos.y, 20, height }, color);

        if (result >= 0) {
            show = false;
            if (result == 1)
                parent->CreateBlock(name, color);
        }
    }

    void CreateBlockDialog::Show() {
        show = true;
    }

    Symulator::~Symulator() {
        for (auto& menu : compMenu) {
            delete menu;
        }
        compMenu.clear();
        DeleteAll();
    }

    void Symulator::DrawPanel() {
        DrawRectangleLines(0, 0, GetScreenWidth(), 40, YELLOW);
        DrawRectangleLines(0, GetScreenHeight() - 40, GetScreenWidth(), 40, YELLOW);
        DrawRectangleLines(0, 0, GetScreenWidth(), GetScreenHeight(), YELLOW);

        DrawComponentMenu();
        menu.Draw();
    }

    void Symulator::DrawComponents() {
        for (auto& comp : comps) {
            comp->Draw();
        }
    }

    void Symulator::DrawConnections() {
        int i = 1;
        for (auto& con : connections) {
            float y1 = con->start->pos.y;
            float y2 = con->end->pos.y;
            float w1 = (con->end->pos.x - con->start->pos.x) * i++ / 10;
            DrawLineEx({ con->start->pos.x, y1 }, { con->start->pos.x + w1, y1 }, 3.0, RAYWHITE);
            DrawLineEx({ con->start->pos.x + w1, y2 }, { con->end->pos.x, y2 }, 3.0, RAYWHITE);
            DrawLineEx({ con->start->pos.x + w1, y1 }, { con->start->pos.x + w1, y2 }, 3.0, RAYWHITE);
        }
    }

    void Symulator::CreateComponentMenu() {
        float x = 20;
        compMenu.push_back(new Gate(x, 5, "AND", Gate::Type::AND));
        x += Gate::WIDTH + 20;
        compMenu.push_back(new Gate(x, 5, "OR", Gate::Type::OR));
        x += Gate::WIDTH + 20;
        compMenu.push_back(new Gate(x, 5, "XOR", Gate::Type::XOR));
        x += Gate::WIDTH + 20;
        compMenu.push_back(new Gate(x, 5, "NOT", Gate::Type::NOT, true));
        x += Gate::WIDTH + 20;
        compMenu.push_back(new Input(x, 5, "IN1"));
        x += Input::WIDTH + 20;
        compMenu.push_back(new Output(x, 5, "OUT1"));
        compMenuX = x + Output::WIDTH + 20;
    }

    void Symulator::CreateBlock(const char* name, Color color) {
        compMenu.push_back(new Block(compMenuX, 5, name, color, comps, connections));
        compMenuX += Block::WIDTH + 20;
        comps.clear();
        connections.clear();
    }

    void Symulator::Log(const char* text) {
        DrawText(text, 10, GetScreenHeight() - 20, 10, RAYWHITE);
    }

    void Symulator::DrawComponentMenu() {
        float width = 20;
        float height = 40;
        for (auto& comp : compMenu) {
            comp->Draw();
        }

        Vector2 pos1 = { 0, 0 };
        DrawTriangle({ pos1.x, pos1.y }, { pos1.x, pos1.y + height },
            { pos1.x + width, pos1.y + height / 2 }, YELLOW);
        Vector2 pos2 = { (float)GetScreenWidth() - 20, 0 };
        DrawTriangle({ pos2.x + width, pos2.y }, { pos2.x, pos2.y + height / 2 },
            { pos2.x + width, pos2.y + height }, YELLOW);
    }

    void Symulator::MoveComponentMenu(float delta) {
        for (auto& comp : compMenu) {
            comp->Move({ delta, 0 });
        }
    }

    void Symulator::DeleteConnection(Connector* conn) {
        for (int i = 0; i < connections.size(); i++) {
            if (connections[i]->start == conn || connections[i]->end == conn) {
                printf("Connection Delete\n");
                Connector* next = nullptr;
                if (conn->type == Connector::Type::OUT) {
                    next = GetNextConnector(connections, conn);
                }

                delete connections[i];
                connections.erase(connections.begin() + i);

                if (next) {
                    next->value = false;
                    UpdateConnections(connections, next);
                }
                break;
            }
        }
    }

    void Symulator::DeleteComponent(Component* comp) {
        if (comp->type == Component::Type::BLOCK) {
            DeleteBlock(static_cast<Block*>(comp));
            return;
        }

        for (auto& in : comp->inConns)
            DeleteConnection(&in);
        DeleteConnection(&comp->outConn);
        comps.erase(std::remove(comps.begin(), comps.end(), comp), comps.end());
    }

    void Symulator::DeleteBlock(Block* comp) {
        for (auto& in : comp->inputs)
            DeleteConnection(&in);
        for (auto& out : comp->outputs)
            DeleteConnection(&out);
        comps.erase(std::remove(comps.begin(), comps.end(), comp), comps.end());
    }

    void Symulator::DeleteAll() {
        for (auto& comp : comps) {
            delete comp;
        }
        comps.clear();
        for (auto& line : connections) {
            delete line;
        }
        connections.clear();
    }

    Component* Symulator::CheckComponentMenu(const Vector2& pos) {
        if (state != State::GATE_MOVING) {
            for (auto& gate : compMenu) {
                if (pos.x >= gate->rect.x && pos.x < gate->rect.x + gate->rect.width &&
                    pos.y >= gate->rect.y && pos.y < gate->rect.y + gate->rect.height) {

                    return gate;
                }
            }
        }
        return nullptr;
    }

    Component* Symulator::CheckComponents(const Vector2& pos) {
        if (state != State::GATE_MOVING) {
            for (auto& comp : comps) {
                if (pos.x >= comp->rect.x && pos.x < comp->rect.x + comp->rect.width &&
                    pos.y >= comp->rect.y && pos.y < comp->rect.y + comp->rect.height) {

                    return comp;
                }
            }
        }
        return nullptr;
    }

    Component* Symulator::CheckInputs(const Vector2& pos) {
        for (auto& comp : comps) {
            if (IsInputComponent(comp) &&
                pos.x >= comp->rect.x && pos.x < comp->rect.x + comp->rect.width &&
                pos.y >= comp->rect.y && pos.y < comp->rect.y + comp->rect.height) {

                return comp;
            }
        }
        return nullptr;
    }

    Connector* Symulator::CheckComponentEndpoints(Component* comp, const Vector2& pos) {
        for (auto& in : comp->inConns) {
            if (CheckCollisionPointRec(pos, { in.pos.x - 5, in.pos.y - 5, 10, 10 })) {
                return &in;
            }
        }
        if (CheckCollisionPointRec(pos, { comp->outConn.pos.x - 5, comp->outConn.pos.y - 5, 10, 10 })) {
            return &comp->outConn;
        }
        return nullptr;
    }

    Connector* Symulator::CheckComponentEndpoints(const Vector2& pos) {
        for (auto& comp : comps) {
            Connector* endPos = comp->CheckEndpoints(pos);
            if (endPos) return endPos;
        }
        return nullptr;
    }

    MenuButton* Symulator::CheckMenu(const Vector2& pos) {
        for (auto& button : menu.buttons) {
            if (CheckCollisionPointRec(pos, button.rec)) {
                menu.selected = &button;
                return menu.selected;
            }
        }
        menu.selected = nullptr;
        return menu.selected;
    }

    bool Symulator::ComponentCollide(Component* comp) {
        // With Gate Menu
        if (CheckCollisionRecs(comp->rect, { 0, 0, (float)GetScreenWidth(), 40 }))
            return true;
        // With Menu
        if (CheckCollisionRecs(comp->rect, { 40, (float)GetScreenHeight() - 40, (float)GetScreenWidth() - 2 * 40, 40 }))
            return true;

        for (auto& g : comps) {
            if (g != comp) {
                if (CheckCollisionRecs(g->rect, comp->rect))
                    return true;
            }
        }
        return false;
    }

    void Symulator::Update() {
        if (state == State::ACTIVE) {
            Vector2 pos = GetMousePosition();

            if (CheckCollisionPointRec(pos, { 0, 0, 20, 40 })) {
                MoveComponentMenu(-5.0);
            }
            if (CheckCollisionPointRec(pos, { (float)GetScreenWidth() - 20, 0, 20, 40 })) {
                MoveComponentMenu(5.0);
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (IsKeyDown(KEY_LEFT_CONTROL)) {
                    Component* in = CheckInputs(pos);
                    if (in) {
                        in->outConn.value = !in->outConn.value;
                    }
                }

                MenuButton* menu = CheckMenu(pos);
                if (menu) {
                    switch (menu->option) {
                    case MenuOption::CREATE:
                        printf("Create selected\n");
                        blockDialog.Show();
                        break;
                    case MenuOption::SAVE:
                        printf("Save selected\n");
                        break;
                    case MenuOption::CLEAR:
                        printf("Clear selected\n");
                        DeleteAll();
                        break;
                    case MenuOption::OPTIONS:
                        printf("Options selected\n");
                        break;
                    }
                }
            }

            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                Connector* conn = CheckComponentEndpoints(pos);
                if (conn) {
                    DeleteConnection(conn);
                }
                else {
                    Component* comp = CheckComponents(pos);
                    if (comp) {
                        DeleteComponent(comp);
                    }
                }
            }

            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                Log(TextFormat("x:%.0f y:%.0f", pos.x, pos.y));

                Component* comp = CheckComponentMenu(pos);
                if (comp) {
                    movingComp = Component::Create(comp);
                    comps.push_back(movingComp);
                    state = State::GATE_MOVING;

                    printf("%s\n", movingComp->text.c_str());
                }
                else if ((comp = CheckComponents(pos)) != nullptr) {
                    lineStart = comp->CheckEndpoints(pos);
                    if (lineStart) {
                        state = State::LINE_DRAWING;

                        printf("Line start\n");
                    }
                    else {
                        movingComp = comp;
                        state = State::GATE_MOVING;

                        printf("%s\n", movingComp->text.c_str());
                    }
                }
            }
        }
        else if (state == State::GATE_MOVING) {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                movingComp->Move(GetMouseDelta());
                movingComp->collide = ComponentCollide(movingComp);
            }
            else {
                if (movingComp->collide) {
                    if (movingComp->prevPos.x >= 0 && movingComp->prevPos.y >= 0) {
                        Vector2 delta = { movingComp->prevPos.x - movingComp->rect.x,
                                         movingComp->prevPos.y - movingComp->rect.y };
                        movingComp->Move(delta);
                        movingComp->collide = false;
                    }
                    else {
                        comps.erase(std::remove(comps.begin(), comps.end(), movingComp), comps.end());
                        delete movingComp;
                    }
                }
                state = State::ACTIVE;
                movingComp->prevPos.x = movingComp->rect.x;
                movingComp->prevPos.y = movingComp->rect.y;
                movingComp = nullptr;

                printf("Gate DROPPED\n");
            }
        }
        else if (state == State::LINE_DRAWING) {
            Vector2 pos = GetMousePosition();
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                DrawLineEx(lineStart->pos, pos, 3.0, RAYWHITE);
            }
            else {
                Connector* conn = CheckComponentEndpoints(pos);
                if (conn) {
                    connections.push_back(new Line(lineStart, conn));
                }
                state = State::ACTIVE;
                printf("Line finished\n");
            }
        }

        UpdateConnections(comps, connections);
        menu.Update();
    }

    void Symulator::Draw() {
        BeginDrawing();

        ClearBackground(GetColor(0x052c46ff));

        if (blockDialog.show) {
            blockDialog.Draw();
        }
        else {
            DrawPanel();
            DrawConnections();
            DrawComponents();
            DrawText("Symulator v 1.0", GetScreenWidth() - 100, GetScreenHeight() - 20, 10, RAYWHITE);
        }

        EndDrawing();
    }

    int Symulator::MainLoop()
    {
        InitWindow(800, 600, "Symulator");
        SetWindowState(FLAG_WINDOW_RESIZABLE);
        SetTargetFPS(60);

        CreateComponentMenu();

        // Main game loop
        while (!WindowShouldClose()) {    // Detect window close button or ESC key
            Update();
            Draw();
        }

        CloseWindow();
        return 0;
    }

} // namespace sym

int main() {
    sym::Symulator sym;
    return sym.MainLoop();
}