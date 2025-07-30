#!/usr/bin/env python3
"""
Advanced Cookie Testing for WebServ
Usage: python3 test_cookies_advanced.py [port]
"""

import requests
import sys
import time
from datetime import datetime

def test_cookies(port=8080):
    base_url = f"http://localhost:{port}"
    
    print("🍪 Advanced Cookie Testing for WebServ")
    print("=" * 50)
    print(f"Server: {base_url}")
    print()
    
    # Test 1: Basic cookie sending
    print("1. Testing basic cookie sending...")
    try:
        cookies = {'test_cookie': 'hello_world', 'session_id': 'abc123'}
        response = requests.get(base_url, cookies=cookies, timeout=5)
        print(f"   Status: {response.status_code}")
        print(f"   Cookies sent: {cookies}")
        if 'Set-Cookie' in response.headers:
            print(f"   Cookies received: {response.headers['Set-Cookie']}")
    except Exception as e:
        print(f"   ❌ Error: {e}")
    print()
    
    # Test 2: Session cookie persistence
    print("2. Testing session cookie persistence...")
    try:
        session = requests.Session()
        session.cookies.set('persistent_session', 'session_value_123')
        
        # First request
        resp1 = session.get(base_url)
        print(f"   First request status: {resp1.status_code}")
        
        # Second request (should maintain cookies)
        resp2 = session.get(f"{base_url}/cookie-test.html")
        print(f"   Second request status: {resp2.status_code}")
        print(f"   Session cookies: {dict(session.cookies)}")
    except Exception as e:
        print(f"   ❌ Error: {e}")
    print()
    
    # Test 3: Cookie with expiration
    print("3. Testing cookies with expiration...")
    try:
        cookies = {
            'temporary': 'short_lived',
            'permanent': 'long_lived'
        }
        headers = {
            'Cookie': 'temporary=short_lived; expires=Wed, 09 Jun 2025 10:18:14 GMT; permanent=long_lived; expires=Thu, 09 Jun 2030 10:18:14 GMT'
        }
        response = requests.get(base_url, headers=headers, timeout=5)
        print(f"   Status: {response.status_code}")
    except Exception as e:
        print(f"   ❌ Error: {e}")
    print()
    
    # Test 4: Large cookie values
    print("4. Testing large cookie values...")
    try:
        large_value = 'x' * 2000  # 2KB cookie
        cookies = {'large_cookie': large_value}
        response = requests.get(base_url, cookies=cookies, timeout=5)
        print(f"   Status: {response.status_code}")
        print(f"   Large cookie size: {len(large_value)} characters")
    except Exception as e:
        print(f"   ❌ Error: {e}")
    print()
    
    # Test 5: Special characters in cookies
    print("5. Testing special characters in cookies...")
    try:
        special_cookies = {
            'encoded': 'hello%20world',
            'symbols': 'test=value&other=data',
            'unicode': 'café_münü'
        }
        response = requests.get(base_url, cookies=special_cookies, timeout=5)
        print(f"   Status: {response.status_code}")
        print(f"   Special cookies: {special_cookies}")
    except Exception as e:
        print(f"   ❌ Error: {e}")
    print()
    
    # Test 6: Multiple cookie headers
    print("6. Testing multiple cookie scenarios...")
    try:
        # Test with many cookies
        many_cookies = {f'cookie_{i}': f'value_{i}' for i in range(10)}
        response = requests.get(base_url, cookies=many_cookies, timeout=5)
        print(f"   Multiple cookies status: {response.status_code}")
        
        # Test with empty cookie values
        empty_cookies = {'empty': '', 'normal': 'value'}
        response = requests.get(base_url, cookies=empty_cookies, timeout=5)
        print(f"   Empty cookie status: {response.status_code}")
    except Exception as e:
        print(f"   ❌ Error: {e}")
    print()
    
    # Test 7: POST with cookies
    print("7. Testing POST requests with cookies...")
    try:
        cookies = {'csrf_token': 'secure_token_123', 'user_id': '456'}
        data = {'test': 'post_data', 'timestamp': str(int(time.time()))}
        response = requests.post(f"{base_url}/api/test", 
                               cookies=cookies, 
                               json=data, 
                               timeout=5)
        print(f"   POST with cookies status: {response.status_code}")
    except Exception as e:
        print(f"   ❌ Error: {e}")
    print()
    
    # Test 8: Cookie parsing edge cases
    print("8. Testing cookie parsing edge cases...")
    try:
        # Malformed cookies
        headers = {'Cookie': 'malformed cookie without equals'}
        response = requests.get(base_url, headers=headers, timeout=5)
        print(f"   Malformed cookie status: {response.status_code}")
        
        # Cookies with spaces
        headers = {'Cookie': 'spaced = value with spaces ; another=normal'}
        response = requests.get(base_url, headers=headers, timeout=5)
        print(f"   Spaced cookie status: {response.status_code}")
    except Exception as e:
        print(f"   ❌ Error: {e}")
    print()
    
    print("✅ Advanced cookie testing completed!")
    print()
    print("📝 Manual testing commands:")
    print(f"   curl -H 'Cookie: test=value' {base_url}")
    print(f"   curl -c cookies.txt -b cookies.txt {base_url}")
    print()
    print("🌐 Open in browser:")
    print(f"   {base_url}/cookie-test.html")

if __name__ == "__main__":
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8080
    test_cookies(port)
