#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    
    Node() = default;
    Node(std::nullptr_t);
    Node(Array);
    Node(Dict);
    Node(bool);
    Node(int);
    Node(double);
    Node(std::string);
    
    const Value& GetValue() const { return value_; }
    
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;
    bool IsBool() const;
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsString() const;
    
    //Ниже перечислены методы, которые возвращают хранящееся внутри Node значение заданного типа.
    //Если внутри содержится значение другого типа, должно выбрасываться исключение std::logic_error.
    int AsInt() const;
    bool AsBool() const;
    // Возвращает значение типа double, если внутри хранится double либо int.
    // В последнем случае возвращается приведенное в double значение.
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    
    bool operator==(const Node& other)const;
    bool operator!=(const Node& other)const;
    
private:
    Value value_;
};
    

class Document {
public:
    explicit Document(Node root);
    const Node& GetRoot() const;
private:
    Node root_;
};
    
bool operator==(const Document& lhs, const Document& rhs);
    
std::ostream& operator<<(std::ostream& out, const Node& node); 

Document Load(std::istream& input);  

void Print(const Document& doc, std::ostream& output);

}  // namespace json
