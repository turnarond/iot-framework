/*
 * @Author: yanchaodong
 * @Date: 2025-11-03 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-11-05 14:45:59
 * @FilePath: /acu/driver-sdk/src/service/linkserver/linkevt_alarm/alarm_subscriber.cpp
 * @Description: 报警事件源插件（基于 IEventSource 接口）
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#include "alarm_subscriber.h"
#include "Poco/Exception.h"
#include "lwcomm/lwcomm.h"
#include "lwlog/lwlog.h"
#include "util/plugin_interface.h"
#include "alarm_node_dto.hpp"

#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Stringifier.h>

using namespace Poco::Data;
using namespace Poco::Data::Keywords;

// 全局注册 SQLite
namespace {
    struct SQLiteInitializer {
        SQLiteInitializer() {
            static bool initialized = false;
            if (!initialized) {
                Poco::Data::SQLite::Connector::registerConnector();
                initialized = true;
            }
        }
    } sqliteInit;
}

// 报警规则到触发源的映射：alarm_rule_id -> trigger_id
std::unordered_map<int, int> g_alarmRuleToTriggerMap;
void AlarmEventSource::OnConnect(bool connect, std::string servername, 
                          vsoa_sdk::client::CLIENT_CONTEXT ctx, void *arg)
{
    AlarmEventSource* cli = static_cast<AlarmEventSource*>(arg);
    if (cli->logger_) {
        cli->logger_->LogMessage(LW_LOGLEVEL_INFO, "AlarmNodeSubscriber::OnConnect, servername = %s, connect = %s",
            servername.c_str(), connect?"connected":"disconnected");
    }
}

bool AlarmEventSource::init(const std::string& configJson)
{
    if (logger_ == nullptr) {
        printf("AlarmEventSource::init logger_ is nullptr");
    }

    client_ = std::make_shared<vsoa_sdk::client::ClientHandle>();
    /* Craete DTO mapping. */
    auto serializerConfig = vsoa::parser::json::mapping::Serializer::Config::createShared();
    serializerConfig->includeNullFields = false;
    
    auto deserializerConfig = vsoa::parser::json::mapping::Deserializer::Config::createShared();
    deserializerConfig->allowUnknownFields = true;

    obj_mapper_ = vsoa::parser::json::mapping::ObjectMapper::createShared(serializerConfig, deserializerConfig);

    vsoa_sdk::client::SrvConnInfo srv_conn_info;
    srv_conn_info.server_name = alarm_server_name_;
    srv_conn_info.conn_cb = OnConnect;
    srv_conn_info.conn_arg = this;
    client_ctx_ = client_->CreateClient(srv_conn_info);
    if (client_ctx_ == nullptr) {
        if (logger_) {
            logger_->LogMessage(LW_LOGLEVEL_ERROR, "create alarm node client to %s failed.",
                                alarm_server_name_.c_str());
        }
        return -1;
    }
    if (logger_) {
        logger_->LogMessage(LW_LOGLEVEL_INFO, "AlarmEventSource: create link event source client to %s success.",
                        alarm_server_name_.c_str());
    }

    // 从数据库中读取告警规则
    std::string db_path_ = LWComm::GetConfigPath();
    db_path_ = db_path_ + LW_OS_DIR_SEPARATOR + "lw_monitor.db";
    try {
        Session session("SQLite", db_path_); // 替换为你的实际 DB 路径

        // 查询所有启用的、事件类型为 'alarm-produce' 的触发源
        Statement select(session);
        select << "SELECT ts.id, ts.alarm_rule_id "
               << "FROM t_linkage_trigger ts "
               << "JOIN t_trigger_type et ON ts.eventtype_name = et.name "
               << "WHERE et.name = 'alarm-produce' AND ts.enable = 1";

        select.execute();

        RecordSet rs(select);
        bool more = rs.moveFirst();
        g_alarmRuleToTriggerMap.clear();

        while (more) {
            try {
                int trigger_id = rs["id"];
                int alarm_rule_id = rs["alarm_rule_id"]; 
                g_alarmRuleToTriggerMap[alarm_rule_id] = trigger_id;
                if (logger_) {
                    logger_->LogMessage(LW_LOGLEVEL_DEBUG, 
                        "Loaded trigger: alarm_rule_id=%d -> trigger_id=%d", 
                        alarm_rule_id, trigger_id);
                }
            } catch (Poco::Exception &e) {
                if (logger_) {
                    logger_->LogMessage(LW_LOGLEVEL_WARN, "get trigger failed: %s", e.displayText().c_str());
                }
            }
            more = rs.moveNext();
        }

        if (logger_) {
            logger_->LogMessage(LW_LOGLEVEL_INFO, 
                "Loaded %zu alarm triggers from DB", g_alarmRuleToTriggerMap.size());
        }

    } catch (Poco::Exception& ex) {
        if (logger_) {
            logger_->LogMessage(LW_LOGLEVEL_ERROR, 
                "Failed to load alarm triggers from DB: %s", ex.displayText().c_str());
        }
        return false;
    }

    return true;
}

