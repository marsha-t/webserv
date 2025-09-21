# 🧪 Tests for `default.conf`

This document lists manual `curl` test cases for validating webserv using `default.conf`.  

---

## GET Requests

```bash
curl -v http://localhost:8080/
# 200 OK
# If autoindex=ON and index.html missing in location / → directory listing
# If autoindex=OFF and index.html missing in location / → 404 Custom

curl -v http://localhost:8080
# 200 OK

curl -v http://localhost:8080/index.html
# 200 OK

curl -v http://localhost:8080/missing.html
# 404 Not Found (Custom)

curl -vL http://localhost:8080/uploads/ 
# 200 OK → directory listing

curl -vL http://localhost:8080/files/ 
# 404 Not Found (Custom)

curl -vL http://localhost:8080/files/files/index.html 
# 200 OK

curl -v http://localhost:8080/%ZZ 
# 400 Bad Request
```

## POST Requests
```bash
echo "This is a test upload" > test.txt
curl -v -F "file=@test.txt" http://localhost:8080/uploads/  
# 201 Created on first try
curl -v -F "file=@test.txt" http://localhost:8080/uploads/  
# 409 Conflict on second try

curl -v -F "username=alice" -F "file=@test.txt" http://localhost:8080/uploads/ 
# 201 Created on first try
curl -v -F "username=alice" -F "file=@test.txt" http://localhost:8080/uploads/ 
# 409 Conflict on second try

curl -v -F "note=hello" -F "id=123" http://localhost:8080/uploads/ 
# 204 No Content

curl -v -F "empty=" http://localhost:8080/uploads/ 
# 204 No Content

curl -X POST -d "name=marsha&age=42" \
	-H "Content-Type: application/x-www-form-urlencoded"  \
	http://localhost:8080/uploads/ -v 
# 200 OK (with body = marsha & age = 42)
    
curl -v -X POST -d "hello from POST" http://localhost:8080/uploads/ 
# 400 Bad Request

curl -v -X POST -d "hello" -H "Content-Type: text/plain" http://localhost:8080/uploads/ 
# 201 Created (body saved to file with timestamp)

```
## DELETE Requests
```bash
curl -X DELETE http://localhost:8080/uploads/files/test.txt -v 
# 204 No Content (if file exists)
# 404 Not Found (if file doesn't exist)

curl -X PATCH http://localhost:8080/ 
# 405 Method Not Allowed
```

## Redirect
``` bash
curl -vL http://localhost:8080/redirect-me 
# 301 Moved Permanently, then 200 OK
```

## CGI
```bash 
curl http://localhost:8080/cgi-bin/test.py -v 
# 403 Forbidden (if not permitted)
# 200 OK otherwise

curl http://localhost:8080/cgi-bin/doesnotexist.py -v 
# 404 Not Found

curl "http://localhost:8080/cgi-bin/test.py?name=marsha&age=42" -v 
# 200 OK

curl -X POST -d "name=webserv&lang=cpp" http://localhost:8080/cgi-bin/testpost.py -v 
# 200 OK

curl http://localhost:8080/cgi-bin/broken.py -v 
# 500 Internal Service Error

echo "name=webserv_chunked&lang=cgi" > yourdata.txt
curl -X POST -H "Transfer-Encoding: chunked" \
     --data-binary @yourdata.txt http://localhost:8080/cgi-bin/testpost.py -v 
# 200 OK

curl http://localhost:8080/cgi-bin/exiterror.py -v 
# 500 Internal Service Error
```