/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: driver_collector.cpp .
*
* Date: 2025-07-25
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/


#include "driver_collector.h"
#include "lwcomm/lwcomm.h"
#include "lwipcssn/ipc_server.h"
#include "lwipcssn/ipc_client.h"
#include "lwipcssn/ipc_parser.h"
#include "lwlog/lwlog.h"
#include "data_center.h"
#include "rtdb.hpp"

#include "lwmsgq/lwmsgq.h"
#include "nodedto.hpp"
#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/core/parser/ParsingError.hpp"
#include "dds/dds_manager.h"
#include "../../common/dto/TagDataDto.hpp"
#include <string>

#ifndef LW_NAME_MAXLEN
#define LW_NAME_MAXLEN          128
#endif
#ifndef LW_VALUE_MAXLEN
#define LW_VALUE_MAXLEN         128
#endif
struct TagCtlInfo {
    std::string tag_name;
    std::string tag_value;
};

extern CLWLog g_logger;
DriverCollector DriverCollector::instance_;
DriverCollector* DriverCollector::GetInstance()
{
    return &instance_;
}

void DriverCollector::OnConnectedCb(void *arg, ipc_server_t *server, ipc_cli_id_t id, bool connect)
{
    DriverCollector* collector = (DriverCollector*)arg;
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "server client %d %s", 
        id, connect?"connected":"disconnected");
}

int DriverCollector::OnDevCtrlCb(void* msg, int msglen, void* userctx)
{
    // do device control.
    if (msg == NULL) {
        return -1;
    }
    DriverCollector *collector = (DriverCollector*)userctx;
    TagCtlInfo *tag_ctl_info = static_cast<TagCtlInfo*>(msg);
    nodeserver::TagRecord rec;
    bool ok = DATA_CENTER->GetRTDB()->getTag(tag_ctl_info->tag_name.c_str(), rec);
    if (!ok) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "tag %s not exist", tag_ctl_info->tag_name.c_str());
        return -1;
    }
    auto client_id = collector->client_map_.find(rec.driver_name);
    if (client_id == collector->client_map_.end()) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "client of %s not exist", 
            rec.driver_name.c_str());
        return -1;
    }
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "server driver %s device control %s", 
        rec.driver_name.c_str(), tag_ctl_info->tag_name.c_str());
    auto control_dto = ControlValueDto::createShared();
    control_dto->name = tag_ctl_info->tag_name;
    control_dto->value = tag_ctl_info->tag_value;
    vsoa::String payload_str =collector->obj_mapper_->writeToString(control_dto);
    ipc_payload_t payload {
        .data = (void*)payload_str->c_str(),
        .data_len = payload_str->length()
    };
    ipc_url_t url {
        .url = (char*)collector->control_url_.c_str(),
        .url_len = collector->control_url_.length()
    };
    ipc_server_cli_datagram(collector->server_, client_id->second, &url, &payload);
    return LW_SUCCESS;
}
void DriverCollector::OnDatagramCb(void *arg, ipc_server_t *server, ipc_cli_id_t id,
                                       ipc_url_t *url, ipc_payload_t *payload)
{
    DriverCollector* collector = (DriverCollector*)arg;
    if (std::string(url->url, url->url_len) == collector->publish_url_) {
        vsoa::List<vsoa::Object<DataValueDto> > dto;
        g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "OnDatagramCb: server %s, url is %s, msg is %.*s",
            collector->server_name_.c_str(), collector->publish_url_.c_str(), payload->data_len, (char*)payload->data);
        // Recv publish data here.
        try {
            dto = collector->obj_mapper_->readFromString<vsoa::List<vsoa::Object<DataValueDto> > >(
                vsoa::String((char*)payload->data, payload->data_len)
            );
        } catch (vsoa::parser::ParsingError &e) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "OnDatagramCb: parse %.*s to DataValueDto failed: %s",
                payload->data_len, payload->data, e.what());
            return;
        }
        // Prepare DDS tag data list
        edge_framework::dto::TagDataList tag_data_list;
        
        auto it_dto = dto->begin();
        for (; it_dto != dto->end(); it_dto++) {
            // parse time as milliseconds if provided
            uint64_t ts = 0;
            if ((*it_dto)->time) ts = (*it_dto)->time.getValue(0);
            
            // Store to RTDB
            DATA_CENTER->GetRTDB()->setTag((*it_dto)->name->c_str(), (*it_dto)->value->c_str(), ts, "", "");
            
            // Add to DDS publish list
            auto tag_data = edge_framework::dto::TagDataDto::createShared(
                (*it_dto)->name->c_str(),
                (*it_dto)->value->c_str(),
                ts
            );
            tag_data_list.push_back(tag_data);
        }
        
        // Publish via DDS
        if (!tag_data_list.empty() && DDS_MANAGER->IsRunning()) {
            bool publish_success = DDS_MANAGER->PublishTagData(tag_data_list, "/tags/update");
            if (!publish_success) {
                g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to publish tag data via DDS");
            } else {
                g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "Published %zu tag(s) via DDS", tag_data_list.size());
            }
        }
    } else if (std::string(url->url, url->url_len) == collector->taginit_url_) {
        vsoa::Object<DriverTagsDto> driver_tags;
        g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "OnDatagramCb: server %s, url is %s, msg is %.*s",
            collector->server_name_.c_str(), collector->taginit_url_.c_str(), payload->data_len, (char*)payload->data);
        // Recv tag init data here.
        try {
            driver_tags = collector->obj_mapper_->readFromString<vsoa::Object<DriverTagsDto> >(
                vsoa::String((char*)payload->data, payload->data_len)
            );
        } catch (vsoa::parser::ParsingError &e) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "OnDatagramCb: parse %.*s to DriverTagsDto failed: %s",
                payload->data_len, payload->data, e.what());
            return;
        }

        collector->client_map_[driver_tags->driver_name->c_str()] = id;

        g_logger.LogMessage(LW_LOGLEVEL_INFO, "OnDatagramCb: driver %s connected with client id %d, device count %zu",
            driver_tags->driver_name->c_str(), id, driver_tags->devtags->size());
        // 保存点位数据到 DataCenter
        for (auto dev_tags = driver_tags->devtags->begin(); dev_tags != driver_tags->devtags->end(); dev_tags++)
        {
            for (auto tag_name = (*dev_tags)->taglist->begin(); tag_name != (*dev_tags)->taglist->end(); tag_name++) {
                DATA_CENTER->GetRTDB()->setTag((*tag_name)->c_str(), "", 0, driver_tags->driver_name->c_str(), (*dev_tags)->device_name->c_str());
            }
        }
    }

    return;
}

