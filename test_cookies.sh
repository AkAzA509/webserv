#!/bin/bash

# Cookie Testing Script for WebServ
# Usage: ./test_cookies.sh [server_port]

PORT=${1:-8080}
SERVER="http://localhost:$PORT"
COOKIE_JAR="/tmp/webserv_cookies.txt"

echo "🍪 WebServ Cookie Testing Script"
echo "================================="
echo "Server: $SERVER"
echo "Cookie jar: $COOKIE_JAR"
echo ""

# Clean up old cookies
rm -f "$COOKIE_JAR"

echo "1. Testing basic GET request without cookies..."
curl -s -w "Status: %{http_code}\n" "$SERVER/" || echo "❌ Server not responding"
echo ""

echo "2. Testing GET request with cookies..."
curl -s -w "Status: %{http_code}\n" \
     -H "Cookie: test_cookie=hello_world; session_id=abc123" \
     "$SERVER/" || echo "❌ Request failed"
echo ""

echo "3. Testing POST with cookies..."
curl -s -w "Status: %{http_code}\n" \
     -X POST \
     -H "Content-Type: application/json" \
     -H "Cookie: user_id=12345; preferences=dark_theme" \
     -d '{"test": "data"}' \
     "$SERVER/api/test" || echo "❌ POST request failed"
echo ""

echo "4. Testing cookie persistence (using cookie jar)..."
# Set a cookie and save it
curl -s -c "$COOKIE_JAR" \
     -H "Cookie: persistent_cookie=save_me" \
     "$SERVER/" > /dev/null

# Use the saved cookie
curl -s -b "$COOKIE_JAR" -w "Status: %{http_code}\n" \
     "$SERVER/cookie-test" || echo "❌ Cookie persistence test failed"
echo ""

echo "5. Testing multiple cookies..."
curl -s -w "Status: %{http_code}\n" \
     -H "Cookie: cookie1=value1; cookie2=value2; cookie3=value3; session=xyz789" \
     "$SERVER/" || echo "❌ Multiple cookies test failed"
echo ""

echo "6. Testing cookie with special characters..."
curl -s -w "Status: %{http_code}\n" \
     -H "Cookie: special_cookie=hello%20world%21; encoded=test%3Dvalue" \
     "$SERVER/" || echo "❌ Special characters test failed"
echo ""

echo "7. Testing very long cookie..."
LONG_VALUE=$(python3 -c "print('x' * 1000)")
curl -s -w "Status: %{http_code}\n" \
     -H "Cookie: long_cookie=$LONG_VALUE" \
     "$SERVER/" || echo "❌ Long cookie test failed"
echo ""

echo "8. Testing empty cookie value..."
curl -s -w "Status: %{http_code}\n" \
     -H "Cookie: empty_cookie=; another_cookie=value" \
     "$SERVER/" || echo "❌ Empty cookie test failed"
echo ""

echo "9. Testing invalid cookie format..."
curl -s -w "Status: %{http_code}\n" \
     -H "Cookie: invalid cookie format" \
     "$SERVER/" || echo "❌ Invalid cookie test failed"
echo ""

echo "10. Showing cookie jar contents (if any)..."
if [ -f "$COOKIE_JAR" ]; then
    echo "Cookie jar contents:"
    cat "$COOKIE_JAR"
else
    echo "No cookie jar found"
fi
echo ""

echo "✅ Cookie testing completed!"
echo ""
echo "📝 To test manually:"
echo "   curl -H 'Cookie: name=value; another=test' $SERVER/"
echo "   curl -c cookies.txt -b cookies.txt $SERVER/"
echo ""
echo "🌐 Web interface:"
echo "   Open $SERVER/cookie-test.html in your browser"
