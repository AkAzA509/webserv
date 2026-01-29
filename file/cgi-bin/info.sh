#!/bin/bash

cat << EOF
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <title>Server Info</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; }
        .container { background: rgba(255,255,255,0.1); padding: 20px; border-radius: 10px; }
        h1 { color: #ffeb3b; }
        .info-section { margin: 20px 0; padding: 15px; background: rgba(0,0,0,0.2); border-radius: 5px; }
        .info-item { margin: 10px 0; }
        .label { font-weight: bold; color: #ffeb3b; }
        a { color: #81c784; text-decoration: none; }
        a:hover { text-decoration: underline; }
        pre { background: rgba(0,0,0,0.3); padding: 10px; border-radius: 3px; overflow-x: auto; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🖥️ Server Information</h1>
        
        <div class="info-section">
            <h2>📡 Request Info (CGI Standard)</h2>
            <div class="info-item"><span class="label">Method:</span> ${REQUEST_METHOD:-N/A}</div>
            <div class="info-item"><span class="label">URI:</span> ${REQUEST_URI:-N/A}</div>
            <div class="info-item"><span class="label">Query String:</span> ${QUERY_STRING:-None}</div>
            <div class="info-item"><span class="label">Server Name:</span> ${SERVER_NAME:-N/A}</div>
            <div class="info-item"><span class="label">Server Port:</span> ${SERVER_PORT:-N/A}</div>
            <div class="info-item"><span class="label">Client IP:</span> ${REMOTE_ADDR:-N/A}</div>
            <div class="info-item"><span class="label">Script Name:</span> ${SCRIPT_NAME:-N/A}</div>
            <div class="info-item"><span class="label">Path Info:</span> ${PATH_INFO:-N/A}</div>
        </div>

        <div class="info-section">
            <h2>🌐 HTTP Headers</h2>
            <div class="info-item"><span class="label">HTTP_HOST:</span> ${HTTP_HOST:-N/A}</div>
            <div class="info-item"><span class="label">HTTP_USER_AGENT:</span> ${HTTP_USER_AGENT:-N/A}</div>
            <div class="info-item"><span class="label">HTTP_ACCEPT:</span> ${HTTP_ACCEPT:-N/A}</div>
            <div class="info-item"><span class="label">CONTENT_TYPE:</span> ${CONTENT_TYPE:-N/A}</div>
            <div class="info-item"><span class="label">CONTENT_LENGTH:</span> ${CONTENT_LENGTH:-N/A}</div>
        </div>

        <div class="info-section">
            <h2>⏰ System Info</h2>
            <div class="info-item"><span class="label">Date:</span> $(date)</div>
            <div class="info-item"><span class="label">Uptime:</span> $(uptime | cut -d',' -f1)</div>
            <div class="info-item"><span class="label">Current Directory:</span> $(pwd)</div>
            <div class="info-item"><span class="label">Process ID:</span> $$</div>
        </div>

        <div style="margin-top: 30px; text-align: center;">
            <a href="/">🏠 Retour à l'accueil</a>
        </div>
    </div>
</body>
</html>
EOF