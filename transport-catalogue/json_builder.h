#pragma once

#include "json.h"

#include <utility>
#include <optional>
#include <string>
#include <vector>
#include <iostream>

namespace json {
    
using namespace std::literals;

class DictItemContext;
class KeyItemContext;
class ArrayItemContext;
    
class Builder {
public:
    DictItemContext StartDict();
    
    Builder& EndDict();
    
    ArrayItemContext StartArray();
    
    Builder& EndArray();
    
    KeyItemContext Key(const std::string& key);
    
    Builder& Value(const Node::Value& value);
    
    Node Build();
    
    Node MakeNodeFromValue(Node::Value value) const;
    
protected:
    Node root_;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> key_;
    size_t counter = 0;
};
    
    
class ItemContext : public Builder {
public:
    ItemContext(Builder& builder)
        : builder_(builder) {}
protected:
    Builder& builder_;
};

    
class KeyItemContext : public ItemContext {
public:
    KeyItemContext(Builder& builder)
        : ItemContext(builder) {}
    
    Node Build() = delete;
    Builder& EndDict() = delete;
    Builder& EndArray() = delete;
    KeyItemContext Key(const std::string& key) = delete;
    DictItemContext Value(const Node::Value& value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
};
    
class DictItemContext : public ItemContext {
public:
    DictItemContext(Builder& builder)
        : ItemContext(builder) {}
    
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    DictItemContext Value(const Node::Value& value) = delete;
    Builder& EndArray() = delete;
    Node Build() = delete;
    KeyItemContext Key(const std::string& key);
    Builder& EndDict();
};
    
class ArrayItemContext : public ItemContext {
public:  
    ArrayItemContext(Builder& builder)
        : ItemContext(builder) {}
    
    Builder& EndDict() = delete;
    Node Build() = delete;
    KeyItemContext Key(const std::string& key) = delete;
    ArrayItemContext Value(const Node::Value& value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
};
    
} //namespace json
