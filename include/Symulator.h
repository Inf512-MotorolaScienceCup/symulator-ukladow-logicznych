#include <string>
#include <vector>
#include <list>

#include "raylib.h"

namespace sym {

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
        Connector(Component* parent, Vector2 pos, Type type) : parent(parent), pos(pos), type(type) {}
        Connector(Vector2 pos, Type type) : parent(nullptr), pos(pos), type(type) {}
        Connector() : parent(nullptr), pos({ 0, 0 }), type(Type::IN) {}
    };

    class Component {
    public:
        enum class Type {
            INPUT1,
            OUTPUT1,
            GATE,
            INPUT2,
            INPUT4,
            INPUT8,
            BLOCK
        } type;

        static Component* Create(Component* comp);

        Component(float x, float y, float width, float height, const char* text, Type type, bool singleInput = false)
            : rect({ x, y, width, height }), prevPos({ -1, -1 }), text(text), type(type) {
        }

        Component(const Component* comp)
            : rect(comp->rect), prevPos({ -1, -1 }), text(comp->text), type(comp->type),
            inConns(comp->inConns), outConn(comp->outConn) {
            for (auto& in : inConns) {
                in.parent = this;
            }
            outConn.parent = this;
        }
        virtual void Calc(std::vector<Connector*>& outConns) { }
        virtual void Draw();
        virtual void Move(const Vector2& delta);
        virtual Connector* CheckEndpoints(const Vector2& pos);
        virtual ~Component() {};

        Rectangle rect;
        Vector2 prevPos;
        std::string text;
        bool collide = false;
        std::vector<Connector> inConns;
        Connector outConn;
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

        Gate(float x, float y, const char* text, Gate::Type gateType, bool singleInput = false)
            : Component(x, y, WIDTH, HEIGHT, text, Component::Type::GATE, singleInput), gateType(gateType) {
            if (singleInput) {
                inConns.push_back(Connector(this, { x + 5, y + 15 }, Connector::Type::IN));
            }
            else {
                inConns.push_back(Connector(this, { x + 5, y + 5 }, Connector::Type::IN));
                inConns.push_back(Connector(this, { x + 5, y + 25 }, Connector::Type::IN));
            }
            outConn = Connector(this, { x + 70, y + 15 }, Connector::Type::OUT);
        }
        Gate(const Gate* gate) : Component(gate), gateType(gate->gateType) {}
        virtual void Calc(std::vector<Connector*>& outConns) override;
        virtual void Draw() override;
    };

    class Input : public Component {
    public:
        static constexpr float WIDTH = 40;
        static constexpr float HEIGHT = 30;

        Input(float x, float y, const char* text)
            : Component(x, y, WIDTH, HEIGHT, text, Component::Type::INPUT1) {
            outConn = Connector(nullptr, { x + 35, y + 15 }, Connector::Type::OUT);
        }
        Input(const Input* in) : Component(in) {}
        virtual void Draw() override;
    };

    class Output : public Component {
    public:
        static constexpr float WIDTH = 40;
        static constexpr float HEIGHT = 30;

        Output(float x, float y, const char* text)
            : Component(x, y, WIDTH, HEIGHT, text, Component::Type::OUTPUT1) {
            inConns.push_back(Connector(nullptr, { x + 5, y + 15 }, Connector::Type::IN));
        }
        Output(const Output* out) : Component(out) {}
        virtual void Draw() override;
    };

    class BlockConnector : public Connector {
    public:
        BlockConnector(Component* parent, Vector2 pos, Type type, Connector* conn)
            : Connector(parent, pos, type), conn(conn) {}

        Connector* conn;
    };

    class Block : public Component {
    public:
        static constexpr float WIDTH = 40;
        static constexpr float HEIGHT = 30;

        Block(float x, float y, const char* text, Color color, std::vector<Component*> comps, std::vector<Line*> connections);
        Block(const Block* block);
        ~Block();
        virtual void Calc(std::vector<Connector*>&) override;
        virtual void Move(const Vector2& delta) override;
        virtual void Draw() override;
        virtual Connector* CheckEndpoints(const Vector2& pos) override;

        std::vector<Component*> comps;
        std::vector<Line*> connections;

        std::vector<BlockConnector> inputs;
        std::vector<BlockConnector> outputs;

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
    };

    enum class MenuOption { CREATE, SAVE, CLEAR, OPTIONS };

    class MenuButton {
    public:
        const char* text;
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
                Rectangle rec = { x, y, width, height };
                buttons.push_back(MenuButton(menuOptions[i], rec));
            }
        }
        void Update();
        void Draw();

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

    class CreateBlockDialog {
    public:
        CreateBlockDialog(Symulator* parent) : parent(parent) {}
        void Draw();
        void Show();

        Symulator* parent;
        int result;
        char name[256] = "BLK1";
        bool show = false;
        Color color = GREEN;
    };

    class Symulator {
    public:
        enum class State {
            ACTIVE,
            GATE_MOVING,
            LINE_DRAWING
        };

        Symulator() : blockDialog(this) {}
        ~Symulator();

        static void Log(const char* text);

        void CreateComponentMenu();
        void CreateBlock(const char* name, Color color);

        Component* CheckComponentMenu(const Vector2& pos);
        Component* CheckComponents(const Vector2& pos);
        Component* CheckInputs(const Vector2& pos);
        Connector* CheckComponentEndpoints(Component* comp, const Vector2& pos);
        Connector* CheckComponentEndpoints(const Vector2& pos);
        MenuButton* CheckMenu(const Vector2& pos);

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

        void Update();
        void Draw();
        int MainLoop();

        State state = State::ACTIVE;

        std::vector<Component*> compMenu;
        std::vector<Component*> comps;
        std::vector<Line*> connections;

        Component* movingComp;
        Connector* lineStart;

        MenuPanel menu;
        float compMenuX;
        int numBlocks = 0;
        CreateBlockDialog blockDialog;
    };

} // namespace sym