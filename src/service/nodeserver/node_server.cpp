/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: node_server.cpp .
*
* Date: 2025-07-23
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "node_server.h"
#include "driver_collector.h"
#include "nodedto.hpp"
#include "rtdb.hpp"
#include "platform_sdk/init.h"
#include "platform_sdk/macro.h"
#include "platform_sdk/status.hpp"
#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/core/parser/ParsingError.hpp"
#include <lwlog/lwlog.h>

#include <platform_sdk/err.h>

#include "static_dto.hpp"
#include "dds/dds_manager.h"
#include "../../common/dto/TagDataDto.hpp"

using namespace vsoa_sdk;

extern CLWLog g_logger;
NodeServer NodeServer::node_server_;
NodeServer* NodeServer::GetInstance()
{
    return &node_server_;
}

// DDS RPC handler callbacks
static void tagReadHandler(void *arg, uint32_t client_id, const char *url, 
                           const lwdistcomm_message_t *req, lwdistcomm_message_t *resp)
{
    // Parse request (tag names list)
    std::string req_str(static_cast<const char*>(req->data), req->data_len);
    g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "DDS tag read request: %s", req_str.c_str());

    // Parse JSON array of tag names
    std::vector<std::string> tag_names;
    // Simple JSON parsing for ["tag1", "tag2"] format
    size_t start = req_str.find('[');
    size_t end = req_str.find(']');
    if (start != std::string::npos && end != std::string::npos && start < end) {
        std::string tags_str = req_str.substr(start + 1, end - start - 1);
        size_t pos = 0;
        while (pos < tags_str.length()) {
            size_t quote_start = tags_str.find('"', pos);
            if (quote_start == std::string::npos) break;
            size_t quote_end = tags_str.find('"', quote_start + 1);
            if (quote_end == std::string::npos) break;
            std::string tag_name = tags_str.substr(quote_start + 1, quote_end - quote_start - 1);
            tag_names.push_back(tag_name);
            pos = quote_end + 1;
        }
    }

    // Prepare response
    edge_framework::dto::TagDataList tag_data_list;
    for (const auto& tag_name : tag_names) {
        nodeserver::TagRecord rec;
        if (DATA_CENTER->GetRTDB()->getTag(tag_name.c_str(), rec)) {
            auto tag_data = edge_framework::dto::TagDataDto::createShared(
                rec.name, rec.value, rec.timestamp_ms
            );
            tag_data_list.push_back(tag_data);
        }
    }

    // Convert to JSON
    nodeserver::dds::DdsManager* dds_manager = nodeserver::dds::DdsManager::GetInstance();
    std::string resp_json = dds_manager->TagDataListToJson(tag_data_list);

    // Set response
    resp->data = const_cast<char*>(resp_json.c_str());
    resp->data_len = resp_json.length();

    g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "DDS tag read response: %s", resp_json.c_str());
}

static void tagControlHandler(void *arg, uint32_t client_id, const char *url, 
                              const lwdistcomm_message_t *req, lwdistcomm_message_t *resp)
{
    // Parse request (control command)
    std::string req_str(static_cast<const char*>(req->data), req->data_len);
    g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "DDS tag control request: %s", req_str.c_str());

    // Simple JSON parsing for {"name":"tag", "value":"value"} format
    std::string tag_name, tag_value;
    size_t name_start = req_str.find("\"name\":\"");
    if (name_start != std::string::npos) {
        size_t name_quote = req_str.find('"', name_start + 8);
        if (name_quote != std::string::npos) {
            size_t name_end = req_str.find('"', name_quote + 1);
            if (name_end != std::string::npos) {
                tag_name = req_str.substr(name_quote + 1, name_end - name_quote - 1);
            }
        }
    }

    size_t value_start = req_str.find("\"value\":\"");
    if (value_start != std::string::npos) {
        size_t value_quote = req_str.find('"', value_start + 9);
        if (value_quote != std::string::npos) {
            size_t value_end = req_str.find('"', value_quote + 1);
            if (value_end != std::string::npos) {
                tag_value = req_str.substr(value_quote + 1, value_end - value_quote - 1);
            }
        }
    }

    std::string resp_json = "{\"status\": \"ok\"}";
    if (!tag_name.empty() && !tag_value.empty()) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "DDS control tag %s to value %s",
            tag_name.c_str(), tag_value.c_str());
        DRIVER_COLLECTOR->SetTagControl(tag_name, tag_value);
    } else {
        resp_json = "{\"status\": \"error\", \"message\": \"Invalid arguments\"}";
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "DDS tag control failed: invalid arguments");
    }

    // Set response
    resp->data = const_cast<char*>(resp_json.c_str());
    resp->data_len = resp_json.length();

    g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "DDS tag control response: %s", resp_json.c_str());
}

