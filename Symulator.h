#include <string>
#include <vector>
#include <list>
#include <fstream>

#include "raylib.h"

namespace sym {
template <typename T>
void Write(std::ofstream& os, T* data) {
    os.write((const char*)data, sizeof(T));
}
void Write(std::ofstream &os, std::string *data);

template <typename T>
void Read(std::ifstream& is, T* data) {
    char buffer[20];
    is.read(buffer, sizeof(T));
    *data = *(T*)buffer;
}
void Read(std::ifstream& is, std::string* data);

class Gate;
class Component;
class Line;
class Symulator;

class Connector {
public:
    enum class Type {
        IN,
        OUT,
    } type;
    Vector2 pos;
    bool value = false;
    Component* parent;
    Connector* conn; // bypass

    Connector(Component* parent, Vector2 pos, Type type): parent(parent), pos(pos), type(type), conn(nullptr) {}
    Connector(Component *parent, Vector2 pos, Type type, Connector* conn)
        : parent(parent), pos(pos), type(type), conn(conn) {}
    Connector(Vector2 pos, Type type) : parent(nullptr), pos(pos), type(type), conn(nullptr) {}
    Connector() : parent(nullptr), pos({0, 0}), type(Type::IN), conn(nullptr) {}
    Connector(std::ifstream& s, Component* parent): parent(parent) {
        Read(s, &type);
        Read(s, &pos);
        Read(s, &value);

        printf("Component::Create type:%d pos:(%f %f)\n", type, pos.x, pos.y);
    }
    void Save(std::ofstream& s) {
        Write(s, &type);
        Write(s, &pos);
        Write(s, &value);

        printf("Component::Save type:%d pos:(%f %f)\n", type, pos.x, pos.y);
    }
};

class Component {
public:
    enum class Type {
        INPUT1,
        INPUT2,
        INPUT4,
        INPUT8,
        OUTPUT1,
        OUTPUT2,
        OUTPUT4,
        OUTPUT8,
        GATE,
        BLOCK
    } type;

    static Component *Clone(Component *comp);

    Component(float x, float y, float width, float height, const char *text, Type type, bool singleInput = false)
        : rect({x, y, width, height}), prevPos({-1, -1}), text(text), type(type) {
    }

    Component(const Component *comp)
        : rect(comp->rect), prevPos({-1, -1}), text(comp->text), type(comp->type),
          inConns(comp->inConns), outConns(comp->outConns) {
        for (auto& in : inConns)
            in.parent = this;
        for (auto& out : outConns)
            out.parent = this;
    }
    Component() {}
    Component(std::ifstream& s, Type type): prevPos({ -1, -1 }), type(type) {
        Read(s, &rect);
        Read(s, &text);
    }

    virtual void Calc(std::vector<Connector*>& outConns) { }
    virtual void Draw();
    virtual void Move(const Vector2 &delta);
    virtual Connector* CheckEndpoints(const Vector2& pos);
    virtual void Save(std::ofstream&) {}
    virtual ~Component() {};

    Rectangle rect;
    Vector2 prevPos;
    std::string text;
    bool collide = false;
    std::vector<Connector> inConns;
    std::vector<Connector> outConns;
};

class Gate : public Component {
public:
    enum class Type {
        NOT,
        AND,
        OR,
        XOR
    } gateType;

    static constexpr float WIDTH = 75;
    static constexpr float HEIGHT = 30;

    Gate(float x, float y, const char *text, Gate::Type gateType, bool singleInput = false)
        : Component(x, y, WIDTH, HEIGHT, text, Component::Type::GATE, singleInput), gateType(gateType) {
        if (singleInput) {
            inConns.push_back(Connector(this, {x + 5, y + 15}, Connector::Type::IN));
        } else {
            inConns.push_back(Connector(this, {x + 5, y + 5}, Connector::Type::IN));
            inConns.push_back(Connector(this, {x + 5, y + 25}, Connector::Type::IN));
        }
        outConns.push_back(Connector(this, {x + 70, y + 15}, Connector::Type::OUT));
    }
    Gate(const Gate* gate) : Component(gate), gateType(gate->gateType) {}
    Gate(std::ifstream&, Component::Type type);
    virtual void Calc(std::vector<Connector*>& outConns) override;
    virtual void Draw() override;
    virtual void Save(std::ofstream& s) override;
};

class Input : public Component {
public:
    static constexpr float WIDTH = 40;
    static constexpr float HEIGHT = 30;
    static char nextText;