int DriverCollector::OnStart()
{
    auto serializerConfig = vsoa::parser::json::mapping::Serializer::Config::createShared();
    serializerConfig->includeNullFields = false;
    
    auto deserializerConfig = vsoa::parser::json::mapping::Deserializer::Config::createShared();
    deserializerConfig->allowUnknownFields = false;

    obj_mapper_ = vsoa::parser::json::mapping::ObjectMapper::createShared(serializerConfig, deserializerConfig);

    sock_addr_ = LWComm::GetDataPath();
    sock_addr_ = sock_addr_ + LW_OS_DIR_SEPARATOR + server_name_;
    server_ = ipc_server_create(sock_addr_.c_str());
    if (server_ == NULL) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, 
            "DriverCollector create server %s failed", server_name_.c_str());
        return -1;
    }
    if (!ipc_server_start(server_, sock_addr_.c_str())) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "DriverCollector start server %s failed, addr is %s",
            server_name_.c_str(), sock_addr_.c_str());
        return -1;
    }

    ipc_server_on_cli(server_, OnConnectedCb, this);

    ipc_server_on_datagram(server_, OnDatagramCb, this);

    dev_ctrl_que_ = MsgQueCreate(100, DriverCollector::OnDevCtrlCb, this);
    if (dev_ctrl_que_ == NULL) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "DriverCollector create dev ctrl que failed");
        return -1;
    }
    ctrl_thread_ = std::thread([&]{
        while (true) {
            int err = 0;
            int ret = MsgQueWorkerWaitfor(dev_ctrl_que_, &err);
            if (ret != 0) {
                g_logger.LogMessage(LW_LOGLEVEL_ERROR, "DriverCollector::OnStart wait update sql queue failed %d.", 
                    err);
            }
        }
    });

    bstop_ = false;
    main_thread_ = std::thread(DriverCollector::SrvThreadMain, this);

    return 0;
}

int DriverCollector::OnStop()
{
    bstop_ = true;
    if (main_thread_.joinable()) {
        main_thread_.join();
    }
    return 0;
}

void DriverCollector::SrvThreadMain(void* arg)
{
    DriverCollector* collector = (DriverCollector*)arg;
    int cnt, max_fd;
    fd_set fds;
    struct timespec timeout = { 1, 0 };
    while (!collector->bstop_) {
        FD_ZERO(&fds);
        max_fd = ipc_server_fds(collector->server_, &fds);

        cnt = pselect(max_fd + 1, &fds, NULL, NULL, &timeout, NULL);
        if (cnt > 0) {
            ipc_server_input_fds(collector->server_, &fds);
        }
    }
    return;
}

int DriverCollector::SetTagControl(std::string tag_name, std::string tag_value)
{
    int ret = 0;
    TagCtlInfo *ctrl_info;
    try {
        ctrl_info = new TagCtlInfo();
    } catch (std::bad_alloc &e) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "new TagCtlInfo failed, %s",
            e.what());
        return -1;
    }

    ctrl_info->tag_name = tag_name;
    ctrl_info->tag_value = tag_value;

    if (MsgQuePush(dev_ctrl_que_, ctrl_info, sizeof(TagCtlInfo)) != 0) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Push log msg to device control queue failed.");
        delete ctrl_info;
    }

    return 0;
}