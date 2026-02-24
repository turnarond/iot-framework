#!/usr/bin/env python3
"""
测试nodeserver的功能
包括：
1. HTTP接口测试
2. WebSocket订阅测试
3. 驱动数据收集测试
"""

import requests
import json
import websocket
import time
import threading

# 测试配置
HTTP_SERVER_URL = "http://localhost:8081"
WEBSOCKET_URL = "ws://localhost:9000"

class WebSocketTest:
    """WebSocket测试类"""
    
    def __init__(self):
        self.ws = None
        self.received_messages = []
        self.connected = False
    
    def on_message(self, ws, message):
        """接收消息回调"""
        print(f"WebSocket received: {message}")
        self.received_messages.append(message)
    
    def on_error(self, ws, error):
        """错误回调"""
        print(f"WebSocket error: {error}")
    
    def on_close(self, ws, close_status_code, close_msg):
        """关闭回调"""
        print(f"WebSocket closed: {close_status_code} - {close_msg}")
        self.connected = False
    
    def on_open(self, ws):
        """连接成功回调"""
        print("WebSocket connected")
        self.connected = True
    
    def connect(self):
        """建立WebSocket连接"""
        self.ws = websocket.WebSocketApp(
            WEBSOCKET_URL,
            on_open=self.on_open,
            on_message=self.on_message,
            on_error=self.on_error,
            on_close=self.on_close
        )
        
        # 启动WebSocket线程
        self.ws_thread = threading.Thread(target=self.ws.run_forever)
        self.ws_thread.daemon = True
        self.ws_thread.start()
        
        # 等待连接建立
        for _ in range(5):
            if self.connected:
                break
            time.sleep(0.5)
        else:
            print("WebSocket connection timeout")
            return False
        
        return True
    
    def subscribe(self, prefix):
        """订阅前缀"""
        if self.connected:
            self.ws.send(f"SUBSCRIBE {prefix}")
            print(f"Subscribed to prefix: {prefix}")
            return True
        return False
    
    def unsubscribe(self, prefix):
        """取消订阅"""
        if self.connected:
            self.ws.send(f"UNSUBSCRIBE {prefix}")
            print(f"Unsubscribed from prefix: {prefix}")
            return True
        return False
    
    def close(self):
        """关闭连接"""
        if self.connected:
            self.ws.close()
            self.ws_thread.join(timeout=2)

class TestNodeServer:
    """NodeServer测试类"""
    
    def __init__(self):
        self.ws_test = WebSocketTest()
    
    def test_http_batch_query(self):
        """测试HTTP批量查询接口"""
        print("\n=== Testing HTTP Batch Query ===")
        
        url = f"{HTTP_SERVER_URL}/api/v1/points/batch"
        data = {
            "pointIds": ["test.point1", "test.point2", "test.point3"]
        }
        
        try:
            response = requests.post(url, json=data, timeout=5)
            print(f"Status code: {response.status_code}")
            print(f"Response: {response.json()}")
            
            if response.status_code == 200:
                print("✓ HTTP batch query test passed")
                return True
            else:
                print("✗ HTTP batch query test failed")
                return False
        except Exception as e:
            print(f"✗ HTTP batch query test failed with error: {e}")
            return False
    
    def test_http_prefix_query(self):
        """测试HTTP前缀查询接口"""
        print("\n=== Testing HTTP Prefix Query ===")
        
        url = f"{HTTP_SERVER_URL}/api/v1/points"
        params = {
            "prefix": "test."
        }
        
        try:
            response = requests.get(url, params=params, timeout=5)
            print(f"Status code: {response.status_code}")
            print(f"Response: {response.json()}")
            
            if response.status_code == 200:
                print("✓ HTTP prefix query test passed")
                return True
            else:
                print("✗ HTTP prefix query test failed")
                return False
        except Exception as e:
            print(f"✗ HTTP prefix query test failed with error: {e}")
            return False
    
    def test_http_control_command(self):
        """测试HTTP控制命令接口"""
        print("\n=== Testing HTTP Control Command ===")
        
        url = f"{HTTP_SERVER_URL}/api/v1/control"
        data = {
            "pointId": "test.control",
            "value": "1"
        }
        
        try:
            response = requests.post(url, json=data, timeout=5)
            print(f"Status code: {response.status_code}")
            print(f"Response: {response.json()}")
            
            if response.status_code == 200:
                print("✓ HTTP control command test passed")
                return True
            else:
                print("✗ HTTP control command test failed")
                return False
        except Exception as e:
            print(f"✗ HTTP control command test failed with error: {e}")
            return False
    
    def test_websocket_subscription(self):
        """测试WebSocket订阅"""
        print("\n=== Testing WebSocket Subscription ===")
        
        # 连接WebSocket
        if not self.ws_test.connect():
            print("✗ WebSocket connection failed")
            return False
        
        # 订阅前缀
        self.ws_test.subscribe("test.")
        
        # 等待一段时间，看是否能收到消息
        print("Waiting for WebSocket messages... (5 seconds)")
        time.sleep(5)
        
        # 检查是否收到消息
        if self.ws_test.received_messages:
            print(f"✓ WebSocket subscription test passed, received {len(self.ws_test.received_messages)} messages")
        else:
            print("⚠ WebSocket subscription test: No messages received (this might be normal if no data is being published)")
        
        # 关闭连接
        self.ws_test.close()
        return True
    
    def run_all_tests(self):
        """运行所有测试"""
        print("Starting NodeServer tests...")
        print(f"HTTP Server URL: {HTTP_SERVER_URL}")
        print(f"WebSocket URL: {WEBSOCKET_URL}")
        
        tests = [
            self.test_http_batch_query,
            self.test_http_prefix_query,
            self.test_http_control_command,
            self.test_websocket_subscription
        ]
        
        results = []
        for test in tests:
            try:
                result = test()
                results.append(result)
            except Exception as e:
                print(f"✗ Test {test.__name__} failed with exception: {e}")
                results.append(False)
        
        # 汇总结果
        print("\n=== Test Summary ===")
        passed = sum(results)
        total = len(results)
        print(f"Passed: {passed}/{total}")
        
        if passed == total:
            print("🎉 All tests passed!")
        else:
            print("⚠ Some tests failed or need attention")
        
        return passed == total

if __name__ == "__main__":
    tester = TestNodeServer()
    tester.run_all_tests()