    Input(float x, float y, const char* text)
        : Component(x, y, WIDTH, HEIGHT, text, Component::Type::INPUT1) {
        outConns.push_back(Connector(nullptr, {x + 35, y + 15}, Connector::Type::OUT));
    }
    Input(const Input* in) : Component(in) { text = nextText++; }
    Input(std::ifstream& s, Component::Type type);
    virtual void Draw() override;
    virtual void Save(std::ofstream& s) override;
};

class InputBlock : public Component {
public:
    static constexpr float WIDTH = 40;
    static constexpr float HEIGHT = 30;
    static char nextText;

    InputBlock(float x, float y, Component::Type type, const char *text)
        : Component(x, y, WIDTH, HEIGHT, text, type) {

        outConns.push_back(Connector(nullptr, {x + WIDTH - 5, y + 15}, Connector::Type::OUT));
    }
    InputBlock(const InputBlock *in) : Component(in), isIcon(false) {

        int numConnectors = 0;
        switch (type) {
        case Component::Type::INPUT2:
            numConnectors = 2;
            break;
        case Component::Type::INPUT4:
            numConnectors = 4;
            break;
        case Component::Type::INPUT8:
            numConnectors = 8;
            break;
        }

        for (int i = 1; i < numConnectors; i++) {
            outConns.push_back(Connector(nullptr, {rect.x + WIDTH - 5, rect.y + 15 + 15 * i}, Connector::Type::OUT));
        }
        char name = nextText++;
        rect.height = 15 + 15 * outConns.size();
    }
    InputBlock(std::ifstream&, Type type);
    virtual void Draw() override;
    virtual void Save(std::ofstream& s) override;

    bool isIcon = true;
};

class Output : public Component {
public:
    static constexpr float WIDTH = 40;
    static constexpr float HEIGHT = 30;

    Output(float x, float y, const char *text)
        : Component(x, y, WIDTH, HEIGHT, text, Component::Type::OUTPUT1) {
        inConns.push_back(Connector(nullptr, {x + 5, y + 15}, Connector::Type::IN));
    }
    Output(const Output* out) : Component(out) {}
    Output(std::ifstream&, Type type);
    virtual void Draw() override;
    virtual void Save(std::ofstream& s) override;
};

class OutputBlock : public Component {
  public:
    static constexpr float WIDTH = 40;
    static constexpr float HEIGHT = 30;

    OutputBlock(float x, float y, Component::Type type, const char *text)
        : Component(x, y, WIDTH, HEIGHT, text, type) {
        // Only one - because only this is needed in menu
        inConns.push_back(Connector(nullptr, {x + 5, y + 15}, Connector::Type::IN));
    }
    OutputBlock(const OutputBlock *out) : Component(out), isIcon(false) {
        int numConnectors = 0;
        switch (type) {
        case Component::Type::OUTPUT2:
            numConnectors = 2;
            break;
        case Component::Type::OUTPUT4:
            numConnectors = 4;
            break;
        case Component::Type::OUTPUT8:
            numConnectors = 8;
            break;
        }

        // First already added
        for (int i = 1; i < numConnectors; i++) {
            inConns.push_back(Connector(nullptr, {rect.x + 5, rect.y + 15 + 15 * i}, Connector::Type::IN));
        }
        rect.height = 15 + 15 * inConns.size();
    }
    OutputBlock(std::ifstream&, Type type);
    virtual void Draw() override;
    virtual void Save(std::ofstream&) override;
    bool isIcon = true;
};

class BlockConnector : public Connector {
public:
    BlockConnector(Component *parent, Vector2 pos, Type type, Connector* conn)
        : Connector(parent, pos, type), conn(conn) {}
    BlockConnector(std::ifstream& s, Component* parent, Connector* conn) {}

    void Save(std::ofstream& s) {
        Write(s, &type);
        Write(s, &pos);
    }
    Connector *conn;
};

class Block : public Component {
public:
    static constexpr float WIDTH = 40;
    static constexpr float HEIGHT = 30;

    Block(float x, float y, const char *text, Color color, std::vector<Component*> comps, std::vector<Line*> connections);
    Block(const Block *block);
    Block(std::ifstream& s, Component::Type type);
    ~Block();
    virtual void Calc(std::vector<Connector*>&) override;
    virtual void Move(const Vector2& delta) override;
    virtual void Draw() override;
    virtual Connector *CheckEndpoints(const Vector2 &pos) override;
    virtual void Save(std::ofstream&) override;

    std::vector<Component*> comps;
    std::vector<Line*> connections;

    // std::vector<BlockConnector> inputs;
    // std::vector<BlockConnector> outputs;

