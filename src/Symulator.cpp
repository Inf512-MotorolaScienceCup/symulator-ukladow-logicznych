#include <algorithm>
#include <vector>

#include "Symulator.h"
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define GUI_FILE_DIALOG_IMPLEMENTATION
#include "gui_file_dialog.h"

namespace sym {

char Input::nextText = 'a';
char InputBlock::nextText = 'A';

struct CompIdx {
    int compIdx;
    Connector::Type type;
    int connIdx;
};

CompIdx GetComponentIdx(std::vector<Line*>& connections, std::vector<Component*>& comps, Connector* conn) {
    for (int i = 0; i < comps.size(); i++) {
        for (int j = 0; j < comps[i]->outConns.size(); j++) {
            if (&comps[i]->outConns[j] == conn)
                return { i, Connector::Type::OUT, 0 };
        }
        for (int j = 0; j < comps[i]->inConns.size(); j++) {
            if (&comps[i]->inConns[j] == conn)
                return { i, Connector::Type::IN, j };
        }
    }
    return { -1, Connector::Type::IN, -1 };
}

void Write(std::ofstream &os, std::string *data) {
    size_t size = data->size();
    os.write((const char *)&size, sizeof(size_t));
    os.write(data->c_str(), data->size());
}

void Read(std::ifstream &is, std::string *data) {
    size_t size;
    is.read((char *)&size, sizeof(size_t));
    char buffer[255] = {};
    is.read(buffer, size);
    data->append(buffer);
}

bool IsInputComponent(Component *comp) {
    return (comp->type == Component::Type::INPUT1 || comp->type == Component::Type::INPUT2 ||
            comp->type == Component::Type::INPUT4 || comp->type == Component::Type::INPUT8);
}

bool IsOutputComponent(Component *comp) {
    return (comp->type == Component::Type::OUTPUT1 || comp->type == Component::Type::OUTPUT2 ||
            comp->type == Component::Type::OUTPUT4 || comp->type == Component::Type::OUTPUT8);
}

std::vector<Connector*> GetNextConnector(std::vector<Line*>& connections, Connector* conn) {
    if (!conn) return {};

    std::vector<Connector*> nextList;
    for (auto& line : connections) {
        if (conn->type == Connector::Type::IN) {
            if (line->end == conn) {
                nextList.push_back(line->start);
                // Only one connection to input allowed
                break;
            }
        } else {
            if (line->start == conn)
                nextList.push_back(line->end);
        }
    }
    return nextList;
}

void AddConnection(std::vector<Line *> &connections, Connector *conn1, Connector *conn2) {
    if (conn1 == conn2) return;
    if (conn1->type == conn2->type) {
        printf("Error: Connectors with the same type:%d\n", conn2->type);
        return;
    }
    Connector* in = conn1->type == Connector::Type::IN ? conn1 : conn2;
    // Check if out connection is already connected
    for (auto& conn : connections) {
        if (in == conn->end) {
            printf("Error: IN Connector already connected\n");
            return;
        }
    }
    if (conn1->type == Connector::Type::OUT)
        connections.push_back(new Line(conn1, conn2));
    else
        connections.push_back(new Line(conn2, conn1));
}

void UpdateConnections(std::vector<Line *> &connections, Connector *conn) {
    if (!conn) return;

    bool value = conn->value;
    std::vector<Connector*> nextList = GetNextConnector(connections, conn);
    for (auto next : nextList) {
        if (next && next->type == Connector::Type::IN) {
            next->value = value;
            if (next->parent) {
                std::vector<Connector *> outConns;
                next->parent->Calc(outConns);
                for (auto& out : outConns) {
                    UpdateConnections(connections, out);
                }
            }
        }
    }
}

void UpdateConnections(std::vector<Component*> comps, std::vector<Line *> &connections) {
    for (auto& comp : comps) {
        if (IsInputComponent(comp)) {
            for (auto& out : comp->outConns)
                UpdateConnections(connections, &out);
        }

    }
}

Component* Component::Clone(Component* comp) {
    switch (comp->type) {
    case Component::Type::INPUT1:
        return new Input(static_cast<Input*>(comp));
    case Component::Type::INPUT2:
        return new InputBlock(static_cast<InputBlock *>(comp));
    case Component::Type::INPUT4:
        return new InputBlock(static_cast<InputBlock *>(comp));
    case Component::Type::INPUT8:
        return new InputBlock(static_cast<InputBlock *>(comp));
    case Component::Type::OUTPUT1:
        return new Output(static_cast<Output*>(comp));
    case Component::Type::OUTPUT2:
        return new OutputBlock(static_cast<OutputBlock *>(comp));
    case Component::Type::OUTPUT4:
        return new OutputBlock(static_cast<OutputBlock *>(comp));
    case Component::Type::OUTPUT8:
        return new OutputBlock(static_cast<OutputBlock *>(comp));
    case Component::Type::GATE:
        return new Gate(static_cast<Gate*>(comp));
    case Component::Type::BLOCK:
        return new Block(static_cast<Block*>(comp));
    default:
        printf("Component::Clone unknown component type:%d\n", comp->type);
    }
    return nullptr;
}

void Component::Move(const Vector2 &delta) {
    rect.x += delta.x;
    rect.y += delta.y;
    for (auto &out : outConns) {
        out.pos.x += delta.x;
        out.pos.y += delta.y;
    }
    for (auto &in : inConns) {
        in.pos.x += delta.x;
        in.pos.y += delta.y;
    }
}

void Component::Draw() {
    Vector2 pos = GetMousePosition();
    for (auto& out : outConns) {
        if (CheckCollisionPointCircle(pos, out.pos, 5)) {
            DrawRectangleLines(out.pos.x - 5, out.pos.y - 5, 10, 10, PINK);
        }
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
    for (auto &in : inConns) {
        if (CheckCollisionPointRec(pos, {in.pos.x - 5, in.pos.y - 5, 10, 10})) {
            return &in;
        }
    }
    for (auto& out : outConns) {
        if (CheckCollisionPointRec(pos, {out.pos.x - 5, out.pos.y - 5, 10, 10})) {
            return &out;
        }
    }
    return nullptr;
}

Gate::Gate(std::ifstream& s, Component::Type type): Component(s, type) {
    Read(s, &gateType);

    size_t size;
    Read(s, &size);
    for (int i = 0; i < size; i++)
        inConns.emplace_back(s, this);
    
    outConns[0] = Connector(s, this);
}

void Gate::Calc(std::vector<Connector*>& _outConns) {
    switch (gateType) {
    case Type::NOT:
        // FIXME: Not sure about this
        outConns[0].value = !inConns[0].value;
        break;
    case Type::AND:
        outConns[0].value = inConns[0].value & inConns[1].value;
        break;
    case Type::OR:
        outConns[0].value = inConns[0].value | inConns[1].value;
        break;
    case Type::XOR:
        outConns[0].value = inConns[0].value ^ inConns[1].value;
        break;
    }
    return _outConns.push_back(&outConns[0]);
}

void Gate::Draw() {
    float x = rect.x;
    float y = rect.y;

    DrawCircle(x + 45, y + 15, 15, BLUE);
    DrawRectangle(x + 15, y, 30, HEIGHT, BLUE);

    Vector2 pos = GetMousePosition();
    if (inConns.size() == 1) {
        DrawLineEx({x + 5, y + 15}, {x + 15, y + 15}, 3.0, BLUE);
        DrawCircle(x + 5, y + 15, 5, inConns[0].value ? RED : GRAY);
    } else {
        DrawLineEx({x + 5, y + 5}, {x + 15, y + 5}, 3.0, BLUE);
        DrawLineEx({x + 5, y + 25}, {x + 15, y + 25}, 3.0, BLUE);
        DrawCircle(x + 5, y + 5, 5, inConns[0].value ? RED : GRAY);
        DrawCircle(x + 5, y + 25, 5, inConns[1].value ? RED : GRAY);
    }

    DrawLineEx({x + 60, y + 15}, {x + 70, y + 15}, 3.0, BLUE);
    DrawCircle(x + 70, y + 15, 5, outConns[0].value ? RED : GRAY);
    DrawText(text.c_str(), x + 20, y + 10, 15, RAYWHITE);

    Component::Draw();
}

void Gate::Save(std::ofstream& s) {
    Write(s, &type);
    Write(s, &rect);
    Write(s, &text);
    Write(s, &gateType);

    size_t size = inConns.size();
    Write(s, &size);
    for (auto& inConn : inConns)
        inConn.Save(s);

    outConns[0].Save(s);
}

Input::Input(std::ifstream& s, Component::Type type): Component(s, type) {
    outConns[0] = Connector(s, this);
}

void Input::Draw() {
    Color color = outConns[0].value ? RED : GRAY;

    DrawRectangle(rect.x, rect.y, 20, HEIGHT, color);
    DrawTriangle({rect.x + 20, rect.y}, {rect.x + 20, rect.y + HEIGHT}, {rect.x + 30, rect.y + HEIGHT / 2}, color);
    DrawCircle(outConns[0].pos.x, outConns[0].pos.y, 5, color);
    DrawText(TextFormat("%.2s", text.c_str()), rect.x + 5, rect.y + 5, 18, RAYWHITE);

    Component::Draw();
}

void Input::Save(std::ofstream& s) {
    Write(s, &type);
    Write(s, &rect);
    Write(s, &text);
    outConns[0].Save(s);
}

InputBlock::InputBlock(std::ifstream& s, Component::Type type) : Component(s, type) {
    size_t size;
    Read(s, &size);
    for (int i = 0; i < size; i++)
        outConns.emplace_back(s, this);

    Read(s, &isIcon);
}

void InputBlock::Draw() {
    if (isIcon) {
        Color color = GRAY;
        DrawRectangle(rect.x, rect.y, 20, HEIGHT, color);
        DrawTriangle({rect.x + 20, rect.y}, {rect.x + 20, rect.y + HEIGHT},
                     {rect.x + 30, rect.y + HEIGHT / 2}, color);
        DrawCircle(outConns[0].pos.x, outConns[0].pos.y, 5, color);
        DrawText(TextFormat("%.2s", text.c_str()), rect.x + 5, rect.y + 5, 18, RAYWHITE);
    } else {
        int value = 0;
        int mod = 1;

        Vector2 pos = GetMousePosition();

        for (auto &out : outConns) {
            Color connColor = out.value ? RED : GRAY;
            DrawCircle(out.pos.x, out.pos.y, 5, connColor);

            value += out.value * mod;
            mod *= 2;
        }
        float saturation = value; // 0.5 (1) - 1.0 (255)
        Color color = value ? ColorFromHSV(360, 0.5 + (float)value / 512, 1) : GRAY;
        DrawRectangleRounded({rect.x, rect.y, rect.width - 10, rect.height}, 0.3, 5, color);
        DrawText(TextFormat("%d", value), rect.x + 15, rect.y + 5, 16, RAYWHITE);
    }

    Component::Draw();
}

void InputBlock::Save(std::ofstream& s) {
    Write(s, &type);
    Write(s, &rect);
    Write(s, &text);

    size_t size = outConns.size();
    Write(s, &size);
    for (auto& outConn : outConns)
        outConn.Save(s);

    Write(s, &isIcon);
}

Output::Output(std::ifstream& s, Component::Type type) : Component(s, type) {
    size_t size;
    Read(s, &size);
    for (int i = 0; i < size; i++)
        inConns.emplace_back(s, this);
}

void Output::Draw() {
    Color color = inConns[0].value ? RED : GRAY;

    DrawRectangle(rect.x + 20, rect.y, 20, HEIGHT, color);
    DrawTriangle({rect.x + 20, rect.y}, {rect.x + 10, rect.y + HEIGHT / 2 }, {rect.x + 20, rect.y + HEIGHT}, color);
    DrawCircle(inConns[0].pos.x, inConns[0].pos.y, 5, color);
    DrawText(TextFormat("%.2s", text.c_str()), rect.x + 20, rect.y + 5, 18, RAYWHITE);

    Component::Draw();
}

void Output::Save(std::ofstream& s) {
    Write(s, &type);
    Write(s, &rect);
    Write(s, &text);

    size_t size = inConns.size();
    Write(s, &size);
    for (auto& inConn : inConns)
        inConn.Save(s);
}

OutputBlock::OutputBlock(std::ifstream& s, Component::Type type) : Component(s, type) {
    size_t size;
    Read(s, &size);
    for (int i = 0; i < size; i++)
        inConns.emplace_back(s, this);

    Read(s, &isIcon);
}

void OutputBlock::Draw() {
    if (isIcon) {
        Color color = GRAY;
        DrawRectangle(rect.x + 20, rect.y, 20, HEIGHT, color);
        DrawTriangle({rect.x + 20, rect.y}, {rect.x + 10, rect.y + HEIGHT / 2},
                     {rect.x + 20, rect.y + HEIGHT}, color);
        DrawCircle(inConns[0].pos.x, inConns[0].pos.y, 5, color);
        DrawText(TextFormat("%.2s", text.c_str()), rect.x + 20, rect.y + 5, 18, RAYWHITE);
    } else {
        int value = 0;
        int mod = 1;

        Vector2 pos = GetMousePosition();
        for (auto &in : inConns) {
            Color connColor = in.value ? RED : GRAY;
            DrawCircle(in.pos.x, in.pos.y, 5, connColor);

            value += in.value * mod;
            mod *= 2;
        }
        float saturation = value; // 0.5 (1) - 1.0 (255)
        Color color = value ? ColorFromHSV(360, 0.5 + (float)value / 512, 1) : GRAY;
        DrawRectangleRounded({rect.x + 10, rect.y, rect.width - 10, rect.height}, 0.3, 5, color);
        DrawText(TextFormat("%d", value), rect.x + 15, rect.y + 5, 16, RAYWHITE);
    }

    Component::Draw();
}

void OutputBlock::Save(std::ofstream& s) {
    Write(s, &type);
    Write(s, &rect);
    Write(s, &text);

    size_t size = inConns.size();
    Write(s, &size);
    for (auto& inConn : inConns)
        inConn.Save(s);

    Write(s, &isIcon);
}
/*
void BlockConnector::Save(std::ofstream& s) {
    Write(s, &type);
    Write(s, &rect);
    Write(s, text.c_str(), text.size());

    size_t size = inputs.size();
    Write(s, &size);
    for (auto& input : inputs)
        Write(s, &input);

    Write(s, &outConn);
}
*/
Block::Block(float x, float y, const char *text, Color color, std::vector<Component *> comps,
      std::vector<Line *> connections)
    : Component(x, y, WIDTH, HEIGHT, text, Component::Type::BLOCK), color(color) {

    for (auto& comp : comps) {
        this->comps.push_back(comp);
    }
    for (auto& line : connections) {
        this->connections.push_back(line);
    }
}

Block::Block(const Block *block)
    : Component(block), color(block->color), isIcon(false) {
    numInputs = 0;
    numOutputs = 0;

    for (auto &comp : block->comps) {
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
        case Type::OUTPUT2:
            numOutputs += 1;
            break;
        case Type::OUTPUT4:
            numOutputs += 1;
            break;
        case Type::OUTPUT8:
            numOutputs += 1;
            break;
        }
        comps.push_back(comp);
    }
    for (auto &line : block->connections) {
        connections.push_back(line);
    }

    rect.width += 20;

    int inIdx = 0;
    int outIdx = 0;
    for (auto& comp : comps) {
        if (IsInputComponent(comp)) {
            for (auto& out : comp->outConns) {
                inConns.push_back({this, {rect.x + 5, 10 + rect.y + 15 * inIdx++}, Connector::Type::IN, &out});
            }
        } else if (IsOutputComponent(comp)) {
            for (auto& in : comp->inConns) {
                outConns.push_back({this, {rect.x + rect.width - 5, 10 + rect.y + 15 * outIdx++}, Connector::Type::OUT, &in});
            }
        }
    }

    rect.height = std::max<float>(HEIGHT, std::max(inConns.size(), outConns.size()) * 15.0 + 10);
    printf("New height:%f %d %d\n", rect.height, numInputs, numOutputs);
    printf("inputs:%d outputs:%d \n", inConns.size(), outConns.size());
}

Block::Block(std::ifstream& s, Component::Type type) : Component(s, type) {
    size_t size;
    Read(s, &size);
    for (int i = 0; i < size; i++)
        ;

    Read(s, &size);
    for (int i = 0; i < size; i++) {
        CompIdx idxStart;
        Read(s, &idxStart);
        CompIdx idxEnd;
        Read(s, &idxEnd);

        Connector* start = nullptr;
        Connector* end = nullptr;
        if (idxStart.type == Connector::Type::OUT) {
            // FIXME: WIP
            start = &comps[idxStart.compIdx]->outConns[0];
        } else {
            start = &comps[idxStart.compIdx]->inConns[idxStart.connIdx];
        }
        if (idxEnd.type == Connector::Type::OUT) {
            // FIXME: WIP
            end = &comps[idxEnd.compIdx]->outConns[0];
        } else {
            end = &comps[idxEnd.compIdx]->inConns[idxEnd.connIdx];
        }
        connections.push_back(new Line(start, end));
    }


    Read(s, &color);
    Read(s, &isIcon);
    Read(s, &numInputs);
    Read(s, &numOutputs);
}

void Block::Calc(std::vector<Connector*>& _outConns) {
    for (auto& in : inConns) {
        if (in.conn)
            in.conn->value = in.value;
    }
    UpdateConnections(comps, connections);
    Connector* result = nullptr;
    for (auto& out : outConns) {
        if (out.conn) {
            out.value = out.conn->value;
            result = &out;
        }
        _outConns.push_back(&out);
    }
}

void Block::Move(const Vector2 &delta) {
    rect.x += delta.x;
    rect.y += delta.y;
    for (auto& in : inConns) {
        in.pos.x += delta.x;
        in.pos.y += delta.y;
    }
    for (auto &out : outConns) {
        out.pos.x += delta.x;
        out.pos.y += delta.y;
    }
}

void Block::Draw() {
    if (isIcon) {
        DrawRectangleRounded({rect.x, rect.y, rect.width, rect.height}, 0.3, 5, color);
        DrawText(TextFormat("%s", text.c_str()), rect.x + 5, rect.y + 5, 18, RAYWHITE);
    } else {
        DrawRectangleRounded({rect.x + 10, rect.y, rect.width - 2 * 10, rect.height}, 0.3, 5, color);
        DrawText(TextFormat("%s", text.c_str()), rect.x + 15, rect.y + 5, 18, RAYWHITE);

        Vector2 pos = GetMousePosition();
        for (auto& in : inConns) {
            Color connColor = in.value ? RED : GRAY;
            DrawCircle(in.pos.x, in.pos.y, 5, connColor);

            if (CheckCollisionPointCircle(pos, in.pos, 5)) {
                DrawRectangleLines(in.pos.x - 5, in.pos.y - 5, 10, 10, PINK);
            }
        }
        for (auto& out : outConns) {
            Color connColor = out.value ? RED : GRAY;
            DrawCircle(out.pos.x, out.pos.y, 5, connColor);

            if (CheckCollisionPointCircle(pos, out.pos, 5)) {
                DrawRectangleLines(out.pos.x - 5, out.pos.y - 5, 10, 10, PINK);
            }
        }
    }

    Component::Draw();
}

Connector* Block::CheckEndpoints(const Vector2 &pos) {
    for (auto& in : inConns) {
        if (CheckCollisionPointRec(pos, {in.pos.x - 5, in.pos.y - 5, 10, 10})) {
            return &in;
        }
    }
    for (auto& out : outConns) {
        if (CheckCollisionPointRec(pos, {out.pos.x - 5, out.pos.y - 5, 10, 10})) {
            return &out;
        }
    }
    return nullptr;
}

void Block::Save(std::ofstream& s) {
    Write(s, &type);
    Write(s, &rect);
    Write(s, &text);

    size_t size = comps.size();
    Write(s, &size);
    for (auto& comp: comps)
        comp->Save(s);

    size = connections.size();
    Write(s, &size);
    for (auto& connection : connections) {
        CompIdx idxStart = GetComponentIdx(connections, comps, connection->start);
        Write(s, &idxStart);
        CompIdx idxEnd = GetComponentIdx(connections, comps, connection->end);
        Write(s, &idxEnd);
    }

    size = inConns.size();
    Write(s, &size);
    for (auto& input : inConns)
        input.Save(s);

    size = outConns.size();
    Write(s, &size);
    for (auto& output : outConns)
        output.Save(s);

    Write(s, &color);
    Write(s, &isIcon);
    Write(s, &numInputs);
    Write(s, &numOutputs);
}

Block::~Block() {
    printf("Block dtor num comps:%d\n", comps.size());
    for (auto &comp : comps) {
        printf("Block dtor delete comp\n");
        // FIXME:
        // delete comp;
        // comp = nullptr;
    }
    comps.clear();

    printf("Block dtor num connections:%d\n", connections.size());
    for (auto &line : connections) {
        printf("Block dtor delete comp\n");
        // FIXME:
        // delete line;
        // line = nullptr;
    }
    connections.clear();
}

void MenuPanel::Update() {
    for (auto &button : buttons) {
        button.rec.y = GetScreenHeight() - 40 + yOffset;
    }
}

void MenuPanel::Draw() {
    Vector2 pos = GetMousePosition();
    for (auto &button : buttons) {
        bool selected = CheckCollisionPointRec(pos, button.rec);
        Color bg = selected ? ORANGE : DARKBLUE;
        Color fg = selected ? DARKBLUE : ORANGE;
        DrawRectangleRec(button.rec, bg);
        DrawText(button.text, button.rec.x + 10, button.rec.y, 20, fg);
    }
}

void MenuPanel::Save(std::ofstream& s) {
    for (auto& button : buttons)
        Write(s, &button);
}

void MainMenu::Update() {
    short int i = -1;
    for (auto &button : buttons) {
        button.rec.y = GetScreenHeight() / 2 + i * 70 - 50;
        button.rec.x = GetScreenWidth() / 2 - button.rec.width / 2;
        i += 2;
    }
}

void MainMenu::Draw() {
    Vector2 pos = GetMousePosition();
    for (auto &button : buttons) {
        bool selected = CheckCollisionPointRec(pos, button.rec);
        Color bg = selected ? ORANGE : DARKBLUE;
        Color fg = selected ? DARKBLUE : ORANGE;
        DrawRectangleRec(button.rec, bg);
        DrawText(button.text, button.rec.x + 20, button.rec.y + 25, 40, fg);
    }
}

void MainMenu::Save(std::ofstream& s) {
    for (auto& button : buttons)
        Write(s, &button);
}

void Dialog::Draw() {
    if (type == Type::NONE) return;
    if (cooldown) {
        cooldown = false;
        return;
    }

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RAYWHITE, 0.8f));

    float width = 250;
    float height = 150;
    Vector2 pos = { (float)GetScreenWidth() / 2 - width / 2, (float)GetScreenHeight() / 2 - height / 2 };

    if (type == Type::CREATE_BLOCK) {
        result = GuiTextInputBox({ pos.x, pos.y, width, height }, "Create Block", "Select name for a block", "Ok;Cancel", blockName);
        color = GuiColorPicker({ pos.x + width, pos.y, 20, height }, color);

        if (result >= 0) {
            type = Type::NONE;
            if (result == 1)
                parent->CreateBlock(blockName, color);
        }
    } else if (type == Type::NEW) {
        result = GuiTextInputBox({ pos.x, pos.y, width, height }, "Create Project", "Give your project a name","Ok;Cancel", projectName);

        if (result >= 0) {
            type = Type::NONE;
            if (result == 1) {
                parent->name += GetWorkingDirectory();
                parent->name += "\\projects\\";
                parent->name += projectName;
                parent->name += ".psf";
                parent->state = Symulator::State::ACTIVE;
            }
        }
    } else {
        GuiFileDialogState fileDialogState = InitGuiFileDialog(GetScreenWidth() / 1.5, GetScreenHeight() / 1.7, "Select project", false);
        bool closeWindow = false;
        fileDialogState.fileDialogActive = true;
        char fileNameToLoad[50] = { 0 };

        while (!WindowShouldClose() && !closeWindow) {
            if (!fileDialogState.fileDialogActive)
                closeWindow = true;

            if (fileDialogState.SelectFilePressed) {
                if (IsFileExtension(fileDialogState.fileNameText, ".psf")) {
                    parent->name += fileDialogState.dirPathText;
                    parent->name += "\\";
                    parent->name += fileDialogState.fileNameText;
                    parent->LoadProject();
                }
                fileDialogState.SelectFilePressed = false;
            }

            BeginDrawing();
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

            DrawText(fileNameToLoad, 208, GetScreenHeight() - 20, 10, GRAY);

            if (fileDialogState.fileDialogActive) 
                GuiLock();

            GuiUnlock();
            GuiFileDialog(&fileDialogState);

            EndDrawing();
        }
        type = Type::NONE;
    }
}