int NodeServer::OnStart()
{
    vsoa_sdk::init();
    vsoa_sdk::setDataMode(DataMode::PARAM_MODE);

    auto serializerConfig = vsoa::parser::json::mapping::Serializer::Config::createShared();
    serializerConfig->includeNullFields = false;
    
    auto deserializerConfig = vsoa::parser::json::mapping::Deserializer::Config::createShared();
    deserializerConfig->allowUnknownFields = false;

    obj_mapper_ = vsoa::parser::json::mapping::ObjectMapper::createShared(serializerConfig, deserializerConfig);

    // Initialize and start DDS server
    std::string dds_address = "/tmp/nodeserver_dds.sock";
    int dds_init_ret = DDS_MANAGER->Init(server_name_, dds_address);
    if (dds_init_ret != 0) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to initialize DDS manager, ret: %d", dds_init_ret);
        // Continue execution even if DDS fails
    } else {
        // Add DDS RPC handlers
        DDS_MANAGER->AddTagReadHandler(tagReadHandler, this);
        DDS_MANAGER->AddTagControlHandler(tagControlHandler, this);

        // Start DDS server
        int dds_start_ret = DDS_MANAGER->Start();
        if (dds_start_ret != 0) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to start DDS server, ret: %d", dds_start_ret);
        } else {
            g_logger.LogMessage(LW_LOGLEVEL_INFO, "DDS server started successfully");
        }
    }

    server_handler_ = new server::ServerHandle(server_name_);
    int ret = server_handler_->CreateServer(
        [&](server::CliRpcInfo &cli_info, const void *dto, size_t len, void *arg)
        {
            vsoa::List<vsoa::Object<VsoaDtoHelp> > helps = vsoa::List<vsoa::Object<VsoaDtoHelp> >::createShared();
            if (cli_info.url == "/")
            {
                vsoa::Object<VsoaDtoHelp> help_req = vsoa::Object<VsoaDtoHelp>::createShared();
                help_req->url = rpc_request_path_;
                help_req->msg = "Read tags from nodeserver.";
                helps->push_back(help_req);

                vsoa::Object<VsoaDtoHelp> help_ctrl = vsoa::Object<VsoaDtoHelp>::createShared();
                help_ctrl->url = rpc_control_path_;
                help_ctrl->msg = "control tag from nodeserver.";
                helps->push_back(help_ctrl);
            }
            vsoa::String res = obj_mapper_->writeToString(helps);
            g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "[%s]:[%s] help",
                cli_info.url.c_str(), res->c_str());
            server_handler_->SrvResponse(cli_info, vsoa::Status::CODE_0, res);
        },
        this
    );
    if (ret != VSOA_OK) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Create server %s failed, ret is %d",
            server_name_.c_str(), ret);
        return -1;
    }

    server_handler_->AddRpcListener(rpc_request_path_.c_str(), 
        [&](server::CliRpcInfo &cli_info, const void *dto, size_t len, void *arg)
        {
            // Recv tags request;
            // ["tag1", "tag2"]
            vsoa::List<vsoa::String> taglist;
            try
            {
                taglist = obj_mapper_->readFromString<vsoa::List<vsoa::String>>(
                    vsoa::String((char*)dto, len)
                );
            }
            catch (vsoa::parser::ParsingError &e)
            {
                g_logger.LogMessage(LW_LOGLEVEL_ERROR, "url:%s, parse msg %.*s failed, errmsg is %s.",
                    cli_info.url.c_str(), len, (char*)dto, e.what());
                server_handler_->SrvResponse(cli_info, vsoa::Status::CODE_2, "Invalid arguments");
                return;
            }

            vsoa::List<vsoa::Object<DataValueDto>> objs = vsoa::List<vsoa::Object<DataValueDto>>::createShared();
            auto it = taglist->begin();
            for (; it != taglist->end() ; it++)
            {
                nodeserver::TagRecord rec;
                if (!DATA_CENTER->GetRTDB()->getTag((*it)->c_str(), rec))
                {
                    continue;
                }
                vsoa::Object<DataValueDto> obj = vsoa::Object<DataValueDto>::createShared();
                obj->name = rec.name;
                obj->value = rec.value;
                obj->time = rec.timestamp_ms;
                objs->push_back(obj);
            }
            vsoa::String req = obj_mapper_->writeToString(objs);
            server_handler_->SrvResponse(cli_info, vsoa::Status::CODE_0, req);
        }, 
        this
    );

    // device control rpc
    server_handler_->AddRpcListener(rpc_control_path_, 
        [&](server::CliRpcInfo &cli_info, const void *dto, size_t len, void *arg)
        {
            vsoa::Object<ControlValueDto> ctrl;
            try
            {
                ctrl = obj_mapper_->readFromString<vsoa::Object<ControlValueDto>>(
                    vsoa::String((char*)dto, len)
                );
            }
            catch (vsoa::parser::ParsingError &e)
            {
                g_logger.LogMessage(LW_LOGLEVEL_ERROR, "url:%s, parse msg %s failed, error:%s", 
                    cli_info.url.c_str(), len, (char*)dto, e.what());
                server_handler_->SrvResponse(cli_info, vsoa::Status::CODE_2, "Invalid arguments");
                return;
            }
            // 控制点位
            if (ctrl->name == nullptr || ctrl->value == nullptr)
            {
                server_handler_->SrvResponse(cli_info, vsoa::Status::CODE_2, "Invalid arguments");
                return;
            }

            g_logger.LogMessage(LW_LOGLEVEL_INFO, "Control tag %s to value %s",
                ctrl->name->c_str(), ctrl->value->c_str());
            DRIVER_COLLECTOR->SetTagControl(ctrl->name, ctrl->value);
            server_handler_->SrvResponse(cli_info, vsoa::Status::CODE_0, "OK");
        },
        this
    );
    bstop_ = false;
    thread_main_ = std::thread(
        this->MainTask,
        this
    );

    return 0;
}

int NodeServer::MainTask(NodeServer* srv)
{
    while (!srv->bstop_) {
        // Process VSOA events
        srv->server_handler_->SrvSpinOnce();
        
        // Process DDS events
        if (DDS_MANAGER->IsRunning()) {
            DDS_MANAGER->ProcessEvents();
        }
        
        // Small sleep to avoid busy loop
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Stop DDS server on exit
    if (DDS_MANAGER->IsRunning()) {
        DDS_MANAGER->Stop();
    }
    
    return 0;
}

int NodeServer::OnStop()
{
    return 0;
}

NodeServer::NodeServer()
{
    // TODO Auto-generated constructor stub 
    
}

NodeServer::~NodeServer()
{

}