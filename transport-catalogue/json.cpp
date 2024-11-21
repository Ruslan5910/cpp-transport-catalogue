#include "json.h"
#include <sstream>

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;
    char c;
    
    for (; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (c != ']') {
        throw json::ParsingError("error ]");
    }
    return Node(move(result));
}

Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(std::stoi(parsed_num));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(std::stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }
    return Node(s);
} 

Node LoadDict(istream& input) {
    Dict result;
    
    char c;
    
    for (; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    if (c != '}') {
        throw json::ParsingError("error ]");
    }

    return Node(move(result));
}
    
Node LoadInt(const auto& x) {
    return Node(get<int>(x));
}
    
Node LoadDouble(const auto& x) {
    return Node(get<double>(x));
}
    
Node LoadNull(istream& in) {
    string n = "null";
    char c;
    for (size_t i = 0; i < n.size(); ++i) {
        c = in.get();
        if (c == n[i]) {
            continue;
        } else {
            throw json::ParsingError("The type does not match the expected null");
        }
    }
    char next = in.peek();    
    if (next != EOF && next != ' ' && next != ',' && next != ']' && next != '}' && next != '\n' && next != '\t') {
        throw json::ParsingError("The type does not match the expected null");
    }
    return Node(nullptr);
}
    
Node LoadBoolTrue(istream& in) {
    string t = "true";
    char c;
    for (size_t i = 0; i < t.size(); ++i) {
        c = in.get();
        if (c == t[i]) {
            continue;
        } else {
            throw json::ParsingError("The type does not match the expected true");
        }
    }
    char next = in.peek();    
    if (next != EOF && next != ' ' && next != ',' && next != ']' && next != '}' && next != '\n' && next != '\t') {
        throw json::ParsingError("The type does not match the expected true");    
    }
    return Node(true);
}
    
Node LoadBoolFalse(istream& in) {
    string t = "false";
    char c;
    for (size_t i = 0; i < t.size(); ++i) {
        c = in.get();
        if (c == t[i]) {
            continue;
        } else {
            throw json::ParsingError("The type does not match the expected false");
        }
    }
    char next = in.peek();    
    if (next != EOF && next != ' ' && next != ',' && next != ']' && next != '}' && next != '\n' && next != '\t') {
        throw json::ParsingError("The type does not match the expected false");    
    }
    return Node(false);
}
 
    
    
Node LoadNode(istream& input) {
    char c;
    input >> c;
    if (c == '}') {
        throw json::ParsingError("error {");
    }
    if (c == ']') {
        throw json::ParsingError("error ]");
    }
    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == 't') {
        input.putback(c);
        return LoadBoolTrue(input);
    } else if (c == 'f') {
        input.putback(c);
        return LoadBoolFalse(input);
    } else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    } else {
        input.putback(c);
        return LoadNumber(input);
    }
}

}  // namespace
    
//----------------Node-------------------------------------
    
Node::Node(std::nullptr_t value) 
    : value_(value) { 
} 
Node::Node(int value) 
    : value_(value) { 
} 
Node::Node(bool value) 
    : value_(value) { 
}     
Node::Node(double value) 
    : value_(value) { 
}         
Node::Node(string str) 
    : value_(move(str)) { 
}     
Node::Node(Array array) 
    : value_(move(array)) { 
} 
Node::Node(Dict map) 
    : value_(move(map)) { 
}

bool Node::IsNull() const {
    return holds_alternative<nullptr_t>(value_);
}
bool Node::IsArray() const {
    return holds_alternative<Array>(value_);
}
bool Node::IsMap() const {
    return holds_alternative<Dict>(value_);
}            
bool Node::IsBool() const {
    return holds_alternative<bool>(value_);
}
bool Node::IsInt() const {
    return holds_alternative<int>(value_);
}
bool Node::IsDouble() const {
    return holds_alternative<double>(value_) || IsInt();
}
bool Node::IsPureDouble() const {
    return holds_alternative<double>(value_);
}
bool Node::IsString() const {
    return holds_alternative<std::string>(value_);
}
    
int Node::AsInt() const {
    if (!IsInt()) {
        throw logic_error("The type does not match the expected int"s);
    }
    return get<int>(value_);
}
bool Node::AsBool() const {
    if (!IsBool()) {
        throw logic_error("The type does not match the expected bool"s);
    }
    return get<bool>(value_);
}

double Node::AsDouble() const {
    if (IsInt()) {
        return get<int>(value_);
    } else if (IsDouble()) {
        return get<double>(value_);
    } else {
        throw logic_error("The type does not match the expected int or double"s);
    }
}
const string& Node::AsString() const {
    if (!IsString()) {
        throw std::logic_error("The type does not match the expected string"s);
    }
    return get<string>(value_);
}
const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw logic_error("The type does not match the expected Array"s);
    }
    return get<Array>(value_);
}
const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw logic_error("The type does not match the expected map");
    }
    return get<Dict>(value_);
}
    
bool Node::operator==(const Node& other) const {
    return value_ == other.GetValue();
}
bool Node::operator!=(const Node& other) const {
    return !(*this == other);
}
    
//------------------document----------------------------------
Document::Document(Node root)
    : root_(move(root)) {
}
    
const Node& Document::GetRoot() const {
    return root_;
}
    
//-------------------------------------------

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();   
}

ostream& operator<<(ostream& out, const Node& node);
    
struct PrintNode {
    ostream& out;
    void operator()(nullptr_t) const {
        out << "null";
    }
    void operator()(const Array& array) const {
        out << "[";
        for (size_t i = 0; i < array.size(); ++i) {
            if (i != array.size() - 1) {
                out << array[i] << ", ";
            } else {
                out << array[i];
            }
        }
        out << "]";
    }
    void operator()(const Dict& dict) const { 
        bool is_first = true;
        out << "{";
        for (auto [key, value] : dict) {
            if (is_first) {
                out << "\"" << key << "\": " << value;
                is_first = false;
            } else if (!is_first) {
                out << ", " << "\"" << key << "\": " << value;
            }
        }
        out << "}";
    }
    void operator()(bool b) const {
        if (b == true) {
            out << "true";
        } else {
            out << "false";
        }
    }
    void operator()(int x) const {
        out << x;
    }
    void operator()(double x) const {
        out << x;
    }
    void operator()(const string& str) const {
        string result;
        char c;
        out << "\"";
        istringstream ss(str);
        while (ss.get(c)) {
            if (c == '\r') {
                result += "\\r";
            } else if (c == '\t') {
                result += "\\t";
            } else if (c == '\n') {
                result += "\\n";
            } else if (c == '"') {
                result += "\\\"";
            } else if (c == '\\') {
                result += "\\\\";
            } else {
            result.push_back(c);
            }
        }
    out << result;
    out << "\"";
    }
};
    
ostream& operator<<(ostream& out, const Node& node) {
    visit(PrintNode{out}, node.GetValue());
    return out;
}
    
void Print(const Document& doc, ostream& output) {
    output << doc.GetRoot();
}

}  // namespace json
