#!/usr/bin/env python3
"""
简化版NodeServer测试脚本
只测试HTTP接口功能，避免使用websocket模块
"""

import requests
import json
import time

# 测试配置
HTTP_SERVER_URL = "http://localhost:8081"

class TestNodeServerHTTP:
    """NodeServer HTTP测试类"""
    
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
    
    def run_all_tests(self):
        """运行所有测试"""
        print("Starting NodeServer HTTP tests...")
        print(f"HTTP Server URL: {HTTP_SERVER_URL}")
        
        tests = [
            self.test_http_batch_query,
            self.test_http_prefix_query,
            self.test_http_control_command
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
            print("🎉 All HTTP tests passed!")
        else:
            print("⚠ Some HTTP tests failed or need attention")
        
        return passed == total

if __name__ == "__main__":
    tester = TestNodeServerHTTP()
    tester.run_all_tests()
