#include "network/MessageSerializer.hpp"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

using namespace rapidjson;

std::string MessageSerializer::toJson(const Message& msg) {
    Document d;
    d.SetObject();
    auto& alloc = d.GetAllocator();

    d.AddMember("msg_type", static_cast<int>(msg.msg_type), alloc);
    d.AddMember("src_id", StringRef(msg.src_id.c_str()), alloc);
    d.AddMember("dst_id", StringRef(msg.dst_id.c_str()), alloc);

    Value content(kObjectType);
    for (const auto& kv : msg.msg_content) {
        content.AddMember(StringRef(kv.first.c_str()), StringRef(kv.second.c_str()), alloc);
    }
    d.AddMember("msg_content", content, alloc);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    return buffer.GetString();
}

Message MessageSerializer::fromJson(const std::string& jsonStr) {
    Document d;
    if (d.Parse(jsonStr.c_str()).HasParseError()) {
        throw std::runtime_error("JSON parsing error");
    }
    if (!d.HasMember("msg_type") || !d.HasMember("src_id") ||
        !d.HasMember("dst_id") || !d.HasMember("msg_content")) {
        throw std::runtime_error("Missing required JSON fields");
    }
    if (!d["msg_content"].IsObject()) {
        throw std::runtime_error("msg_content must be an object");
    }

    Message msg;
    msg.msg_type = static_cast<Message::Type>(d["msg_type"].GetInt());
    msg.src_id   = d["src_id"].GetString();
    msg.dst_id   = d["dst_id"].GetString();

    const Value& obj = d["msg_content"];
    for (auto it = obj.MemberBegin(); it != obj.MemberEnd(); ++it) {
        msg.msg_content[it->name.GetString()] = it->value.GetString();
    }
    return msg;
}

