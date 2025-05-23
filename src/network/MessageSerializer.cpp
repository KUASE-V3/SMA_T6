#include "network/MessageSerializer.hpp"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

using namespace network;

std::string MessageSerializer::toJson(const Message& msg)
{
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> w(buf);
    w.StartObject();
    w.Key("msg_type"); w.Int(static_cast<int>(msg.msg_type));
    w.Key("src_id");   w.String(msg.src_id.c_str());
    w.Key("dst_id");   w.String(msg.dst_id.c_str());
    w.Key("msg_content"); w.StartObject();
    for (auto& kv : msg.msg_content) {
        w.Key(kv.first.c_str()); w.String(kv.second.c_str());
    }
    w.EndObject();
    w.EndObject();
    return buf.GetString();
}

Message MessageSerializer::fromJson(const std::string& s)
{
    rapidjson::Document d; d.Parse(s.c_str());
    Message m;
    m.msg_type = static_cast<Message::Type>(d["msg_type"].GetInt());
    m.src_id   = d["src_id"].GetString();
    m.dst_id   = d["dst_id"].GetString();
    for (auto& mem : d["msg_content"].GetObject()) {
        m.msg_content[mem.name.GetString()] = mem.value.GetString();
    }
    return m;
}