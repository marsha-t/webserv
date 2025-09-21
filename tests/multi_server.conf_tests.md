# 🧪 Tests for `multi_server.conf`

This document contains manual `curl` test cases to validate webserv when running with `multi_server.conf`.  
These tests demonstrate multiple servers running on different ports and virtual host resolution.

---

## Without Default Server

```bash
curl -v http://localhost:8080/
# 200 OK → serves index.html

curl -v http://localhost:9090/
# 200 OK → serves dashboard.html

curl -v http://localhost:8080/not-found.html
# 404 Custom (file not found)

curl -v --resolve example.com:8080:127.0.0.1 http://example.com:8080/ 
# 200 OK → serves index.html

curl -v --resolve another.com:8080:127.0.0.1 http://another.com:8080/ #another html
# 200 OK → serves another.html

curl -vL --resolve example.com:8080:127.0.0.1 http://example.com:8080/legacy 
# 301 Moved Permanently, then 200 OK → serves new-location.html

curl -v -H "Host: unknownhost.com" http://127.0.0.1:8080/ 
# 200 OK → serves index.html

curl -v -H "Host: admin.localhost" http://127.0.0.1:9090/ 
# 200 OK → serves dashboard.html

```

## With default server 
This works in Mac (as Linux doesn't allow binding to same host/port twice even if one is 0.0.0.0 and the other is more specific)

```bash
curl -v http://127.0.0.1:8080/
# 200 OK → serves default.html

curl -v http://localhost:8080/
# 200 OK → serves default.html

curl -v -H "Host: example.com" http://127.0.0.1:8080/ 
# 200 OK → serves index.html

curl -v -H "Host: another.com" http://127.0.0.1:8080/ 
# 200 OK → serves another.html

curl -v -H "Host: unknownhost.com" http://127.0.0.1:8080/ 
# 200 OK → serves default.html

curl -v -H "Host: admin.localhost" http://127.0.0.1:9090/ 
# 200 OK → serves dashboard.html
```