bool AlarmEventSource::start(const EventCallback& cb) 
{
    if (!client_) {
        return false;
    }
    user_callback_ = cb;
    client_->Subscribe(client_ctx_, url_alarm_publish_, [&](const std::string& url, const void *dto, size_t len, void *arg)
    {
        // 处理收到的事件点位信息
        // push 到报警判断模块
        vsoa::Object<AlarmMessageDto> alarm_node;
        if (logger_) {
            logger_->LogMessage(LW_LOGLEVEL_INFO, "AlarmEventSource receive alarm message %.*s.",
                len, (char*)dto);
        }
        try {
            alarm_node = obj_mapper_->readFromString<vsoa::Object<AlarmMessageDto> >(
                vsoa::String((char*)dto, len));
        } catch (vsoa::parser::ParsingError &e) {
            if (logger_) {
                logger_->LogMessage(LW_LOGLEVEL_ERROR, "AlarmEventSource ParsingError: %s", 
                    e.what());
            } else {
                printf("AlarmEventSource ParsingError: %s\n", e.what());
            }
            return;
        }
        // 检查是触发还是恢复
        if (alarm_node->message_type == AlarmMessageType::ALARM_CLEAR) {
            if (logger_) {
                logger_->LogMessage(LW_LOGLEVEL_INFO, "AlarmEventSource receive alarm clear message %.*s.",
                    len, (char*)dto);
            }
        }

        // 触发报警；检查是否配置了对应的联动
        int alarm_rule_id = alarm_node->rule_id;
        try {
            auto it = g_alarmRuleToTriggerMap.find(alarm_rule_id);
            if (it != g_alarmRuleToTriggerMap.end()) {
                int trigger_id = it->second;
                // 构造事件 payload：包含 trigger_id + 原始数据
                Poco::JSON::Object eventJson;
                eventJson.set("trigger_id", trigger_id);
                eventJson.set("point_id", alarm_node->point_id->c_str());
                eventJson.set("value", alarm_node->point_value->c_str());
                eventJson.set("method", alarm_node->rule_method.getValue(0));
                eventJson.set("timestamp", alarm_node->timestamp.getValue(0));
                eventJson.set("raw", std::string((char*)dto, len));

                std::ostringstream oss;
                eventJson.stringify(oss);
                std::string finalPayload = oss.str();

                if (logger_) {
                    logger_->LogMessage(LW_LOGLEVEL_INFO, 
                        "Alarm matched trigger %d, firing event", trigger_id);
                }

                this->user_callback_(trigger_id, finalPayload); // 通知联动服务
            } else {
                if (logger_) {
                    logger_->LogMessage(LW_LOGLEVEL_DEBUG, 
                        "Alarm rule %d not configured for linkage, ignored", alarm_rule_id);
                }
            }
        } catch (std::exception &e) {
            if (logger_) {
                logger_->LogMessage(LW_LOGLEVEL_ERROR, "AlarmEventSource error: %s", 
                    e.what());
            } else {
                printf("AlarmEventSource error: %s\n", e.what());
            }
        } catch (...) {
            if (logger_) {
                logger_->LogMessage(LW_LOGLEVEL_ERROR, "AlarmEventSource unknown error");
            } else {
                printf("AlarmEventSource unknown error\n");
            }
        }
    }, 
    this);

    return true;
}

void AlarmEventSource::stop()
{
    if (logger_) {
        logger_->LogMessage(LW_LOGLEVEL_INFO, "AlarmEventSource stop");
    }

    client_->DestroyClient(client_ctx_);

    return;

}

// =============== 插件导出函数 ===============
extern "C" {
    IEventSource* create_event_source() {
        try {
            return new AlarmEventSource();
        } catch (...) {
            return nullptr;
        }
    }

    void destroy_event_source(IEventSource* p) {
        delete p;
    }
} // extern "C"
