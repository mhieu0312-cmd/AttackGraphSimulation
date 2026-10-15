#include "LoadDataset.h"

#include <climits>
#include <fstream>
#include <stdexcept>
#include <vector>

using namespace std;

namespace {
// Object dung keys va values; array chi dung values.
// Kieu: o = object, a = array, s = string, i = number, b = boolean, n = null.
struct JsonValue {
    char type = 'n';
    string text;
    vector<string> keys;
    vector<JsonValue> values;
};

class JsonReader {
    istream& input;
    int position = 0;

    void error(const string& message) const {
        throw invalid_argument("JSON tai vi tri " + to_string(position) + ": " + message);
    }

    int take() {
        int character = input.get();
        if (character == EOF) {
            error("File ket thuc khi chua doc xong");
        }
        ++position;
        return character;
    }

    void skipSpaces() {
        while (input.peek() == ' ' || input.peek() == '\t' ||
               input.peek() == '\r' || input.peek() == '\n') {
            take();
        }
    }

    void expect(char expected) {
        skipSpaces();
        if (take() != expected) {
            error(string("Can ky tu '") + expected + "'");
        }
    }

    bool digit(int character) const {
        return character >= '0' && character <= '9';
    }

    int readHex() {
        int value = 0;
        for (int i = 0; i < 4; ++i) {
            int character = take();
            int number;
            if (character >= '0' && character <= '9') {
                number = character - '0';
            } else if (character >= 'a' && character <= 'f') {
                number = character - 'a' + 10;
            } else if (character >= 'A' && character <= 'F') {
                number = character - 'A' + 10;
            } else {
                error("Ma Unicode khong hop le");
                return 0;
            }
            value = value * 16 + number;
        }
        return value;
    }

    void appendUnicode(string& text, int code) {
        if (code <= 0x7F) {
            text += char(code);
        } else if (code <= 0x7FF) {
            text += char(0xC0 | (code >> 6));
            text += char(0x80 | (code & 0x3F));
        } else if (code <= 0xFFFF) {
            text += char(0xE0 | (code >> 12));
            text += char(0x80 | ((code >> 6) & 0x3F));
            text += char(0x80 | (code & 0x3F));
        } else {
            text += char(0xF0 | (code >> 18));
            text += char(0x80 | ((code >> 12) & 0x3F));
            text += char(0x80 | ((code >> 6) & 0x3F));
            text += char(0x80 | (code & 0x3F));
        }
    }

    string readString() {
        expect('"');
        string result;
        while (true) {
            int character = take();
            if (character == '"') {
                return result;
            }
            if (character < 32) {
                error("Ky tu dieu khien trong chuoi");
            }
            if (character == '\\') {
                character = take();
                if (character == '"' || character == '\\' || character == '/') {
                    result += char(character);
                } else if (character == 'b') {
                    result += '\b';
                } else if (character == 'f') {
                    result += '\f';
                } else if (character == 'n') {
                    result += '\n';
                } else if (character == 'r') {
                    result += '\r';
                } else if (character == 't') {
                    result += '\t';
                } else if (character == 'u') {
                    int code = readHex();
                    if (code >= 0xD800 && code <= 0xDBFF) {
                        if (take() != '\\' || take() != 'u') {
                            error("Thieu nua sau cua ma Unicode");
                        }
                        int low = readHex();
                        if (low < 0xDC00 || low > 0xDFFF) {
                            error("Cap ma Unicode khong hop le");
                        }
                        code = 0x10000 + (code - 0xD800) * 1024 + low - 0xDC00;
                    } else if (code >= 0xDC00 && code <= 0xDFFF) {
                        error("Ma Unicode khong co nua dau");
                    }
                    appendUnicode(result, code);
                } else {
                    error("Ky tu escape khong hop le");
                }
            } else if (character < 128) {
                result += char(character);
            } else {
                // Kiem tra chuoi UTF-8, giu nguyen ten tieng Viet.
                int count = 0, code = 0, minimum = 0;
                if (character >= 0xC2 && character <= 0xDF) {
                    count = 1; code = character & 0x1F; minimum = 0x80;
                } else if (character >= 0xE0 && character <= 0xEF) {
                    count = 2; code = character & 0x0F; minimum = 0x800;
                } else if (character >= 0xF0 && character <= 0xF4) {
                    count = 3; code = character & 0x07; minimum = 0x10000;
                } else {
                    error("Chuoi UTF-8 khong hop le");
                }
                for (int i = 0; i < count; ++i) {
                    int next = take();
                    if (next < 0x80 || next > 0xBF) {
                        error("Chuoi UTF-8 khong hop le");
                    }
                    code = code * 64 + (next & 0x3F);
                }
                if (code < minimum || code > 0x10FFFF ||
                    (code >= 0xD800 && code <= 0xDFFF)) {
                    error("Chuoi UTF-8 khong hop le");
                }
                appendUnicode(result, code);
            }
        }
    }

    string readNumber() {
        string result;
        if (input.peek() == '-') {
            result += char(take());
        }
        if (input.peek() == '0') {
            result += char(take());
        } else {
            if (!digit(input.peek())) {
                error("So khong hop le");
            }
            while (digit(input.peek())) {
                result += char(take());
            }
        }
        if (input.peek() == '.') {
            result += char(take());
            if (!digit(input.peek())) {
                error("Thieu phan thap phan");
            }
            while (digit(input.peek())) {
                result += char(take());
            }
        }
        if (input.peek() == 'e' || input.peek() == 'E') {
            result += char(take());
            if (input.peek() == '+' || input.peek() == '-') {
                result += char(take());
            }
            if (!digit(input.peek())) {
                error("Thieu so mu");
            }
            while (digit(input.peek())) {
                result += char(take());
            }
        }
        return result;
    }