    Color color;
    bool isIcon = true;
    int numInputs;
    int numOutputs;
};

class Line {
public:
    Connector* start;
    Connector* end;
    Line(Connector* start, Connector* end) : start(start), end(end) {}
/*
    Line(std::ifstream& s) {
        start = new Connector(s);
        end = ;
    }
    void Save(std::ofstream& s) {
        start->Save(s);
        end->Save(s);
    }
    */
};

enum class MenuOption { CREATE, SAVE, CLEAR, OPTIONS, NEW, LOAD };

class MenuButton {
public:
    const char *text;
    MenuOption option;
    Rectangle rec;
    MenuButton(MenuOption option, Rectangle rec)
        : option(option), rec(rec) {
        switch (option) {
        case MenuOption::CREATE:
            text = "Create";
            break;
        case MenuOption::SAVE:
            text = "Save";
            break;
        case MenuOption::CLEAR:
            text = "Clear";
            break;
        case MenuOption::OPTIONS:
            text = "Options";
            break;
        case MenuOption::NEW:
            text = "New project";
            break;
        case MenuOption::LOAD:
            text = "Load project";
            break;
        }
    }
};

class MenuPanel {
public:
    MenuPanel() {
        float offset = 10;
        float width = 100;
        float height = 20;
        int initialScreenHeight = 600;
        for (int i = 0; i < menuOptions.size(); i++) {
            float y = initialScreenHeight - 40 + offset;
            float x = 40 + offset + i * (offset + width);
            Rectangle rec = {x, y, width, height};
            buttons.push_back(MenuButton(menuOptions[i], rec));
        }
    }
    void Update();
    void Draw();
    void Save(std::ofstream&);

    std::vector<MenuButton> buttons;
    std::vector<MenuOption> menuOptions = {
        MenuOption::CREATE,
        MenuOption::SAVE,
        MenuOption::CLEAR,
        MenuOption::OPTIONS
    };
    MenuButton* selected = nullptr;
    float yOffset = 10;
};

class MainMenu {
public:
    MainMenu() {
        float width = 300;
        float height = 100;
        float k = -1;
        for (int i = 0; i < menuOptions.size(); i++) {
            float y = GetScreenHeight() / 2 + k * 100;
            float x = GetScreenWidth() / 2 - width / 2;
            Rectangle rec = { x, y, width, height };
            buttons.push_back(MenuButton(menuOptions[i], rec));
            k += 2;
        }
    }
    void Update();
    void Draw();
    void Save(std::ofstream&);

    std::vector<MenuButton> buttons;
    std::vector<MenuOption> menuOptions = {
        MenuOption::NEW,
        MenuOption::LOAD
    };
    MenuButton* selected = nullptr;
};

class Dialog {
public:
    enum class Type {
        CREATE_BLOCK,
        NEW,
        LOAD,
        NONE
    };
    Dialog(Symulator* parent): parent(parent) {}
    void Draw();
    void Show(Type);

    Symulator* parent;
    Type type = Type::NONE;
    int result;
    char blockName[256] = "BLK1";
    char projectName[256] = "Project1";
    bool cooldown;
    Color color = GREEN;
};

class Symulator {
public:
    enum class State {
        ACTIVE,
        GATE_MOVING,
        LINE_DRAWING,
        MENU
    };

    Symulator(): blockDialog(this) {}
    ~Symulator();

    static void Log(const char *text);

    void CreateComponentMenu();
    void CreateBlock(const char* name, Color color);

    Component* CheckComponentMenu(const Vector2& pos);
    Component* CheckComponents(const Vector2& pos);
    Component* CheckInputs(const Vector2 &pos);
    Connector* CheckInputConnectors(const Vector2 &pos);
    Connector* CheckComponentEndpoints(Component* comp, const Vector2 &pos);
    Connector* CheckComponentEndpoints(const Vector2 &pos);
    MenuButton* CheckMenu(const Vector2 &pos);

    bool ComponentCollide(Component* comp);

    void DeleteConnection(Connector* conn);
    void DeleteComponent(Component* comp);
    void DeleteBlock(Block* comp);
    void DeleteAll();

    void DrawComponentMenu();
    void DrawPanel();
    void DrawComponents();
    void DrawConnections();

    void MoveComponentMenu(float delta);

    void ReadProjectData(std::ifstream&);
    void WriteProjectData(std::ofstream&);
    void LoadProject();
    void SaveProject();

    void Update();
    void Draw();
    int MainLoop();

    State state = State::MENU;

    std::vector<Component*> compMenu;
    std::vector<Component*> comps;
    std::vector<Line*> connections;

    Component* movingComp;
    Connector* lineStart;

    MenuPanel menu;
    MainMenu mainMenu;
    float compMenuNextX;
    int numBlocks = 0;
    Dialog blockDialog;
    std::string name;
};

} // namespace sym