void Dialog::Show(Type showType) {
    type = showType;
    cooldown = true;
}

Symulator::~Symulator() {
    for (auto &menu : compMenu) {
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
    DrawText(TextFormat("[ %s ]", name.c_str()), 5, 45, 16, YELLOW);
    menu.Draw();
}

void Symulator::DrawComponents() {
    for (auto &comp : comps) {
        comp->Draw();
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
    compMenu.push_back(new Input(x, 5, "I1"));
    x += Input::WIDTH + 20;
    compMenu.push_back(new InputBlock(x, 5, Component::Type::INPUT2, "I2"));
    x += Input::WIDTH + 20;
    compMenu.push_back(new InputBlock(x, 5, Component::Type::INPUT4, "I4"));
    x += Input::WIDTH + 20;
    compMenu.push_back(new InputBlock(x, 5, Component::Type::INPUT8, "I8"));
    x += Input::WIDTH + 20;
    compMenu.push_back(new Output(x, 5, "O1"));
    x += Input::WIDTH + 20;
    compMenu.push_back(new OutputBlock(x, 5, Component::Type::OUTPUT2, "O2"));
    x += Input::WIDTH + 20;
    compMenu.push_back(new OutputBlock(x, 5, Component::Type::OUTPUT4, "O4"));
    x += Input::WIDTH + 20;
    compMenu.push_back(new OutputBlock(x, 5, Component::Type::OUTPUT8, "O8"));

    compMenuNextX = x + Output::WIDTH + 20;
}

void Symulator::CreateBlock(const char* name, Color color) {
    compMenu.push_back(new Block(compMenuNextX, 5, name, color, comps, connections));
    compMenuNextX += Block::WIDTH + 20;
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

    Color color = ColorAlpha(YELLOW, 0.8);
    if (compMenu[0]->rect.x < 20) {
        Vector2 pos1 = {0, 0};
        DrawTriangle({pos1.x, pos1.y}, {pos1.x, pos1.y + height},
                     {pos1.x + width, pos1.y + height / 2}, color);
    }
    if (compMenuNextX > GetScreenWidth()) {
        Vector2 pos2 = {(float)GetScreenWidth() - 20, 0};
        DrawTriangle({pos2.x + width, pos2.y}, {pos2.x, pos2.y + height / 2},
                     {pos2.x + width, pos2.y + height}, color);
    }
}

void Symulator::MoveComponentMenu(float delta) {
    float minX = 20;
    float maxX = GetScreenWidth();
    if (delta < 0) {
        if (compMenuNextX < maxX) {
            return;
        }
    } else {
        float newX = compMenu[0]->rect.x + delta;
        if (newX > minX) {
            return;
        }
    }
    compMenuNextX += delta;
    for (auto &comp : compMenu) {
        comp->Move({delta, 0});
    }
}

void Symulator::DeleteConnection(Connector* conn) {
    std::vector<Connector *> nextList;
    std::list<int> idxToDelete;
    for (int i = 0; i < connections.size(); i++) {
        if (connections[i]->start == conn || connections[i]->end == conn) {
            printf("Connection Delete %d\n", i);
            if (conn->type == Connector::Type::OUT && nextList.size() == 0) {
                nextList = GetNextConnector(connections, conn);
            }

            idxToDelete.push_front(i);
        }
    }

    for (auto i : idxToDelete) {
        delete connections[i];
        connections.erase(connections.begin() + i);
    }

    for (auto next : nextList) {
        printf("next type:%d val:%d\n", next->type, next->value);
        next->value = false;
        UpdateConnections(connections, next);
    }
}

void Symulator::DeleteComponent(Component* comp) {
    if (comp->type == Component::Type::BLOCK) {
        DeleteBlock(static_cast<Block *>(comp));
        return;
    }

    for (auto& in : comp->inConns)
        DeleteConnection(&in);
    for (auto& out : comp->outConns)
        DeleteConnection(&out);
    comps.erase(std::remove(comps.begin(), comps.end(), comp), comps.end());
}

void Symulator::DeleteBlock(Block* comp) {
    for (auto &in : comp->inConns)
        DeleteConnection(&in);
    for (auto &out : comp->outConns)
        DeleteConnection(&out);
    comps.erase(std::remove(comps.begin(), comps.end(), comp), comps.end());
}

void Symulator::DeleteAll() {
    for (auto &comp : comps) {
        delete comp;
    }
    comps.clear();
    for (auto &line : connections) {
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
        for (auto &comp : comps) {
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
        if (IsInputComponent(comp) && CheckCollisionPointRec(pos, comp->rect))
            return comp;
    }
    return nullptr;
}

Connector *Symulator::CheckInputConnectors(const Vector2 &pos) {
    for (auto &comp : comps) {
        if (IsInputComponent(comp)) {
            for (auto& out : comp->outConns) {
                if (CheckCollisionPointRec(pos, {out.pos.x - 5, out.pos.y - 5, 10, 10}))
                    return &out;
            }
        }
    }
    return nullptr;
}

Connector* Symulator::CheckComponentEndpoints(Component* comp, const Vector2 &pos) {
    for (auto& in : comp->inConns) {
        if (CheckCollisionPointRec(pos, {in.pos.x - 5, in.pos.y - 5, 10, 10})) {
            return &in;
        }
    }
    for (auto& out : comp->outConns) {
        if (CheckCollisionPointRec(pos, {out.pos.x - 5, out.pos.y - 5, 10, 10})) {
            return &out;
        }
    }
    return nullptr;
}

Connector* Symulator::CheckComponentEndpoints(const Vector2 &pos) {
    for (auto &comp : comps) {
        Connector *endPos = comp->CheckEndpoints(pos);
        if (endPos) return endPos;
    }
    return nullptr;
}

MenuButton* Symulator::CheckMenu(const Vector2 &pos) {
    if (state == State::MENU) {
        for (auto& button : mainMenu.buttons) {
            if (CheckCollisionPointRec(pos, button.rec)) {
                mainMenu.selected = &button;
                return mainMenu.selected;
            }
        }
    } else {
        for (auto& button : menu.buttons) {
            if (CheckCollisionPointRec(pos, button.rec)) {
                menu.selected = &button;
                return menu.selected;
            }
        }
    }
    menu.selected = nullptr;
    return menu.selected;
}

bool Symulator::ComponentCollide(Component* comp) {
    // With Gate Menu
    if (CheckCollisionRecs(comp->rect, {0, 0, (float)GetScreenWidth(), 40}))
        return true;
    // With Menu
    if (CheckCollisionRecs(comp->rect, {40, (float)GetScreenHeight() - 40, (float)GetScreenWidth() - 2 * 40, 40}))
        return true;

    for (auto& g : comps) {
        if (g != comp) {
            if (CheckCollisionRecs(g->rect, comp->rect))
                return true;
        }
    }
    return false;
}

void Symulator::ReadProjectData(std::ifstream& s) {
    size_t size;
    Read(s, &size);
    for (int i = 8; i < size; i++) {
        Component::Type type;
        Read(s, &type);
        if (type == Component::Type::BLOCK) {
            //compMenu.push_back(new Block(s));
        } else
            printf("Warning! Incorrect componet type: %d", type);
    }

    Read(s, &size);
    comps.reserve(size);
    for (int i = 0; i < size; i++) {
        Component::Type type;
        Read(s, &type);
        switch (type) {
        case Component::Type::INPUT1:
            comps.push_back(new Input(s, type));
            break;
        case Component::Type::OUTPUT1:
            comps.push_back(new Output(s, type));
            break;
        case Component::Type::OUTPUT8:
            comps.push_back(new OutputBlock(s, type));
            break;
        case Component::Type::GATE:
            comps.push_back(new Gate(s, type));
            break;
        case Component::Type::INPUT2:
        case Component::Type::INPUT4:
        case Component::Type::INPUT8:
            comps.push_back(new InputBlock(s, type));
            break;
        case Component::Type::BLOCK:
            //comps.push_back(new Block(s));
            break;
        default:
            break;
        }
    }
    Read(s, &size);
    connections.reserve(size);
    for (int i = 0; i < size; i++) {
        CompIdx idxStart;
        Read(s, &idxStart);
        CompIdx idxEnd;
        Read(s, &idxEnd);

        Connector* start = nullptr;
        Connector* end = nullptr;
        if (idxStart.type == Connector::Type::OUT) {
            start = &comps[idxStart.compIdx]->outConns[0];
        } else {
            start = &comps[idxStart.compIdx]->inConns[idxStart.connIdx];
        }
        if (idxEnd.type == Connector::Type::OUT) {
            end = &comps[idxEnd.compIdx]->outConns[0];
        } else {
            end = &comps[idxEnd.compIdx]->inConns[idxEnd.connIdx];
        }
        connections.push_back(new Line(start, end));
    }
}

void Symulator::WriteProjectData(std::ofstream& s) {
    size_t size = compMenu.size();
    // FIXME: It will change!!!
    Write(s, &size);
    for (size_t i = 8; i < size; i++)
        compMenu[i]->Save(s);

    size = comps.size();
    Write(s, &size);
    for (int i = 0; i < comps.size(); i++)
        comps[i]->Save(s);

    size = connections.size();
    Write(s, &size);
    for (auto& connection : connections) {
        CompIdx idxStart = GetComponentIdx(connections, comps, connection->start);
        Write(s, &idxStart);
        CompIdx idxEnd = GetComponentIdx(connections, comps, connection->end);
        Write(s, &idxEnd);
    }

    Write(s, &compMenuNextX);
    Write(s, &numBlocks);
}

void Symulator::LoadProject() {
    std::ifstream loadFile(name, std::ios_base::binary);
    if (loadFile.is_open()) {
        ReadProjectData(loadFile);

        loadFile.close();
        printf("Load complete\n");
        state = State::ACTIVE;
    }
    else {
        printf("Unable to open file");
    }
}

void Symulator::SaveProject() {
    std::ofstream saveFile(name, std::ios_base::binary);
    if (saveFile.is_open()) {
        WriteProjectData(saveFile);

        saveFile.close();

        printf("Save file created");
        return;
    } else {
        printf("Unable to open file");
        return;
    }
}

void Symulator::Update() {
    if (state == State::ACTIVE) {
        Vector2 pos = GetMousePosition();

        if (CheckCollisionPointRec(pos, {0, 0, 20, 40})) {
            MoveComponentMenu(5.0);
        }
        if (CheckCollisionPointRec(pos, {(float)GetScreenWidth() - 20, 0, 20, 40})) {
            MoveComponentMenu(-5.0);
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (IsKeyDown(KEY_LEFT_CONTROL)) {
                Connector *conn = CheckComponentEndpoints(pos);
                if (conn) {
                    DeleteConnection(conn);
                } else {
                    Component *comp = CheckComponents(pos);
                    if (comp) {
                        DeleteComponent(comp);
                    }
                }
                return;
            }

            MenuButton *menu = CheckMenu(pos);
            if (menu) {
                switch (menu->option) {
                case MenuOption::CREATE:
                    printf("Create selected\n");
                    blockDialog.Show(Dialog::Type::CREATE_BLOCK);
                    break;
                case MenuOption::SAVE:
                    printf("Save selected\n");
                    SaveProject();
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
            Component *in = CheckInputs(pos);
            if (in && in->type == Component::Type::INPUT1) {
                in->outConns[0].value = !in->outConns[0].value;
            } else {
                Connector *conn = CheckInputConnectors(pos);
                if (conn)
                    conn->value = !conn->value;
            }
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Log(TextFormat("x:%.0f y:%.0f", pos.x, pos.y));

            Component *comp = CheckComponentMenu(pos);
            if (comp) {
                movingComp = Component::Clone(comp);
                comps.push_back(movingComp);
                state = State::GATE_MOVING;

                printf("%s\n", movingComp->text.c_str());
            } else if ((comp = CheckComponents(pos)) != nullptr) {
                lineStart = comp->CheckEndpoints(pos);
                if (lineStart) {
                    state = State::LINE_DRAWING;

                    printf("Line start\n");
                } else {
                    movingComp = comp;
                    state = State::GATE_MOVING;

                    printf("%s\n", movingComp->text.c_str());
                }
            }
        }
    } else if (state == State::GATE_MOVING) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            movingComp->Move(GetMouseDelta());
            movingComp->collide = ComponentCollide(movingComp);
        } else {
            if (movingComp->collide) {
                if (movingComp->prevPos.x >= 0 && movingComp->prevPos.y >= 0) {
                    Vector2 delta = {movingComp->prevPos.x - movingComp->rect.x,
                                     movingComp->prevPos.y - movingComp->rect.y};
                    movingComp->Move(delta);
                    movingComp->collide = false;
                } else {
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
    } else if (state == State::LINE_DRAWING) {
        Vector2 pos = GetMousePosition();
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            DrawLineEx(lineStart->pos, pos, 3.0, RAYWHITE);
        } else {
            Connector* conn = CheckComponentEndpoints(pos);
            if (conn) {
                AddConnection(connections, lineStart, conn);
            }
            state = State::ACTIVE;
            printf("Line finished\n");
        }
    } else if (state == State::MENU && blockDialog.type == Dialog::Type::NONE) {
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            Vector2 pos = GetMousePosition();
            MenuButton *menu = CheckMenu(pos);
            if (menu) {
                switch (menu->option) {
                case MenuOption::NEW:
                    printf("New selected\n");
                    blockDialog.Show(Dialog::Type::NEW);
                    break;
                case MenuOption::LOAD:
                    printf("Load selected\n");
                    blockDialog.Show(Dialog::Type::LOAD);
                    break;
                }
            }
        }
        mainMenu.Update();
        
    }
    UpdateConnections(comps, connections);
    menu.Update();
}

void Symulator::Draw() {
    BeginDrawing();

    ClearBackground(GetColor(0x052c46ff));

    if (state == State::MENU) {
        if (blockDialog.type != Dialog::Type::NONE)
            blockDialog.Draw();
        else
            mainMenu.Draw();
    } else {
        if (blockDialog.type == Dialog::Type::CREATE_BLOCK) {
            blockDialog.Draw();
        } else {
            DrawPanel();
            DrawConnections();
            DrawComponents();
            DrawText("Symulator v 1.0", GetScreenWidth() - 100, GetScreenHeight() - 20, 10, RAYWHITE);
        }
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