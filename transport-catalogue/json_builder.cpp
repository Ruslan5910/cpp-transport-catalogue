#include "json_builder.h"


namespace json {

DictItemContext Builder::StartDict() {
    if (nodes_stack_.empty() && counter > 0) {
        throw std::logic_error("The described object is ready"s);
    }
    if (nodes_stack_.size() == 0) {
        root_.GetValue() = Dict();
        ++counter;
        nodes_stack_.push_back(&root_);
    } else if (nodes_stack_.back()->IsDict()) {
        if (!key_) {
            throw std::logic_error("Do not have a key for create dictionary"s);
        }
        auto& last_complex_node = nodes_stack_.back()->AsDict();    
        last_complex_node.insert({*key_, MakeNodeFromValue(Dict())});
        nodes_stack_.push_back(&last_complex_node.at(*key_));
        key_ = std::nullopt;
        ++counter;
    } else if (nodes_stack_.back()->IsArray()) {
        auto& last_complex_node = nodes_stack_.back()->AsArray();    
        last_complex_node.push_back(MakeNodeFromValue(Dict()));
        nodes_stack_.push_back(&last_complex_node.back());
        ++counter;
    }
    return *this;
}
    
Builder& Builder::EndDict() {
    if (nodes_stack_.empty() && counter > 0) {
        throw std::logic_error("The described object is ready"s);
    }
    if (nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Trying to close dictionary"s);
    }
    if (nodes_stack_.back()->IsDict()) {
        nodes_stack_.pop_back();
    }
    return *this;
}
    
ArrayItemContext Builder::StartArray() {
    if (nodes_stack_.empty() && counter > 0) {
        throw std::logic_error("The described object is ready"s);
    }
    if (nodes_stack_.size() == 0) {
        root_.GetValue() = Array();
        ++counter;
        nodes_stack_.push_back(&root_);
    } else if (nodes_stack_.back()->IsDict()) {
        
        auto& last_complex_node = nodes_stack_.back()->AsDict();    
        last_complex_node.insert({*key_, MakeNodeFromValue(Array())});
        nodes_stack_.push_back(&last_complex_node.at(*key_));
        key_ = std::nullopt;
        ++counter;
    } else if (nodes_stack_.back()->IsArray()) {
        auto& last_complex_node = nodes_stack_.back()->AsArray();    
        last_complex_node.push_back(MakeNodeFromValue(Array()));
        nodes_stack_.push_back(&last_complex_node.back());    
        ++counter;
    }
    return *this;
}
    
Builder& Builder::EndArray() {
    if (nodes_stack_.empty() && counter > 0) {
        throw std::logic_error("The described object is ready"s);
    }
    if (nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Trying to close array container"s);
    }
    if (nodes_stack_.back()->IsArray()) {
        nodes_stack_.pop_back();
    }
    return *this;
}
    
KeyItemContext Builder::Key(const std::string& key) {
    if (nodes_stack_.empty() && counter > 0) {
        throw std::logic_error("The described object is ready"s);
    }
    if (!nodes_stack_.back()->IsDict() || !(key_ == std::nullopt)) {
        throw std::logic_error("Incorrect call chain. Key called outside the dictionary or after Key command"s);
    }
    if (nodes_stack_.back()->IsDict() && !key_) {
        key_ = key;
    }
    return *this;
}
    
Builder& Builder::Value(const Node::Value& value) {
    if (nodes_stack_.empty() && counter > 0) {
        throw std::logic_error("The described object is ready"s);
    }
    if (nodes_stack_.size() == 0) {
        ++counter;
        root_.GetValue() = value;
    } else if (nodes_stack_.back()->IsArray()) {
        nodes_stack_.back()->AsArray().push_back(MakeNodeFromValue(value));
        ++counter;
    } else if (nodes_stack_.back()->IsDict()) {
        if (!key_) {
            throw std::logic_error("Do not have a key for create dictionary"s);
        }
        nodes_stack_.back()->AsDict()[*key_] = MakeNodeFromValue(value);
        key_ = std::nullopt;
        ++counter;
    }
    return *this;
}
    
Node Builder::Build() {
    if (!nodes_stack_.empty()) {
        throw std::logic_error("Can't build the object. Start != End"s);
    } else if (counter == 0) {
        throw std::logic_error("Can't build the object. Build created after constructor");
    }
    return root_;
}
    
Node Builder::MakeNodeFromValue(Node::Value value) const { 
    if (std::holds_alternative<int>(value)) { 
        return std::get<int>(value); 
    } 
    if (std::holds_alternative<double>(value)) { 
        return std::get<double>(value); 
    } 
    if (std::holds_alternative<bool>(value)) { 
        return std::get<bool>(value); 
    } 
    if (std::holds_alternative<std::string>(value)) { 
        return std::get<std::string>(value); 
    } 
    if (std::holds_alternative<Array>(value)) { 
        return std::get<Array>(value); 
    } 
    if (std::holds_alternative<Dict>(value)) { 
        return std::get<Dict>(value); 
    } 
    return {}; 
}
    
//---------------------------------------------------------KeyItemContext-----------------------------------------
DictItemContext KeyItemContext::Value(const Node::Value& value) {
    return builder_.Value(value);
}
  
DictItemContext KeyItemContext::StartDict() {
    return builder_.StartDict();
}
    
ArrayItemContext KeyItemContext::StartArray() {
    return builder_.StartArray();
}
    
//---------------------------------------------------------DictItemContext--------------------------------------    
    
KeyItemContext DictItemContext::Key(const std::string& key) {
    return builder_.Key(key);
}
    
Builder& DictItemContext::EndDict() {
    return builder_.EndDict();
}

//--------------------------------------------------------ArrayItemContext---------------------------------------
  
ArrayItemContext ArrayItemContext::Value(const Node::Value& value) {
    return builder_.Value(value);
}
    
DictItemContext ArrayItemContext::StartDict() {
    return builder_.StartDict();
}
    
ArrayItemContext ArrayItemContext::StartArray() {
    return builder_.StartArray();
}
    
Builder& ArrayItemContext::EndArray() {
    return builder_.EndArray();
}
    
} //namespace json
