#!/bin/bash

# 视频服务器启动脚本

# 脚本所在目录
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# 项目根目录
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"

# 可执行文件路径
EXECUTABLE="$PROJECT_ROOT/build/video-server"
# 配置文件路径
CONFIG_FILE="$PROJECT_ROOT/config/config.json"
# 日志目录
LOG_DIR="$PROJECT_ROOT/logs"
# PID文件
PID_FILE="/tmp/video-server.pid"

# 确保日志目录存在
mkdir -p "$LOG_DIR"

# 函数：显示帮助信息
show_help() {
    echo "视频服务器启动脚本"
    echo "用法: $0 [start|stop|restart|status]"
    echo ""
    echo "选项:"
    echo "  start    启动视频服务器"
    echo "  stop     停止视频服务器"
    echo "  restart  重启视频服务器"
    echo "  status   查看视频服务器状态"
    echo "  help     显示帮助信息"
}

# 函数：检查服务器是否在运行
is_running() {
    if [ -f "$PID_FILE" ]; then
        PID=$(cat "$PID_FILE")
        if ps -p "$PID" > /dev/null 2>&1; then
            return 0
        else
            rm -f "$PID_FILE"
            return 1
        fi
    else
        return 1
    fi
}

# 函数：启动服务器
start_server() {
    if is_running; then
        echo "视频服务器已经在运行（PID: $(cat "$PID_FILE")）"
        return 1
    fi

    echo "正在启动视频服务器..."
    
    # 检查可执行文件是否存在
    if [ ! -f "$EXECUTABLE" ]; then
        echo "错误: 可执行文件不存在，请先编译项目"
        echo "编译命令: cd $PROJECT_ROOT && mkdir -p build && cd build && cmake .. && make"
        return 1
    fi

    # 检查配置文件是否存在
    if [ ! -f "$CONFIG_FILE" ]; then
        echo "错误: 配置文件不存在"
        return 1
    fi

    # 启动服务器
    "$EXECUTABLE" --config "$CONFIG_FILE" > "$LOG_DIR/video-server.log" 2>&1 &
    
    # 保存PID
    echo $! > "$PID_FILE"
    
    # 检查启动是否成功
    sleep 2
    if is_running; then
        echo "视频服务器启动成功（PID: $(cat "$PID_FILE")）"
        return 0
    else
        echo "视频服务器启动失败"
        return 1
    fi
}

# 函数：停止服务器
stop_server() {
    if ! is_running; then
        echo "视频服务器未在运行"
        return 1
    fi

    echo "正在停止视频服务器..."
    
    PID=$(cat "$PID_FILE")
    kill "$PID"
    
    # 等待进程退出
    for i in {1..10}; do
        if ! ps -p "$PID" > /dev/null 2>&1; then
            break
        fi
        sleep 1
    done

    if ps -p "$PID" > /dev/null 2>&1; then
        echo "强制停止视频服务器..."
        kill -9 "$PID"
        sleep 1
    fi

    rm -f "$PID_FILE"
    echo "视频服务器已停止"
    return 0
}

# 函数：查看服务器状态
check_status() {
    if is_running; then
        echo "视频服务器正在运行（PID: $(cat "$PID_FILE")）"
        return 0
    else
        echo "视频服务器未在运行"
        return 1
    fi
}

# 主函数
main() {
    case "$1" in
        start)
            start_server
            ;;
        stop)
            stop_server
            ;;
        restart)
            stop_server
            start_server
            ;;
        status)
            check_status
            ;;
        help|")
            show_help
            ;;
        *)
            echo "错误: 无效的参数"
            show_help
            return 1
            ;;
    esac
}

# 执行主函数
main "$@"