    JsonValue readValue(int depth) {
        if (depth > 128) {
            error("JSON long qua nhieu cap");
        }
        skipSpaces();
        JsonValue result;
        int character = input.peek();
        if (character == '{' || character == '[') {
            result.type = character == '{' ? 'o' : 'a';
            char closing = character == '{' ? '}' : ']';
            take();
            skipSpaces();
            if (input.peek() == closing) {
                take();
                return result;
            }
            while (true) {
                if (result.type == 'o') {
                    string key = readString();
                    for (const string& previous : result.keys) {
                        if (previous == key) {
                            error("Key trung: " + key);
                        }
                    }
                    result.keys.push_back(key);
                    expect(':');
                }
                result.values.push_back(readValue(depth + 1));
                skipSpaces();
                if (input.peek() == closing) {
                    take();
                    return result;
                }
                expect(',');
            }
        }
        if (character == '"') {
            result.type = 's';
            result.text = readString();
        } else if (character == '-' || digit(character)) {
            result.type = 'i';
            result.text = readNumber();
        } else {
            string word;
            if (character == 't') word = "true";
            else if (character == 'f') word = "false";
            else if (character == 'n') word = "null";
            else error("Gia tri khong hop le");
            for (char expected : word) {
                if (take() != expected) {
                    error("Gia tri khong hop le");
                }
            }
            result.type = word == "null" ? 'n' : 'b';
            result.text = word;
        }
        return result;
    }

  public:
    explicit JsonReader(istream& stream) : input(stream) {}

    JsonValue read() {
        if (input.peek() == 0xEF) {
            take();
            if (take() != 0xBB || take() != 0xBF) {
                error("UTF-8 BOM khong hop le");
            }
        }
        JsonValue result = readValue(0);
        skipSpaces();
        if (input.bad()) {
            error("Khong doc duoc file");
        }
        if (input.peek() != EOF) {
            error("Du noi dung sau JSON");
        }
        return result;
    }
};

const JsonValue* findField(const JsonValue& object, const char* key) {
    for (int i = 0; i < (int) object.keys.size(); ++i) {
        if (object.keys[i] == key) {
            return &object.values[i];
        }
    }
    return nullptr;
}

const JsonValue& field(const JsonValue& object, const char* key, char type) {
    const JsonValue* value = findField(object, key);
    if (value == nullptr) {
        throw invalid_argument(string("Thieu truong ") + key);
    }
    if (value->type != type) {
        throw invalid_argument(string("Sai kieu du lieu cua ") + key);
    }
    return *value;
}

void validateFields(const JsonValue& object, const vector<string>& allowedFields) {
    if (object.type != 'o') {
        throw invalid_argument("Ban ghi phai la object");
    }
    for (const string& key : object.keys) {
        bool allowed = false;
        for (const string& name : allowedFields) {
            if (key == name) {
                allowed = true;
                break;
            }
        }
        if (!allowed) {
            throw invalid_argument("Truong khong duoc ho tro: " + key);
        }
    }
}

int integerField(const JsonValue& object, const char* key) {
    string number = field(object, key, 'i').text;
    if (number == "-0") {
        return 0;
    }
    int value = 0;
    for (char character : number) {
        if (character < '0' || character > '9') {
            throw invalid_argument(string(key) + " phai la so nguyen khong am");
        }
        int digit = character - '0';
        if (value > (INT_MAX - digit) / 10) {
            throw invalid_argument(string(key) + " vuot mien int");
        }
        value = value * 10 + digit;
    }
    return value;
}
} // namespace

Graph loadDataset(istream& input) {
    Graph graph;
    string location = "root";
    try {
        JsonValue data = JsonReader(input).read();
        validateFields(data, {"nodes", "edges", "metadata"});
        const JsonValue& nodes = field(data, "nodes", 'a');
        const JsonValue& edges = field(data, "edges", 'a');
        if (findField(data, "metadata") != nullptr) {
            field(data, "metadata", 'o');
        }
        for (int i = 0; i < (int) nodes.values.size(); ++i) {
            location = "nodes[" + to_string(i) + "]";
            const JsonValue& node = nodes.values[i];
            validateFields(node, {"id", "name", "type", "assets"});
            int id = integerField(node, "id");
            string name = field(node, "name", 's').text;
            NodeType type = nodeTypeFromString(field(node, "type", 's').text);
            int assets = integerField(node, "assets");
            graph.addNode(Node(id, name, type, assets));
        }
        for (int i = 0; i < (int) edges.values.size(); ++i) {
            location = "edges[" + to_string(i) + "]";
            const JsonValue& edge = edges.values[i];
            validateFields(edge, {"id", "from", "to", "weight", "capacity", "relation", "blocked"});
            int id = integerField(edge, "id");
            int from = integerField(edge, "from");
            int to = integerField(edge, "to");
            int weight = integerField(edge, "weight");
            int capacity = integerField(edge, "capacity");
            string relation = field(edge, "relation", 's').text;
            bool blocked = false;
            if (findField(edge, "blocked") != nullptr) {
                blocked = field(edge, "blocked", 'b').text == "true";
            }
            graph.addEdge(Edge(from, to, weight, relation, blocked, id, capacity));
        }
    } catch (const exception& error) {
        throw invalid_argument("Dataset khong hop le tai " + location + ": " + error.what());
    }
    return graph;
}

Graph loadDataset(const string& filename) {
    ifstream input(filename);
    if (!input) {
        throw runtime_error("Khong mo duoc dataset: " + filename);
    }
    return loadDataset(input);
}
